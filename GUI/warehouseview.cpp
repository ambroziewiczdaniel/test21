#include "warehouseview.h"
#include <QDebug>
#include <QFile> // Te include były w Twoim kodzie, więc je zachowuję.
#include <QTextStream>
#include <QDateTime>
#include <QPushButton>
#include <QSqlQuery>
#include <QVariant>
#include <QHBoxLayout>
#include <QMessageBox> // Dla QMessageBox::critical

WarehouseView::WarehouseView(systemWarehouse* system, QWidget *parent) :
    QWidget(parent),
    m_system(system), // <-- Upewnij się, że to pasuje do deklaracji w .h
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

    model = new QSqlQueryModel(this); // Upewnij się, że tu tworzysz QSqlQueryModel
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

    setProductQuery(""); // Załaduj dane początkowe
}

WarehouseView::~WarehouseView()
{
    // Obiekty będące dziećmi WarehouseView zostaną automatycznie usunięte.
}

void WarehouseView::setProductQuery(const QString &filterText)
{
    QSqlQuery query = m_system->getWarehouseProductQuery(filterText); // <-- Użyj m_system

    if (!query.exec()) { // Wykonaj zapytanie i sprawdź błędy tutaj, w widoku.
        QMessageBox::critical(this, "Błąd Zapytania SQL (Magazyn)",
                              "Błąd podczas ładowania danych: " + query.lastError().text() + "\\nZapytanie: " + query.executedQuery());
        qDebug() << "WarehouseView: Błąd Zapytania SQL:" << query.lastError().text();
        qDebug() << "WarehouseView: Zapytanie:" << query.executedQuery();
        return;
    }

    model->setQuery(std::move(query)); // Model jest teraz QSqlQueryModel*, więc ma setQuery

    if (model->lastError().isValid()) { // Model jest teraz QSqlQueryModel*, więc ma lastError
        QMessageBox::critical(this, "Błąd Modelu SQL (Magazyn)",
                              "Błąd po ustawieniu zapytania w modelu: " + model->lastError().text());
        qDebug() << "WarehouseView: Błąd Modelu SQL:" << model->lastError().text();
    }

    // Ustawienie nagłówków
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
