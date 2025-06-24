#include "dispatchorderdialog.h"
#include <QDebug>
#include <QHeaderView> // Dla stretchLastSection
#include <QSpinBox> // Dla edytowalnej kolumny
#include <QItemDelegate> // Dla edytowalnej kolumny

// Nowy delegat dla edytowalnej kolumny (ilość do wydania)
class DispatchQuantityDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit DispatchQuantityDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        Q_UNUSED(option);
        Q_UNUSED(index);
        QSpinBox *editor = new QSpinBox(parent);
        editor->setMinimum(0); // Możliwe wydanie 0 (anulowanie wydania danego produktu)
        editor->setMaximum(99999);
        return editor;
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const override
    {
        int value = index.model()->data(index, Qt::EditRole).toInt();
        QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
        spinBox->setValue(value);
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override
    {
        QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
        spinBox->interpretText();
        int value = spinBox->value();
        model->setData(index, value, Qt::EditRole);
    }

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        Q_UNUSED(index);
        editor->setGeometry(option.rect);
    }
};


DispatchOrderDialog::DispatchOrderDialog(systemWarehouse* system, int orderId, QWidget *parent) :
    QDialog(parent),
    m_system(system), // Inicjalizujemy wskaźnik do systemu
    m_orderId(orderId),
    orderInfoLabel(nullptr),
    dispatchProductsModel(nullptr),
    dispatchProductsTableView(nullptr),
    confirmDispatchButton(nullptr),
    cancelButton(nullptr)
{
    setWindowTitle("Wydanie Zamówienia ID: " + QString::number(m_orderId));
    setMinimumSize(600, 400);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    orderInfoLabel = new QLabel("Produkty do wydania dla zamówienia ID: " + QString::number(m_orderId), this);
    mainLayout->addWidget(orderInfoLabel);

    dispatchProductsModel = new QStandardItemModel(0, 4, this); // ID_prod, Nazwa, Ilość_zamówiona, Ilość_do_wydania
    dispatchProductsModel->setHeaderData(0, Qt::Horizontal, "ID Produktu");
    dispatchProductsModel->setHeaderData(1, Qt::Horizontal, "Nazwa Produktu");
    dispatchProductsModel->setHeaderData(2, Qt::Horizontal, "Ilość w zamówieniu");
    dispatchProductsModel->setHeaderData(3, Qt::Horizontal, "Ilość do wydania");

    dispatchProductsTableView = new QTableView(this);
    dispatchProductsTableView->setModel(dispatchProductsModel);
    dispatchProductsTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    dispatchProductsTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    dispatchProductsTableView->horizontalHeader()->setStretchLastSection(true);
    dispatchProductsTableView->verticalHeader()->setVisible(false);
    dispatchProductsTableView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed); // Edycja ilości do wydania
    dispatchProductsTableView->setItemDelegateForColumn(3, new DispatchQuantityDelegate(this)); // Ustawienie delegata dla kolumny "Ilość do wydania"

    mainLayout->addWidget(dispatchProductsTableView);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    confirmDispatchButton = new QPushButton("Potwierdź Wydanie", this);
    cancelButton = new QPushButton("Anuluj", this);

    connect(confirmDispatchButton, &QPushButton::clicked, this, &DispatchOrderDialog::onConfirmDispatchClicked);
    connect(cancelButton, &QPushButton::clicked, this, &DispatchOrderDialog::onCancelClicked);

    buttonLayout->addStretch();
    buttonLayout->addWidget(confirmDispatchButton);
    buttonLayout->addWidget(cancelButton);
    mainLayout->addLayout(buttonLayout);

    loadProductsForDispatch();
}

DispatchOrderDialog::~DispatchOrderDialog()
{
    // Obiekty są dziećmi QDialog, więc zostaną automatycznie usunięte.
}

void DispatchOrderDialog::loadProductsForDispatch()
{
    // Wyczyść model przed załadowaniem
    dispatchProductsModel->clear();
    dispatchProductsModel->setHeaderData(0, Qt::Horizontal, "ID Produktu");
    dispatchProductsModel->setHeaderData(1, Qt::Horizontal, "Nazwa Produktu");
    dispatchProductsModel->setHeaderData(2, Qt::Horizontal, "Ilość w zamówieniu");
    dispatchProductsModel->setHeaderData(3, Qt::Horizontal, "Ilość do wydania");

    // Pobierz pozycje zamówienia za pomocą OrderItemDao
    QVector<OrderItem*> items = m_system->getOrderItemDao()->getOrderItemsByOrderId(m_orderId);

    if (items.isEmpty()) {
        QMessageBox::warning(this, "Brak Produktów", "Brak produktów w tym zamówieniu do wydania.");
        qDebug() << "DispatchOrderDialog: Brak pozycji produktów dla zamówienia ID" << m_orderId;
        return;
    }

    for (OrderItem* item : items) {
        QList<QStandardItem*> rowItems;
        QStandardItem *idItem = new QStandardItem(QString::number(item->getProductId()));
        idItem->setEditable(false);
        rowItems << idItem;

        QStandardItem *nameItem = new QStandardItem(item->getProductName());
        nameItem->setEditable(false);
        rowItems << nameItem;

        QStandardItem *orderedQtyItem = new QStandardItem(QString::number(item->getQuantity()));
        orderedQtyItem->setEditable(false);
        rowItems << orderedQtyItem;

        QStandardItem *dispatchQtyItem = new QStandardItem(QString::number(item->getQuantity())); // Domyślnie wydajemy tyle, ile zamówiono
        dispatchQtyItem->setEditable(true); // Ta kolumna jest edytowalna
        rowItems << dispatchQtyItem;

        dispatchProductsModel->appendRow(rowItems);
        delete item; // Zwolnij pamięć po OrderItem
    }
}

void DispatchOrderDialog::onConfirmDispatchClicked()
{
    if (dispatchProductsModel->rowCount() == 0) {
        QMessageBox::warning(this, "Błąd", "Brak produktów do wydania.");
        return;
    }

    // Rozpoczęcie transakcji
    if (!m_system->db.transaction()) {
        QMessageBox::critical(this, "Błąd Bazy Danych", "Nie można rozpocząć transakcji wydania: " + m_system->db.lastError().text());
        qDebug() << "DispatchOrderDialog: Błąd rozpoczęcia transakcji:" << m_system->db.lastError().text();
        return;
    }

    bool allDispatchSuccessful = true;
    QStringList dispatchSummary; // Do podsumowania wydania
    dispatchSummary << "Wydano produkty dla zamówienia ID " + QString::number(m_orderId) + ":";

    // Przechodzimy przez każdy produkt w modelu wydania
    for (int i = 0; i < dispatchProductsModel->rowCount(); ++i) {
        int productId = dispatchProductsModel->item(i, 0)->text().toInt();
        QString productName = dispatchProductsModel->item(i, 1)->text();
        int orderedQuantity = dispatchProductsModel->item(i, 2)->text().toInt();
        int quantityToDispatch = dispatchProductsModel->item(i, 3)->text().toInt();

        // Walidacja ilości
        if (quantityToDispatch < 0) {
            QMessageBox::warning(this, "Błąd Ilości", "Ilość do wydania dla produktu '" + productName + "' nie może być ujemna.");
            allDispatchSuccessful = false;
            break;
        }
        if (quantityToDispatch > orderedQuantity) {
             QMessageBox::warning(this, "Błąd Ilości", "Ilość do wydania dla produktu '" + productName + "' (" + QString::number(quantityToDispatch) + ") nie może być większa niż zamówiona ilość (" + QString::number(orderedQuantity) + ").");
             allDispatchSuccessful = false;
             break;
        }
        if (quantityToDispatch == 0) {
             dispatchSummary << "- " + productName + ": anulowano wydanie.";
             continue; // Pomiń ten produkt, jeśli ilość do wydania to 0
        }

        // Sprawdź bieżącą łączną ilość w magazynie (niezależnie od lokalizacji)
        // Musimy to zrobić, aby wiedzieć, czy możemy wydać
        QSqlQuery totalQtyQuery(m_system->db);
        totalQtyQuery.prepare("SELECT COALESCE(SUM(ilosc), 0) AS total_qty FROM locations WHERE id = :productId");
        totalQtyQuery.bindValue(":productId", productId);
        if (!totalQtyQuery.exec() || !totalQtyQuery.next()) {
            QMessageBox::critical(this, "Błąd Magazynu", "Błąd sprawdzania łącznej ilości dla produktu '" + productName + "'.");
            allDispatchSuccessful = false;
            break;
        }
        int currentTotalStock = totalQtyQuery.value("total_qty").toInt();

        if (currentTotalStock < quantityToDispatch) {
            QMessageBox::critical(this, "Błąd Magazynu", "Brak wystarczającej ilości produktu '" + productName + "' w magazynie. Dostępno: " + QString::number(currentTotalStock) + ", wymagane: " + QString::number(quantityToDispatch) + ".");
            allDispatchSuccessful = false;
            break;
        }

        // --- Proces wydania z lokalizacji (najpierw z konkretnych lokalizacji, potem z pozostałych) ---
        QVector<int> updatedLocationQuantities; // Przechowuje zmiany dla transakcji
        int remainingToDispatch = quantityToDispatch;

        // 1. Pobierz wszystkie lokalizacje dla produktu, posortowane rosnąco
        QSqlQuery locationsQuery(m_system->db);
        locationsQuery.prepare("SELECT regal, polka, kolumna, ilosc FROM locations WHERE id = :productId ORDER BY regal, polka, kolumna");
        locationsQuery.bindValue(":productId", productId);
        if (!locationsQuery.exec()) {
            QMessageBox::critical(this, "Błąd Magazynu", "Błąd pobierania lokalizacji dla produktu '" + productName + "': " + locationsQuery.lastError().text());
            allDispatchSuccessful = false;
            break;
        }

        while (locationsQuery.next() && remainingToDispatch > 0) {
            int regal = locationsQuery.value("regal").toInt();
            int polka = locationsQuery.value("polka").toInt();
            int kolumna = locationsQuery.value("kolumna").toInt();
            int currentIlosc = locationsQuery.value("ilosc").toInt();

            int qtyFromLocation = qMin(remainingToDispatch, currentIlosc);
            int newIloscInLocation = currentIlosc - qtyFromLocation;

            // Zaktualizuj ilość w danej lokalizacji
            // Używamy LocationDao
            if (!m_system->getLocationDao()->updateProductLocationQuantity(productId, regal, polka, kolumna, newIloscInLocation)) {
                QMessageBox::critical(this, "Błąd Wydania", "Nie można zaktualizować lokalizacji: " + productName + " w R" + QString::number(regal) + "-P" + QString::number(polka) + "-K" + QString::number(kolumna) + ".");
                allDispatchSuccessful = false;
                break;
            }
            remainingToDispatch -= qtyFromLocation;
        }

        if (!allDispatchSuccessful) break; // Przerwij zewnętrzną pętlę, jeśli wewnętrzne operacje zawiodły

        dispatchSummary << "- " + productName + ": wydano " + QString::number(quantityToDispatch) + " szt.";
    }

    if (allDispatchSuccessful) {
        m_system->db.commit(); // Zatwierdź transakcję
        QMessageBox::information(this, "Wydanie Potwierdzone", dispatchSummary.join("\n"));
        emit dispatchCompleted(true); // Sygnalizuj sukces
        accept(); // Zamknij dialog
    } else {
        m_system->db.rollback(); // Wycofaj transakcję
        QMessageBox::critical(this, "Błąd Wydania", "Operacja wydania nie powiodła się. Wszystkie zmiany zostały wycofane.");
        emit dispatchCompleted(false); // Sygnalizuj niepowodzenie
    }
}

void DispatchOrderDialog::onCancelClicked()
{
    emit dispatchCancelled();
    reject(); // Zamknij dialog z wynikiem QDialog::Rejected
}

void DispatchOrderDialog::onQuantityChanged(const QModelIndex &index)
{
    // Opcjonalnie: Dodaj logikę walidacji na bieżąco, jeśli użytkownik zmieni ilość
    // np. upewnij się, że ilość do wydania <= zamówionej ilości.
    // Index.column() == 3 (ilość do wydania)
    // Index.column() == 2 (ilość zamówiona)
    // Walidacja jest już w onConfirmDispatchClicked, więc to może być puste.
    Q_UNUSED(index);
}