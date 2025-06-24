#include "warehouseview.h"
#include <QDebug>

WarehouseView::WarehouseView(systemWarehouse* System, QWidget *parent) :
    QWidget(parent),
    model(nullptr),
    tableView(nullptr),
    searchLabel(nullptr),
    searchLineEdit(nullptr),
    System(System) // Upewnij się, że ten konstruktor jest poprawnie inicjalizowany
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

    setProductQuery(""); // Załaduj dane początkowe
}

WarehouseView::~WarehouseView()
{
    // model i tableView są dziećmi WarehouseView, więc zostaną automatycznie usunięte.
}

void WarehouseView::setProductQuery(const QString &filterText)
{
    // Pobieramy QSqlQuery z systemWarehouse, a następnie ustawiamy je w modelu.
    QSqlQuery query = System->getWarehouseProductQuery(filterText);

    if (!query.exec()) { // Wykonaj zapytanie i sprawdź błędy tutaj, w widoku.
        QMessageBox::critical(this, "Błąd Zapytania SQL (Magazyn)",
                              "Błąd podczas ładowania danych: " + query.lastError().text() + "\nZapytanie: " + query.executedQuery());
        qDebug() << "WarehouseView: Błąd Zapytania SQL:" << query.lastError().text();
        qDebug() << "WarehouseView: Zapytanie:" << query.executedQuery();
        return;
    }

    model->setQuery(std::move(query)); // Ustaw model na wynik zapytania

    if (model->lastError().isValid()) { // Sprawdź błędy modelu
        QMessageBox::critical(this, "Błąd Modelu SQL (Magazyn)",
                              "Błąd po ustawieniu zapytania w modelu: " + model->lastError().text());
        qDebug() << "WarehouseView: Błąd Modelu SQL:" << model->lastError().text();
    }

    // Ustawienie nagłówków (może być powtarzalne, ale zapewnia poprawność po każdym zapytaniu)
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