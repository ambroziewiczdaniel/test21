#include "warehouseview.h"
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QPushButton>
#include <QSqlQuery>
#include <QVariant>
#include <QHBoxLayout>
#include <QMessageBox>

WarehouseView::WarehouseView(systemWarehouse* system, QWidget *parent) :
    QWidget(parent),
    m_system(system),
    model(nullptr),
    tableView(nullptr),
    searchLabel(nullptr),
    searchLineEdit(nullptr)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    searchLabel = new QLabel("Wyszukaj produkt (po nazwie):", this);
    searchLineEdit = new QLineEdit(this);
    searchLineEdit->setPlaceholderText("Wpisz nazwę produktu...");

    layout->addWidget(searchLabel);
    layout->addWidget(searchLineEdit);

    model = new QSqlQueryModel(this);
    tableView = new QTableView(this);
    tableView->setModel(model);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->verticalHeader()->setVisible(false);

    tableView->setItemDelegate(new WarehouseItemDelegate(this));

    layout->addWidget(tableView);

    connect(searchLineEdit, &QLineEdit::textChanged, this, &WarehouseView::onSearchTextChanged);

    setProductQuery("");
}

WarehouseView::~WarehouseView()
{

}

void WarehouseView::setProductQuery(const QString &filterText)
{
    QSqlQuery query = m_system->getWarehouseProductQuery(filterText);

    if (!query.exec()) { // Wykonaj zapytanie i sprawdź błędy tutaj, w widoku.
        QMessageBox::critical(this, "Błąd Zapytania SQL (Magazyn)",
                              "Błąd podczas ładowania danych: " + query.lastError().text() + "\\nZapytanie: " + query.executedQuery());
        qDebug() << "WarehouseView: Błąd Zapytania SQL:" << query.lastError().text();
        qDebug() << "WarehouseView: Zapytanie:" << query.executedQuery();
        return;
    }

    model->setQuery(std::move(query)); 

    if (model->lastError().isValid()) {
        QMessageBox::critical(this, "Błąd Modelu SQL (Magazyn)",
                              "Błąd po ustawieniu zapytania w modelu: " + model->lastError().text());
        qDebug() << "WarehouseView: Błąd Modelu SQL:" << model->lastError().text();
    }

    model->setHeaderData(0, Qt::Horizontal, "ID Produktu");
    model->setHeaderData(1, Qt::Horizontal, "Nazwa Produktu");
    model->setHeaderData(2, Qt::Horizontal, QString::fromUtf8("Minimalna dopuszczalna ilosc"));
    model->setHeaderData(3, Qt::Horizontal, QString::fromUtf8("Całkowita ilość w magazynie"));
    model->setHeaderData(4, Qt::Horizontal, QString::fromUtf8("Lokalizacje (Regał-Półka-Kolumna, Ilość)"));
}

void WarehouseView::onSearchTextChanged(const QString &text)
{
    setProductQuery(text);
}

void WarehouseView::refreshData()
{
    qDebug() << "WarehouseView: Odświeżam dane.";
    setProductQuery(searchLineEdit->text());
}
