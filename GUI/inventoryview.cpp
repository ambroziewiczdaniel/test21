#include "inventoryview.h"
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QPushButton>
#include <QSqlQuery>
#include <QVariant>
#include <QHBoxLayout>

InventoryView::InventoryView(systemWarehouse* system, QWidget *parent) :
    QWidget(parent),
    m_system(system),
    mainTableModel(nullptr),
    mainTableView(nullptr),
    searchLabel(nullptr),
    searchLineEdit(nullptr),
    selectedProductsLabel(nullptr),
    selectedProductsTableView(nullptr),
    selectedProductsModel(nullptr),
    generateInventoryDocumentButton(nullptr)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QHBoxLayout *searchLayout = new QHBoxLayout();
    searchLabel = new QLabel("Wyszukaj produkt (po nazwie):", this);
    searchLineEdit = new QLineEdit(this);
    searchLineEdit->setPlaceholderText("Wpisz nazwę produktu...");

    searchLayout->addWidget(searchLabel);
    searchLayout->addWidget(searchLineEdit);
    mainLayout->addLayout(searchLayout);

    mainTableModel = new QSqlQueryModel(this);
    mainTableView = new QTableView(this);
    mainTableView->setModel(mainTableModel);
    mainTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mainTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    mainTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    mainTableView->horizontalHeader()->setStretchLastSection(true);
    mainTableView->verticalHeader()->setVisible(false);

    mainTableView->setItemDelegateForColumn(4, new InventoryCheckboxDelegate(this, &m_checkboxStates, this));

    mainLayout->addWidget(mainTableView);

    connect(searchLineEdit, &QLineEdit::textChanged, this, &InventoryView::onSearchTextChanged);

    selectedProductsLabel = new QLabel("Zaznaczone produkty do inwentaryzacji:", this);
    selectedProductsTableView = new QTableView(this);
    selectedProductsModel = new QStandardItemModel(0, 4, this);
    selectedProductsModel->setHeaderData(0, Qt::Horizontal, "ID Produktu");
    selectedProductsModel->setHeaderData(1, Qt::Horizontal, "Nazwa Produktu");
    selectedProductsModel->setHeaderData(2, Qt::Horizontal, "Ilość w magazynie (wg systemu)");
    selectedProductsModel->setHeaderData(3, Qt::Horizontal, "Prawdziwa ilość w magazynie");
    selectedProductsTableView->setModel(selectedProductsModel);
    selectedProductsTableView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::AnyKeyPressed);
    selectedProductsTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    selectedProductsTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    selectedProductsTableView->horizontalHeader()->setStretchLastSection(true);
    selectedProductsTableView->verticalHeader()->setVisible(false);

    selectedProductsTableView->setItemDelegateForColumn(3, new EditableQuantityDelegate(this));

    generateInventoryDocumentButton = new QPushButton("Generuj dokument inwentaryzacji", this);
    connect(generateInventoryDocumentButton, &QPushButton::clicked, this, &InventoryView::onGenerateInventoryDocumentClicked);

    mainLayout->addWidget(selectedProductsLabel);
    mainLayout->addWidget(selectedProductsTableView);
    mainLayout->addWidget(generateInventoryDocumentButton);

    setProductQuery("");
}

InventoryView::~InventoryView()
{
}

void InventoryView::setProductQuery(const QString &filterText)
{
    m_checkboxStates.clear();
    mainTableView->viewport()->update();

    QSqlQuery query = m_system->getWarehouseProductQuery(filterText);

    if (!query.exec()) {
        QMessageBox::critical(this, "Błąd Zapytania SQL (Inwentaryzacja)",
                              "Błąd podczas ładowania danych: " + query.lastError().text() + "\nZapytanie: " + query.executedQuery());
        qDebug() << "InventoryView: Błąd Zapytania SQL (Inwentaryzacja):" << query.lastError().text();
        qDebug() << "InventoryView: Zapytanie:" << query.executedQuery();
        return;
    }

    mainTableModel->setQuery(std::move(query));

    if (mainTableModel->lastError().isValid()) {
        QMessageBox::critical(this, "Błąd Modelu SQL (Inwentaryzacja)",
                              "Błąd po ustawieniu zapytania w modelu: " + mainTableModel->lastError().text());
        qDebug() << "InventoryView: Błąd Modelu SQL (Inwentaryzacja):" << mainTableModel->lastError().text();
    }

    mainTableModel->setHeaderData(0, Qt::Horizontal, "ID Produktu");
    mainTableModel->setHeaderData(1, Qt::Horizontal, "Nazwa Produktu");
    mainTableModel->setHeaderData(2, Qt::Horizontal, "Całkowita ilość w magazynie (wg systemu)");
    mainTableModel->setHeaderData(3, Qt::Horizontal, "Lokalizacje (Regał-Półka-Kolumna, Ilość)");
    mainTableModel->setHeaderData(4, Qt::Horizontal, "Zaznacz do inwentaryzacji");

    selectedProductsModel->clear();
    selectedProductsModel->setHeaderData(0, Qt::Horizontal, "ID Produktu");
    selectedProductsModel->setHeaderData(1, Qt::Horizontal, "Nazwa Produktu");
    selectedProductsModel->setHeaderData(2, Qt::Horizontal, "Ilość w magazynie (wg systemu)");
    selectedProductsModel->setHeaderData(3, Qt::Horizontal, "Prawdziwa ilość w magazynie");
    m_realQuantities.clear();
}

void InventoryView::onSearchTextChanged(const QString &text)
{
    setProductQuery(text);
}

void InventoryView::refreshData()
{
    qDebug() << "InventoryView: Odświeżam dane.";
    setProductQuery(searchLineEdit->text());
}

void InventoryView::updateSelectedProductsTable(int row, bool checked)
{
    int productId = mainTableModel->data(mainTableModel->index(row, 0)).toInt();
    QString productName = mainTableModel->data(mainTableModel->index(row, 1)).toString();
    int actualQuantity = mainTableModel->data(mainTableModel->index(row, 2)).toInt();

    if (checked) {
        QList<QStandardItem*> rowItems;
        QStandardItem *idItem = new QStandardItem(QString::number(productId));
        idItem->setEditable(false);
        rowItems << idItem;

        QStandardItem *nameItem = new QStandardItem(productName);
        nameItem->setEditable(false);
        rowItems << nameItem;

        QStandardItem *actualQtyItem = new QStandardItem(QString::number(actualQuantity));
        actualQtyItem->setEditable(false);
        rowItems << actualQtyItem;

        QStandardItem *realQtyItem = new QStandardItem(QString::number(actualQuantity));
        realQtyItem->setEditable(true);
        rowItems << realQtyItem;

        selectedProductsModel->appendRow(rowItems);

        m_realQuantities.insert(productId, actualQuantity);
    } else {
        for (int i = 0; i < selectedProductsModel->rowCount(); ++i) {
            if (selectedProductsModel->item(i, 0)->text().toInt() == productId) {
                selectedProductsModel->removeRow(i);
                m_realQuantities.remove(productId);
                break;
            }
        }
    }
}

void InventoryView::onGenerateInventoryDocumentClicked()
{
    if (selectedProductsModel->rowCount() == 0) {
        QMessageBox::warning(this, "Błąd", "Brak zaznaczonych produktów do wygenerowania dokumentu inwentaryzacji.");
        return;
    }

    QList<InventoryProductData> productsToGenerate;
    for (int i = 0; i < selectedProductsModel->rowCount(); ++i) {
        InventoryProductData data;
        data.id = selectedProductsModel->item(i, 0)->text().toInt();
        data.name = selectedProductsModel->item(i, 1)->text();
        data.quantityInSystem = selectedProductsModel->item(i, 2)->text().toInt();
        data.realQuantity = selectedProductsModel->item(i, 3)->text().toInt();
        productsToGenerate.append(data);
    }

    InventoryDocumentGenerator generator; 
    if (generator.generateDocument(productsToGenerate)) {
        QMessageBox::information(this, "Sukces", "Wygenerowano plik inwentaryzacji:\\n" + QDir::currentPath() + "/INW-" + QDateTime::currentDateTime().toString("hhmmss-yyyyMMdd") + ".txt");
    } else {
        QMessageBox::critical(this, "Błąd Zapisywania", "Nie można zapisać pliku inwentaryzacji.");
    }
}

// Implementacja delegata dla checkboxów
InventoryCheckboxDelegate::InventoryCheckboxDelegate(QObject *parent, QHash<int, bool> *checkboxStates, InventoryView *view)
    : QStyledItemDelegate(parent), m_checkboxStates(checkboxStates), m_inventoryView(view)
{
}

void InventoryCheckboxDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.column() == 4) {
        QStyleOptionButton checkboxOption;
        checkboxOption.state = QStyle::State_Enabled;

        if (m_checkboxStates->value(index.row(), false)) {
            checkboxOption.state |= QStyle::State_On;
        } else {
            checkboxOption.state |= QStyle::State_Off;
        }

        checkboxOption.rect = QApplication::style()->subElementRect(QStyle::SE_CheckBoxIndicator, &checkboxOption, option.widget);
        checkboxOption.rect.moveCenter(option.rect.center());

        painter->save();
        QApplication::style()->drawControl(QStyle::CE_CheckBox, &checkboxOption, painter, option.widget);
        painter->restore();
    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

bool InventoryCheckboxDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (index.column() == 4) {
        if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            QStyleOptionButton checkboxOption;
            checkboxOption.state = QStyle::State_Enabled;
            checkboxOption.rect = QApplication::style()->subElementRect(QStyle::SE_CheckBoxIndicator, &checkboxOption, option.widget);
            checkboxOption.rect.moveCenter(option.rect.center());

            if (checkboxOption.rect.contains(mouseEvent->pos())) {
                bool currentState = m_checkboxStates->value(index.row(), false);
                m_checkboxStates->insert(index.row(), !currentState);

                if (m_inventoryView) {
                    m_inventoryView->updateSelectedProductsTable(index.row(), !currentState);
                }

                QWidget *viewWidget = const_cast<QWidget*>(option.widget);
                if (viewWidget) {
                    viewWidget->update(option.rect);
                }
                return true;
            }
        }
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

// Implementacja delegata dla edytowalnej kolumny ilości
EditableQuantityDelegate::EditableQuantityDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QWidget *EditableQuantityDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    QSpinBox *editor = new QSpinBox(parent);
    editor->setMinimum(0);
    editor->setMaximum(99999);
    return editor;
}

void EditableQuantityDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    int value = index.model()->data(index, Qt::EditRole).toInt();
    QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
    spinBox->setValue(value);
}

void EditableQuantityDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
    spinBox->interpretText();
    int value = spinBox->value();
    model->setData(index, value, Qt::EditRole);
}

void EditableQuantityDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    editor->setGeometry(option.rect);
}
