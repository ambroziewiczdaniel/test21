#include "ksiegowoscview.h"
#include <QDebug>
#include <QSqlRecord>
#include <QHeaderView>
#include <QGridLayout> // Potrzebne dla lepszego układu edycji zamówień

// Zmieniono konstruktor, aby przyjmował systemWarehouse*
KsiegowoscView::KsiegowoscView(systemWarehouse* system, QWidget *parent) :
    QWidget(parent),
    m_system(system), // Inicjalizujemy wskaźnik do systemu
    tabWidget(nullptr), // Inicjalizacja tabWidget
    // Inicjalizacja zmiennych dla zakładki "Nowe Zamówienie"
    kontrahentLabel(nullptr),
    kontrahentLineEdit(nullptr),
    orderDateLabel(nullptr),
    productSearchLabel(nullptr),
    productSearchLineEdit(nullptr),
    productCompleter(nullptr),
    productCompleterModel(nullptr),
    currentProductIdLabel(nullptr),
    currentProductNameLabel(nullptr),
    productQuantitySpinBox(nullptr),
    addProductToOrderButton(nullptr),
    orderItemsLabel(nullptr),
    orderItemsTableView(nullptr),
    orderItemsModel(nullptr),
    createOrderButton(nullptr),
    // Inicjalizacja zmiennych dla zakładki "Edytuj Zamówienia"
    allOrdersTableView(nullptr),
    allOrdersModel(nullptr),
    editOrderIdLineEdit(nullptr),
    editKontrahentLineEdit(nullptr),
    editOrderDateTimeEdit(nullptr),
    editOrderStatusComboBox(nullptr),
    editedOrderItemsTableView(nullptr),
    editedOrderItemsModel(nullptr),
    editOrderItemProductSearchLineEdit(nullptr),
    editOrderItemProductCompleter(nullptr),
    editOrderItemProductCompleterModel(nullptr),
    editOrderItemProductIdLabel(nullptr),
    editOrderItemProductNameLabel(nullptr),
    editOrderItemQuantitySpinBox(nullptr),
    addProductToExistingOrderButton(nullptr),
    deleteOrderItemFromExistingOrderButton(nullptr),
    updateOrderItemQuantityButton(nullptr),
    saveOrderChangesButton(nullptr),
    deleteOrderButton(nullptr),
    refreshOrdersButton(nullptr)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    tabWidget = new QTabWidget(this); // Utworzenie QTabWidget
    mainLayout->addWidget(tabWidget);

    setupCreateOrderTab(); // Konfiguracja istniejącej zakładki
    setupEditOrdersTab();  // Konfiguracja nowej zakładki
}

KsiegowoscView::~KsiegowoscView()
{
    // Obiekty będące dziećmi KsiegowoscView zostaną automatycznie usunięte.
}

// --- Funkcje dla zakładki "Nowe Zamówienie" ---
void KsiegowoscView::setupCreateOrderTab()
{
    QWidget *createOrderTab = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(createOrderTab);

    // --- Sekcja ogólnych danych zamówienia ---
    QHBoxLayout *orderInfoLayout = new QHBoxLayout();
    kontrahentLabel = new QLabel("Kontrahent:", this);
    kontrahentLineEdit = new QLineEdit(this);
    kontrahentLineEdit->setPlaceholderText("Wpisz nazwę kontrahenta");
    orderDateLabel = new QLabel("Data zamówienia: " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"), this); // Automatyczna data

    orderInfoLayout->addWidget(kontrahentLabel);
    orderInfoLayout->addWidget(kontrahentLineEdit);
    orderInfoLayout->addWidget(orderDateLabel);
    orderInfoLayout->addStretch();
    layout->addLayout(orderInfoLayout);

    // --- Sekcja wyszukiwania i dodawania produktów ---
    layout->addWidget(new QLabel("Dodaj produkty do zamówienia:", this));
    QHBoxLayout *productInputLayout = new QHBoxLayout();
    productSearchLabel = new QLabel("Nazwa produktu:", this);
    productSearchLineEdit = new QLineEdit(this);
    productSearchLineEdit->setPlaceholderText("Wyszukaj produkt...");

    // QCompleter dla produktów (pobieramy dane przez ProductDao)
    productCompleterModel = new QSqlQueryModel(this);
    productCompleterModel->setQuery("SELECT name, id FROM products", m_system->db); // Używamy m_system->db
    if (productCompleterModel->lastError().isValid()) {
        QMessageBox::warning(this, "Błąd Bazy Danych",
                             "Nie można załadować nazw produktów do autouzupełniania (Księgowość): " + productCompleterModel->lastError().text());
        qDebug() << "Błąd QCompleter SQL (Księgowość):" << productCompleterModel->lastError().text();
    }
    productCompleter = new QCompleter(productCompleterModel, this);
    productCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    productCompleter->setCompletionColumn(0); // Kolumna z nazwami
    productSearchLineEdit->setCompleter(productCompleter);

    connect(productSearchLineEdit, &QLineEdit::textChanged, this, &KsiegowoscView::onProductSearchTextChanged);
    connect(productCompleter, QOverload<const QString &>::of(&QCompleter::activated),
            this, &KsiegowoscView::onProductSelected);

    currentProductIdLabel = new QLabel("ID Produktu:", this);
    currentProductNameLabel = new QLabel("Nazwa:", this); // Będzie wyświetlać nazwę po wybraniu
    productQuantitySpinBox = new QSpinBox(this);
    productQuantitySpinBox->setMinimum(1);
    productQuantitySpinBox->setMaximum(99999);
    productQuantitySpinBox->setValue(1);
    addProductToOrderButton = new QPushButton("Dodaj produkt do zamówienia", this);
    connect(addProductToOrderButton, &QPushButton::clicked, this, &KsiegowoscView::onAddProductToOrderClicked);

    productInputLayout->addWidget(productSearchLabel);
    productInputLayout->addWidget(productSearchLineEdit);
    productInputLayout->addWidget(currentProductIdLabel);
    productInputLayout->addWidget(currentProductNameLabel);
    productInputLayout->addWidget(new QLabel("Ilość:", this));
    productInputLayout->addWidget(productQuantitySpinBox);
    productInputLayout->addWidget(addProductToOrderButton);
    productInputLayout->addStretch();
    layout->addLayout(productInputLayout);

    // --- Tabela pozycji zamówienia (tymczasowa) ---
    orderItemsLabel = new QLabel("Produkty w bieżącym zamówieniu:", this);
    orderItemsTableView = new QTableView(this);
    orderItemsModel = new QStandardItemModel(0, 3, this); // ID_produktu, Nazwa, Ilość
    orderItemsModel->setHeaderData(0, Qt::Horizontal, "ID Produktu");
    orderItemsModel->setHeaderData(1, Qt::Horizontal, "Nazwa Produktu");
    orderItemsModel->setHeaderData(2, Qt::Horizontal, "Ilość");
    orderItemsTableView->setModel(orderItemsModel);
    orderItemsTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    orderItemsTableView->horizontalHeader()->setStretchLastSection(true);
    orderItemsTableView->verticalHeader()->setVisible(false);
    layout->addWidget(orderItemsLabel);
    layout->addWidget(orderItemsTableView);

    // --- Przycisk utworzenia zamówienia ---
    createOrderButton = new QPushButton("Utwórz Zamówienie", this);
    connect(createOrderButton, &QPushButton::clicked, this, &KsiegowoscView::onCreateOrderClicked);
    layout->addWidget(createOrderButton);

    layout->addStretch(); // Rozciągnij na końcu
    createOrderTab->setLayout(layout);
    tabWidget->addTab(createOrderTab, "Nowe Zamówienie");
}

void KsiegowoscView::onProductSearchTextChanged(const QString &text)
{
    Q_UNUSED(text);
}

void KsiegowoscView::onProductSelected(const QString &text)
{
    // Używamy ProductDao do pobrania ID i nazwy produktu
    Product* product = m_system->getProductDao()->getProductById(m_system->getProductDao()->getProductIdByName(text));

    if (product) {
        currentProductIdLabel->setText("ID Produktu: " + QString::number(product->get_id()));
        currentProductNameLabel->setText("Nazwa: " + product->get_name());
        delete product; // Zwolnij pamięć po pobranym obiekcie
    } else {
        currentProductIdLabel->setText("ID Produktu: Nie znaleziono");
        currentProductNameLabel->setText("Nazwa: N/A");
        QMessageBox::warning(this, "Błąd", "Nie znaleziono produktu o nazwie: " + text);
        qDebug() << "KsiegowoscView: Produkt '" << text << "' nie znaleziono w bazie danych (przez DAO).";
    }
}

void KsiegowoscView::onAddProductToOrderClicked()
{
    QString productName = productSearchLineEdit->text().trimmed();
    int quantity = productQuantitySpinBox->value();

    if (productName.isEmpty()) {
        QMessageBox::warning(this, "Błąd", "Proszę wybrać lub wpisać nazwę produktu.");
        return;
    }
    if (quantity <= 0) {
        QMessageBox::warning(this, "Błąd", "Ilość musi być większa od zera.");
        return;
    }

    // Pobierz ID produktu za pomocą ProductDao
    int productId = m_system->getProductDao()->getProductIdByName(productName);
    if (productId == -1) {
        QMessageBox::warning(this, "Błąd", "Produkt '" + productName + "' nie istnieje w bazie danych.");
        return;
    }

    // Sprawdź, czy produkt już jest w liście
    for (int i = 0; i < orderItemsModel->rowCount(); ++i) {
        if (orderItemsModel->item(i, 0)->text().toInt() == productId) {
            QMessageBox::information(this, "Informacja", "Ten produkt jest już w zamówieniu. Zmień jego ilość lub dodaj inny.");
            return;
        }
    }

    QList<QStandardItem*> rowItems;
    rowItems << new QStandardItem(QString::number(productId)); // Kolumna 0: ID Produktu
    rowItems << new QStandardItem(productName);                // Kolumna 1: Nazwa Produktu
    rowItems << new QStandardItem(QString::number(quantity));  // Kolumna 2: Ilość
    orderItemsModel->appendRow(rowItems);

    QMessageBox::information(this, "Dodano", "Produkt '" + productName + "' dodano do zamówienia.");

    // Wyczyść pola
    productSearchLineEdit->clear();
    currentProductIdLabel->setText("ID Produktu:");
    currentProductNameLabel->setText("Nazwa:");
    productQuantitySpinBox->setValue(1);
}

void KsiegowoscView::onCreateOrderClicked()
{
    QString kontrahent = kontrahentLineEdit->text().trimmed();
    if (kontrahent.isEmpty()) {
        QMessageBox::warning(this, "Błąd", "Proszę podać nazwę kontrahenta.");
        return;
    }
    if (orderItemsModel->rowCount() == 0) {
        QMessageBox::warning(this, "Błąd", "Zamówienie musi zawierać produkty. Proszę dodać produkty do listy.");
        return;
    }

    // --- Rozpoczęcie transakcji --- (zarządzane przez systemWarehouse, tu tylko wywołujemy)
    if (!m_system->db.transaction()) {
        QMessageBox::critical(this, "Błąd Bazy Danych", "Nie można rozpocząć transakcji: " + m_system->db.lastError().text());
        qDebug() << "KsiegowoscView: Błąd rozpoczęcia transakcji:" << m_system->db.lastError().text();
        return;
    }

    bool success = true;
    int newOrderId = -1;

    // 1. Wstaw nowe zamówienie do tabeli 'zamowienia' za pomocą OrderDao
    Order newOrder(-1, kontrahent, QDateTime::currentDateTime(), "Nowe"); // -1 jako placeholder dla ID, które zostanie nadane przez bazę
    newOrderId = m_system->getOrderDao()->addOrder(newOrder);

    if (newOrderId != -1) {
        qDebug() << "KsiegowoscView: Utworzono nowe zamówienie z ID:" << newOrderId;
    } else {
        // Komunikat o błędzie już wyświetla OrderDao
        qDebug() << "KsiegowoscView: Nie mozna utworzyc zamowienia przez OrderDao.";
        success = false;
    }

    // 2. Wstaw produkty do tabeli 'order_items' za pomocą OrderItemDao, jeśli zamówienie zostało utworzone
    if (success && newOrderId != -1) {
        for (int i = 0; i < orderItemsModel->rowCount(); ++i) {
            int productId = orderItemsModel->item(i, 0)->text().toInt();
            QString productName = orderItemsModel->item(i, 1)->text(); // Potrzebne do OrderItem encji
            int quantity = orderItemsModel->item(i, 2)->text().toInt();

            OrderItem newOrderItem(newOrderId, productId, productName, quantity);

            if (!m_system->getOrderItemDao()->addOrderItem(newOrderItem)) {
                QMessageBox::critical(this, "Błąd Bazy Danych", "Nie można dodać pozycji zamówienia dla produktu ID " + QString::number(productId) + ".");
                qDebug() << "KsiegowoscView: Błąd dodawania pozycji zamówienia przez OrderItemDao.";
                success = false;
                break;
            }
        }
    } else {
        success = false; // Jeśli OrderDao zawiodło
    }

    // --- Zakończenie transakcji ---
    if (success) {
        m_system->db.commit(); // Zatwierdź transakcję
        QMessageBox::information(this, "Sukces", "Zamówienie zostało utworzone pomyślnie (ID: " + QString::number(newOrderId) + ").");
        // Wyczyść UI po udanym zamówieniu
        kontrahentLineEdit->clear();
        orderItemsModel->clear();
        orderItemsModel->setHeaderData(0, Qt::Horizontal, "ID Produktu");
        orderItemsModel->setHeaderData(1, Qt::Horizontal, "Nazwa Produktu");
        orderItemsModel->setHeaderData(2, Qt::Horizontal, "Ilość");
        productSearchLineEdit->clear();
        currentProductIdLabel->setText("ID Produktu:");
        currentProductNameLabel->setText("Nazwa:");
        productQuantitySpinBox->setValue(1);

        emit databaseUpdated(); // Emituj sygnał, aby MainWindow odświeżył listę zamówień (w tym zakładkę edycji)
        loadAllOrdersTable(); // Odśwież tabelę zamówień w zakładce edycji
    } else {
        m_system->db.rollback(); // Wycofaj transakcję
        QMessageBox::critical(this, "Błąd", "Wystąpił błąd podczas tworzenia zamówienia. Transakcja została wycofana.");
    }
}

// Ta funkcja jest nieużywana i może być usunięta, jeśli nie ma innych odniesień
/*
void KsiegowoscView::loadProductDetails(int productId)
{
    Q_UNUSED(productId);
}
*/


// --- Funkcje dla zakładki "Edytuj Zamówienia" (NOWE) ---
void KsiegowoscView::setupEditOrdersTab()
{
    QWidget *editOrdersTab = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(editOrdersTab);

    // Tabela wszystkich zamówień
    mainLayout->addWidget(new QLabel("Wybierz zamówienie do edycji:", this));
    allOrdersTableView = new QTableView(this);
    allOrdersModel = new QSqlQueryModel(this);
    allOrdersTableView->setModel(allOrdersModel);
    allOrdersTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    allOrdersTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    allOrdersTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    allOrdersTableView->horizontalHeader()->setStretchLastSection(true);
    mainLayout->addWidget(allOrdersTableView);

    // Formularz edycji danych zamówienia
    mainLayout->addWidget(new QLabel("Edytuj szczegóły zamówienia:", this));
    QGridLayout *orderDetailsLayout = new QGridLayout();
    orderDetailsLayout->addWidget(new QLabel("ID Zamówienia:", this), 0, 0);
    editOrderIdLineEdit = new QLineEdit(this);
    editOrderIdLineEdit->setReadOnly(true);
    orderDetailsLayout->addWidget(editOrderIdLineEdit, 0, 1);

    orderDetailsLayout->addWidget(new QLabel("Kontrahent:", this), 1, 0);
    editKontrahentLineEdit = new QLineEdit(this);
    orderDetailsLayout->addWidget(editKontrahentLineEdit, 1, 1);

    orderDetailsLayout->addWidget(new QLabel("Data Zamówienia:", this), 2, 0);
    editOrderDateTimeEdit = new QDateTimeEdit(this);
    editOrderDateTimeEdit->setCalendarPopup(true);
    editOrderDateTimeEdit->setDisplayFormat("yyyy-MM-dd hh:mm:ss");
    orderDetailsLayout->addWidget(editOrderDateTimeEdit, 2, 1);

    orderDetailsLayout->addWidget(new QLabel("Status:", this), 3, 0);
    editOrderStatusComboBox = new QComboBox(this);
    editOrderStatusComboBox->addItem("Nowe");
    editOrderStatusComboBox->addItem("W realizacji");
    editOrderStatusComboBox->addItem("Zrealizowane");
    editOrderStatusComboBox->addItem("Anulowane");
    orderDetailsLayout->addWidget(editOrderStatusComboBox, 3, 1);
    mainLayout->addLayout(orderDetailsLayout);

    // Tabela pozycji edytowanego zamówienia
    mainLayout->addWidget(new QLabel("Pozycje w zamówieniu:", this));
    editedOrderItemsTableView = new QTableView(this);
    editedOrderItemsModel = new QStandardItemModel(0, 3, this); // product_id, name, quantity
    editedOrderItemsModel->setHeaderData(0, Qt::Horizontal, "ID Prod.");
    editedOrderItemsModel->setHeaderData(1, Qt::Horizontal, "Nazwa Prod.");
    editedOrderItemsModel->setHeaderData(2, Qt::Horizontal, "Ilość");
    editedOrderItemsTableView->setModel(editedOrderItemsModel);
    editedOrderItemsTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    editedOrderItemsTableView->horizontalHeader()->setStretchLastSection(true);
    editedOrderItemsTableView->verticalHeader()->setVisible(false);
    mainLayout->addWidget(editedOrderItemsTableView);

    // Dodawanie/edycja/usuwanie pozycji w edytowanym zamówieniu
    mainLayout->addWidget(new QLabel("Dodaj/edytuj/usuń pozycję:", this));
    QGridLayout *editOrderItemsLayout = new QGridLayout();
    editOrderItemsLayout->addWidget(new QLabel("Nazwa produktu:", this), 0, 0);
    editOrderItemProductSearchLineEdit = new QLineEdit(this);
    editOrderItemProductSearchLineEdit->setPlaceholderText("Wyszukaj produkt...");
    editOrderItemsLayout