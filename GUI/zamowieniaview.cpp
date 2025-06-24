#include "zamowieniaview.h"
#include <QDebug>

ZamowieniaView::ZamowieniaView(systemWarehouse* system, QWidget *parent) :
    QWidget(parent),
    m_system(system), // Inicjalizujemy wskaźnik do systemu
    model(nullptr),
    tableView(nullptr),
    searchLabel(nullptr),
    searchLineEdit(nullptr)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    searchLabel = new QLabel("Wyszukaj zamówienie (po kontrahencie lub statusie):", this);
    searchLineEdit = new QLineEdit(this);
    searchLineEdit->setPlaceholderText("Wpisz kontrahenta lub status...");

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

    layout->addWidget(tableView);

    connect(searchLineEdit, &QLineEdit::textChanged, this, &ZamowieniaView::onSearchTextChanged);
    connect(tableView->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &ZamowieniaView::onRowClicked);

    setOrderQuery(""); // Załaduj dane początkowe
}

ZamowieniaView::~ZamowieniaView()
{
    // model i tableView są dziećmi ZamowieniaView, więc zostaną automatycznie usunięte.
}

void ZamowieniaView::setOrderQuery(const QString &filterText)
{
    // Pobieramy QSqlQuery z systemWarehouse, które korzysta z OrderDao
    QSqlQuery query = m_system->getOrdersQuery(filterText);

    if (!query.exec()) { // Wykonaj zapytanie i obsłuż błędy tutaj
        QMessageBox::critical(this, "Błąd Zapytania SQL (Zamówienia)",
                              "Błąd podczas ładowania danych: " + query.lastError().text() + "\nZapytanie: " + query.executedQuery());
        qDebug() << "ZamowieniaView: Błąd Zapytania SQL:" << query.lastError().text();
        qDebug() << "ZamowieniaView: Zapytanie:" << query.executedQuery();
        return;
    }

    model->setQuery(std::move(query));

    if (model->lastError().isValid()) {
        QMessageBox::critical(this, "Błąd Modelu SQL (Zamówienia)",
                              "Błąd po ustawieniu zapytania w modelu: " + model->lastError().text());
        qDebug() << "ZamowieniaView: Błąd Modelu SQL:" << model->lastError().text();
    }

    // Ustawienie nagłówków (pamiętając o id_zamowienia)
    model->setHeaderData(0, Qt::Horizontal, "ID Zamówienia");
    model->setHeaderData(1, Qt::Horizontal, "Kontrahent");
    model->setHeaderData(2, Qt::Horizontal, "Data");
    model->setHeaderData(3, Qt::Horizontal, "Status");
}

void ZamowieniaView::onSearchTextChanged(const QString &text)
{
    setOrderQuery(text);
}

void ZamowieniaView::refreshData()
{
    qDebug() << "ZamowieniaView: Odświeżam dane.";
    setOrderQuery(searchLineEdit->text());
}

void ZamowieniaView::onRowClicked(const QModelIndex &index)
{
    if (!index.isValid()) return;

    int orderId = model->data(model->index(index.row(), 0)).toInt();
    qDebug() << "Wybrano zamówienie o ID:" << orderId;

    if (orderId > 0) {
        // OrderDetailsDialog nadal używa QSqlDatabase&. To będzie kolejny cel refaktoryzacji.
        // Na razie przekazujemy m_system.
        OrderDetailsDialog dialog(m_system, orderId, this); // Przekazujemy m_system
        connect(&dialog, &OrderDetailsDialog::orderStatusChanged, this, &ZamowieniaView::onOrderStatusChanged);
        dialog.exec();
    }
}

void ZamowieniaView::onOrderStatusChanged(int orderId)
{
    Q_UNUSED(orderId);
    qDebug() << "ZamowieniaView: Odbieram sygnał zmiany statusu zamówienia. Odświeżam tabelę i emituję databaseUpdated.";
    setOrderQuery(searchLineEdit->text());
    emit databaseUpdated();
}
