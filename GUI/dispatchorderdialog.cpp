#include "dispatchorderdialog.h"
#include <QDebug>
#include <QHeaderView>

#include "../LocationDao.h"

DispatchOrderDialog::DispatchOrderDialog(systemWarehouse* system, int orderId, QWidget *parent) :
    QDialog(parent),
    m_system(system),
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

    dispatchProductsModel = new QStandardItemModel(0, 4, this);
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
    dispatchProductsTableView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    // Now creates the delegate from its own file
    dispatchProductsTableView->setItemDelegateForColumn(3, new DispatchQuantityDelegate(this));

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
    // Objects that are children of QDialog will be automatically deleted.
}

void DispatchOrderDialog::loadProductsForDispatch()
{
    dispatchProductsModel->clear();
    dispatchProductsModel->setHeaderData(0, Qt::Horizontal, "ID Produktu");
    dispatchProductsModel->setHeaderData(1, Qt::Horizontal, "Nazwa Produktu");
    dispatchProductsModel->setHeaderData(2, Qt::Horizontal, "Ilość w zamówieniu");
    dispatchProductsModel->setHeaderData(3, Qt::Horizontal, "Ilość do wydania");

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

        QStandardItem *dispatchQtyItem = new QStandardItem(QString::number(item->getQuantity()));
        dispatchQtyItem->setEditable(true);
        rowItems << dispatchQtyItem;

        dispatchProductsModel->appendRow(rowItems);
        delete item;
    }
}

void DispatchOrderDialog::onConfirmDispatchClicked()
{
    if (dispatchProductsModel->rowCount() == 0) {
        QMessageBox::warning(this, "Błąd", "Brak produktów do wydania.");
        return;
    }

    if (!m_system->getDb().transaction()) {
        QMessageBox::critical(this, "Błąd Bazy Danych", "Nie można rozpocząć transakcji wydania: " + m_system->getDb().lastError().text());
        qDebug() << "DispatchOrderDialog: Błąd rozpoczęcia transakcji:" << m_system->getDb().lastError().text();
        return;
    }

    bool allDispatchSuccessful = true;
    QStringList dispatchSummary;
    dispatchSummary << "Wydano produkty dla zamówienia ID " + QString::number(m_orderId) + ":";

    for (int i = 0; i < dispatchProductsModel->rowCount(); ++i) {
        int productId = dispatchProductsModel->item(i, 0)->text().toInt();
        QString productName = dispatchProductsModel->item(i, 1)->text();
        int orderedQuantity = dispatchProductsModel->item(i, 2)->text().toInt();
        int quantityToDispatch = dispatchProductsModel->item(i, 3)->text().toInt();

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
            continue;
        }

        QVector<WarehouseProductData> warehouseData = m_system->getLocationDao()->getWarehouseProducts();
        int currentTotalStock = 0;
        for (const auto& data : warehouseData) {
            if (data.id == productId) {
                currentTotalStock = data.totalActualQuantity;
                break;
            }
        }

        if (currentTotalStock < quantityToDispatch) {
            QMessageBox::critical(this, "Błąd Magazynu", "Brak wystarczającej ilości produktu '" + productName + "' w magazynie. Dostępno: " + QString::number(currentTotalStock) + ", wymagane: " + QString::number(quantityToDispatch) + ".");
            allDispatchSuccessful = false;
            break;
        }

        QSqlQuery locationsQuery(m_system->getDb());
        locationsQuery.prepare("SELECT regal, polka, kolumna, ilosc FROM locations WHERE id = :productId ORDER BY regal, polka, kolumna");
        locationsQuery.bindValue(":productId", productId);
        if (!locationsQuery.exec()) {
            QMessageBox::critical(this, "Błąd Magazynu", "Błąd pobierania lokalizacji dla produktu '" + productName + "': " + locationsQuery.lastError().text());
            allDispatchSuccessful = false;
            break;
        }

        int remainingToDispatch = quantityToDispatch;
        while (locationsQuery.next() && remainingToDispatch > 0) {
            int regal = locationsQuery.value("regal").toInt();
            int polka = locationsQuery.value("polka").toInt();
            int kolumna = locationsQuery.value("kolumna").toInt();
            int currentIlosc = locationsQuery.value("ilosc").toInt();

            int qtyFromLocation = qMin(remainingToDispatch, currentIlosc);
            int newIloscInLocation = currentIlosc - qtyFromLocation;

            if (!m_system->getLocationDao()->updateProductLocationQuantity(productId, regal, polka, kolumna, newIloscInLocation)) {
                QMessageBox::critical(this, "Błąd Wydania", "Nie można zaktualizować lokalizacji: " + productName + " w R" + QString::number(regal) + "-P" + QString::number(polka) + "-K" + QString::number(kolumna) + ".");
                allDispatchSuccessful = false;
                break;
            }
            remainingToDispatch -= qtyFromLocation;
        }

        if (!allDispatchSuccessful) break;

        dispatchSummary << "- " + productName + ": wydano " + QString::number(quantityToDispatch) + " szt.";
    }

    if (allDispatchSuccessful) {
        m_system->getDb().commit();
        QMessageBox::information(this, "Wydanie Potwierdzone", dispatchSummary.join("\n"));
        emit dispatchCompleted(true);
        accept();
    } else {
        m_system->getDb().rollback();
        QMessageBox::critical(this, "Błąd Wydania", "Operacja wydania nie powiodła się. Wszystkie zmiany zostały wycofane.");
        emit dispatchCompleted(false);
    }
}

void DispatchOrderDialog::onCancelClicked()
{
    emit dispatchCancelled();
    reject();
}

void DispatchOrderDialog::onQuantityChanged(const QModelIndex &index)
{
    Q_UNUSED(index);
}
