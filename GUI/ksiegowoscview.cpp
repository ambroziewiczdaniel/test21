#include "ksiegowoscview.h"
#include <QDebug>
#include <QSqlRecord> // Do QSqlQuery::record()
#include <QHeaderView>
#include <QGridLayout> // Potrzebne dla lepszego układu edycji zamówień

// Konstruktor klasy KsiegowoscView
KsiegowoscView::KsiegowoscView(systemWarehouse* system, QWidget *parent) :
    QWidget(parent),
    m_system(system), // Inicjalizacja wskaźnika do systemu
    tabWidget(nullptr),
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
    tabWidget = new QTabWidget(this);
    mainLayout->addWidget(tabWidget);

    setupCreateOrderTab(); // Konfiguracja zakładki "Nowe Zamówienie"
    setupEditOrdersTab();  // Konfiguracja zakładki "Edytuj Zamówienia"
}

// Destruktor klasy KsiegowoscView
KsiegowoscView::~KsiegowoscView()
{
    // Obiekty będące dziećmi KsiegowoscView zostaną automatycznie usunięte.
}
void KsiegowoscView::refreshData()
{
    qDebug() << "KsiegowoscView: Odświeżam dane.";
    loadAllOrdersTable(); // Wywołuje prywatną metodę ładowania tabeli
}
// --- Metody dla zakładki "Nowe Zamówienie" ---
void KsiegowoscView::setupCreateOrderTab()
{
    QWidget *createOrderTab = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(createOrderTab);

    // Sekcja ogólnych danych zamówienia
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

    // Sekcja wyszukiwania i dodawania produktów
    layout->addWidget(new QLabel("Dodaj produkty do zamówienia:", this));
    QHBoxLayout *productInputLayout = new QHBoxLayout();
    productSearchLabel = new QLabel("Nazwa produktu:", this);
    productSearchLineEdit = new QLineEdit(this);
    productSearchLineEdit->setPlaceholderText("Wyszukaj produkt...");

    // QCompleter dla produktów (pobieramy dane przez ProductDao za pomocą m_system->getDb())
    productCompleterModel = new QSqlQueryModel(this);
    productCompleterModel->setQuery("SELECT name, id FROM products", m_system->getDb());
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

    // Tabela pozycji zamówienia (tymczasowa)
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

    // Przycisk utworzenia zamówienia
    createOrderButton = new QPushButton("Utwórz Zamówienie", this);
    connect(createOrderButton, &QPushButton::clicked, this, &KsiegowoscView::onCreateOrderClicked);
    layout->addWidget(createOrderButton);

    layout->addStretch();
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
    int productId = m_system->getProductDao()->getProductIdByName(text);
    Product* product = nullptr;
    if (productId != -1) {
        product = m_system->getProductDao()->getProductById(productId);
    }

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

    // Rozpoczęcie transakcji (zarządzane przez systemWarehouse, tu tylko wywołujemy)
    if (!m_system->getDb().transaction()) { // Używamy m_system->getDb()
        QMessageBox::critical(this, "Błąd Bazy Danych", "Nie można rozpocząć transakcji: " + m_system->getDb().lastError().text());
        qDebug() << "KsiegowoscView: Błąd rozpoczęcia transakcji:" << m_system->getDb().lastError().text();
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
            QString productName = orderItemsModel->item(i, 1)->text();
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
        m_system->getDb().commit(); // Używamy m_system->getDb()
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
        m_system->getDb().rollback(); // Używamy m_system->getDb()
        QMessageBox::critical(this, "Błąd", "Wystąpił błąd podczas tworzenia zamówienia. Transakcja została wycofana.");
    }
}


// --- Metody dla zakładki "Edytuj Zamówienia" ---
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
    editOrderItemsLayout->addWidget(editOrderItemProductSearchLineEdit, 0, 1, 1, 3);

    // QCompleter dla produktów do edycji zamówień (pobieramy dane przez ProductDao za pomocą m_system->getDb())
    editOrderItemProductCompleterModel = new QSqlQueryModel(this);
    editOrderItemProductCompleterModel->setQuery("SELECT name, id FROM products", m_system->getDb());
    if (editOrderItemProductCompleterModel->lastError().isValid()) {
        QMessageBox::warning(this, "Błąd Bazy Danych",
                             "Nie można załadować nazw produktów do autouzupełniania (Edycja Zamówień): " + editOrderItemProductCompleterModel->lastError().text());
        qDebug() << "Błąd QCompleter SQL (Edycja Zamówień):" << editOrderItemProductCompleterModel->lastError().text();
    }
    editOrderItemProductCompleter = new QCompleter(editOrderItemProductCompleterModel, this);
    editOrderItemProductCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    editOrderItemProductCompleter->setCompletionColumn(0);
    editOrderItemProductSearchLineEdit->setCompleter(editOrderItemProductCompleter);

    connect(editOrderItemProductCompleter, QOverload<const QString &>::of(&QCompleter::activated),
            [this](const QString &text){
                int productId = m_system->getProductDao()->getProductIdByName(text);
                Product* product = nullptr;
                if (productId != -1) {
                    product = m_system->getProductDao()->getProductById(productId);
                }

                if (product) {
                    editOrderItemProductIdLabel->setText("ID: " + QString::number(product->get_id()));
                    editOrderItemProductNameLabel->setText("Nazwa: " + product->get_name());
                    delete product;
                } else {
                    editOrderItemProductIdLabel->setText("ID: N/A");
                    editOrderItemProductNameLabel->setText("Nazwa: N/A");
                }
            });


    editOrderItemProductIdLabel = new QLabel("ID:", this);
    editOrderItemsLayout->addWidget(editOrderItemProductIdLabel, 1, 0);
    editOrderItemProductNameLabel = new QLabel("Nazwa:", this);
    editOrderItemsLayout->addWidget(editOrderItemProductNameLabel, 1, 1);
    editOrderItemsLayout->addWidget(new QLabel("Ilość:", this), 1, 2);
    editOrderItemQuantitySpinBox = new QSpinBox(this);
    editOrderItemQuantitySpinBox->setMinimum(1);
    editOrderItemQuantitySpinBox->setMaximum(99999);
    editOrderItemsLayout->addWidget(editOrderItemQuantitySpinBox, 1, 3);

    addProductToExistingOrderButton = new QPushButton("Dodaj Produkt do Zamówienia", this);
    editOrderItemsLayout->addWidget(addProductToExistingOrderButton, 2, 0, 1, 2);
    connect(addProductToExistingOrderButton, &QPushButton::clicked, this, &KsiegowoscView::onAddProductToExistingOrderClicked);

    deleteOrderItemFromExistingOrderButton = new QPushButton("Usuń Wybraną Pozycję", this);
    editOrderItemsLayout->addWidget(deleteOrderItemFromExistingOrderButton, 2, 2);
    connect(deleteOrderItemFromExistingOrderButton, &QPushButton::clicked, this, &KsiegowoscView::onDeleteOrderItemFromExistingOrderClicked);

    updateOrderItemQuantityButton = new QPushButton("Aktualizuj Ilość Wybranej Pozycji", this);
    editOrderItemsLayout->addWidget(updateOrderItemQuantityButton, 2, 3);
    connect(updateOrderItemQuantityButton, &QPushButton::clicked, this, &KsiegowoscView::onUpdateOrderItemQuantityClicked);


    mainLayout->addLayout(editOrderItemsLayout);

    // Przyciski akcji dla zamówienia
    QHBoxLayout *actionButtonsLayout = new QHBoxLayout();
    saveOrderChangesButton = new QPushButton("Zapisz Zmiany Zamówienia", this);
    saveOrderChangesButton->setEnabled(false);
    connect(saveOrderChangesButton, &QPushButton::clicked, this, &KsiegowoscView::onSaveOrderChangesButtonClicked);
    actionButtonsLayout->addWidget(saveOrderChangesButton);

    deleteOrderButton = new QPushButton("Usuń Zamówienie", this);
    deleteOrderButton->setEnabled(false);
    connect(deleteOrderButton, &QPushButton::clicked, this, &KsiegowoscView::onDeleteOrderButtonClicked);
    actionButtonsLayout->addWidget(deleteOrderButton);

    refreshOrdersButton = new QPushButton("Odśwież Listę Zamówień", this);
    connect(refreshOrdersButton, &QPushButton::clicked, this, &KsiegowoscView::onRefreshOrdersButtonClicked);
    actionButtonsLayout->addWidget(refreshOrdersButton);

    mainLayout->addLayout(actionButtonsLayout);
    mainLayout->addStretch();
    editOrdersTab->setLayout(mainLayout);
    tabWidget->addTab(editOrdersTab, "Edytuj Zamówienia");

    // Połączenie sygnału zaznaczenia wiersza z slotem
    connect(allOrdersTableView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &KsiegowoscView::onOrderSelectionChanged);

    loadAllOrdersTable(); // Załaduj dane przy starcie
}

void KsiegowoscView::loadAllOrdersTable()
{
    // Pobieramy QSqlQuery z systemWarehouse, które korzysta z OrderDao
    QSqlQuery query = m_system->getOrdersQuery();

    if (!query.exec()) {
        QMessageBox::warning(this, "Błąd Bazy Danych",
                             "Nie można załadować zamówień: " + query.lastError().text());
        qDebug() << "KsiegowoscView: Błąd ładowania zamówień:" << query.lastError().text();
        return;
    }

    allOrdersModel->setQuery(std::move(query));

    if (allOrdersModel->lastError().isValid()) {
        QMessageBox::warning(this, "Błąd Bazy Danych",
                             "Błąd po ustawieniu zapytania w modelu: " + allOrdersModel->lastError().text());
        qDebug() << "KsiegowoscView: Błąd ładowania zamówień (model):" << allOrdersModel->lastError().text();
    }

    allOrdersModel->setHeaderData(0, Qt::Horizontal, "ID Zam.");
    allOrdersModel->setHeaderData(1, Qt::Horizontal, "Kontrahent");
    allOrdersModel->setHeaderData(2, Qt::Horizontal, "Data");
    allOrdersModel->setHeaderData(3, Qt::Horizontal, "Status");
}

void KsiegowoscView::loadOrderDetails(int orderId)
{
    // Wyczyść poprzednie dane
    editedOrderItemsModel->clear();
    editedOrderItemsModel->setHeaderData(0, Qt::Horizontal, "ID Prod.");
    editedOrderItemsModel->setHeaderData(1, Qt::Horizontal, "Nazwa Prod.");
    editedOrderItemsModel->setHeaderData(2, Qt::Horizontal, "Ilość");

    // Załaduj główne dane zamówienia za pomocą OrderDao
    Order* order = m_system->getOrderDao()->getOrderById(orderId);
    if (order) {
        editOrderIdLineEdit->setText(QString::number(order->getId()));
        editKontrahentLineEdit->setText(order->getKontrahent());
        editOrderDateTimeEdit->setDateTime(order->getData());
        editOrderStatusComboBox->setCurrentText(order->getStatus());
        delete order; // Zwolnij pamięć

        saveOrderChangesButton->setEnabled(true);
        deleteOrderButton->setEnabled(true);
        addProductToExistingOrderButton->setEnabled(true);
        deleteOrderItemFromExistingOrderButton->setEnabled(true);
        updateOrderItemQuantityButton->setEnabled(true);

    } else {
        QMessageBox::warning(this, "Błąd", "Nie znaleziono zamówienia o ID: " + QString::number(orderId) + ".");
        qDebug() << "KsiegowoscView: Nie znaleziono zamówienia ID" << orderId << " (przez DAO).";
        // Wyczyść pola
        editOrderIdLineEdit->clear();
        editKontrahentLineEdit->clear();
        editOrderDateTimeEdit->clear();
        editOrderStatusComboBox->setCurrentIndex(-1);
        saveOrderChangesButton->setEnabled(false);
        deleteOrderButton->setEnabled(false);
        addProductToExistingOrderButton->setEnabled(false);
        deleteOrderItemFromExistingOrderButton->setEnabled(false);
        updateOrderItemQuantityButton->setEnabled(false);
        return;
    }

    // Załaduj pozycje zamówienia za pomocą OrderItemDao
    QVector<OrderItem*> items = m_system->getOrderItemDao()->getOrderItemsByOrderId(orderId);
    if (!items.isEmpty()) {
        for (OrderItem* item : items) {
            QList<QStandardItem*> rowItems;
            rowItems << new QStandardItem(QString::number(item->getProductId()));
            rowItems << new QStandardItem(item->getProductName());
            rowItems << new QStandardItem(QString::number(item->getQuantity()));
            editedOrderItemsModel->appendRow(rowItems);
            delete item; // Zwolnij pamięć po każdym OrderItem
        }
    } else {
        qDebug() << "KsiegowoscView: Brak pozycji dla zamówienia ID" << orderId << " lub błąd (przez DAO).";
    }
}

void KsiegowoscView::onOrderSelectionChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);
    if (current.isValid()) {
        int orderId = allOrdersModel->data(allOrdersModel->index(current.row(), 0)).toInt();
        loadOrderDetails(orderId);
    } else {
        editOrderIdLineEdit->clear();
        editKontrahentLineEdit->clear();
        editOrderDateTimeEdit->clear();
        editOrderStatusComboBox->setCurrentIndex(-1);
        editedOrderItemsModel->clear();
        editedOrderItemsModel->setHeaderData(0, Qt::Horizontal, "ID Prod.");
        editedOrderItemsModel->setHeaderData(1, Qt::Horizontal, "Nazwa Prod.");
        editedOrderItemsModel->setHeaderData(2, Qt::Horizontal, "Ilość");

        saveOrderChangesButton->setEnabled(false);
        deleteOrderButton->setEnabled(false);
        addProductToExistingOrderButton->setEnabled(false);
        deleteOrderItemFromExistingOrderButton->setEnabled(false);
        updateOrderItemQuantityButton->setEnabled(false);
    }
}

void KsiegowoscView::onRefreshOrdersButtonClicked()
{
    loadAllOrdersTable();
    QMessageBox::information(this, "Odświeżono", "Lista zamówień została odświeżona.");
    onOrderSelectionChanged(QModelIndex(), QModelIndex());
}

void KsiegowoscView::onSaveOrderChangesButtonClicked()
{
    int orderId = editOrderIdLineEdit->text().toInt();
    QString newKontrahent = editKontrahentLineEdit->text().trimmed();
    QDateTime newDate = editOrderDateTimeEdit->dateTime();
    QString newStatus = editOrderStatusComboBox->currentText();

    if (newKontrahent.isEmpty()) {
        QMessageBox::warning(this, "Błąd", "Nazwa kontrahenta nie może być pusta.");
        return;
    }

    if (!m_system->getDb().transaction()) { // Rozpoczęcie transakcji
        QMessageBox::critical(this, "Błąd Bazy Danych", "Nie można rozpocząć transakcji: " + m_system->getDb().lastError().text());
        return;
    }

    bool success = true;

    // Aktualizuj główne dane zamówienia za pomocą OrderDao
    Order updatedOrder(orderId, newKontrahent, newDate, newStatus);
    if (!m_system->getOrderDao()->updateOrder(updatedOrder)) {
        qDebug() << "KsiegowoscView: Blad aktualizacji zamowienia przez OrderDao.";
        success = false;
    }

    // Aktualizacja pozycji zamówienia: usuwamy stare i dodajemy nowe
    if (success) {
        if (!m_system->getOrderItemDao()->deleteOrderItemsByOrderId(orderId)) {
            qDebug() << "KsiegowoscView: Blad usuwania starych pozycji zamowienia przez OrderItemDao.";
            success = false;
        }
    }

    if (success) {
        for (int i = 0; i < editedOrderItemsModel->rowCount(); ++i) {
            int productId = editedOrderItemsModel->item(i, 0)->text().toInt();
            QString productName = editedOrderItemsModel->item(i, 1)->text();
            int quantity = editedOrderItemsModel->item(i, 2)->text().toInt();

            OrderItem newItem(orderId, productId, productName, quantity);
            if (!m_system->getOrderItemDao()->addOrderItem(newItem)) {
                QMessageBox::critical(this, "Błąd Bazy Danych", "Nie można dodać nowej pozycji zamówienia (produkt ID " + QString::number(productId) + ").");
                qDebug() << "KsiegowoscView: Blad dodawania nowej pozycji zamowienia przez OrderItemDao.";
                success = false;
                break;
            }
        }
    }

    if (success) {
        m_system->getDb().commit(); // Zatwierdź transakcję
        QMessageBox::information(this, "Sukces", "Zamówienie ID " + QString::number(orderId) + " zostało zaktualizowane.");
        loadAllOrdersTable(); // Odśwież listę zamówień
        emit databaseUpdated();
    } else {
        m_system->getDb().rollback(); // Wycofaj transakcję
        QMessageBox::critical(this, "Błąd", "Wystąpił błąd podczas zapisywania zmian w zamówieniu. Transakcja została wycofana.");
    }
}

void KsiegowoscView::onDeleteOrderButtonClicked()
{
    int orderId = editOrderIdLineEdit->text().toInt();
    if (orderId == 0) {
        QMessageBox::warning(this, "Błąd", "Nie wybrano zamówienia do usunięcia.");
        return;
    }

    if (QMessageBox::question(this, "Potwierdź Usunięcie",
                              "Czy na pewno chcesz usunąć zamówienie ID " + QString::number(orderId) + "? "
                                                                                                       "Spowoduje to również usunięcie wszystkich pozycji w tym zamówieniu.",
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) {
        return;
    }

    if (!m_system->getDb().transaction()) { // Rozpoczęcie transakcji
        QMessageBox::critical(this, "Błąd Bazy Danych", "Nie można rozpocząć transakcji: " + m_system->getDb().lastError().text());
        return;
    }

    bool success = true;

    // Usuń pozycje zamówienia za pomocą OrderItemDao (jeśli nie masz CASCADE DELETE)
    if (!m_system->getOrderItemDao()->deleteOrderItemsByOrderId(orderId)) {
        qDebug() << "KsiegowoscView: Blad usuwania pozycji dla zamowienia przez OrderItemDao.";
        success = false;
    }

    if (success) {
        // Usuń zamówienie za pomocą OrderDao
        if (!m_system->getOrderDao()->deleteOrder(orderId)) {
            qDebug() << "KsiegowoscView: Blad usuwania zamowienia przez OrderDao.";
            success = false;
        }
    }

    if (success) {
        m_system->getDb().commit(); // Zatwierdź transakcję
        QMessageBox::information(this, "Sukces", "Zamówienie ID " + QString::number(orderId) + " zostało pomyślnie usunięte.");
        loadAllOrdersTable();
        emit databaseUpdated();
        onOrderSelectionChanged(QModelIndex(), QModelIndex()); // Wyczyść formularz
    } else {
        m_system->getDb().rollback(); // Wycofaj transakcję
        QMessageBox::critical(this, "Błąd", "Wystąpił błąd podczas usuwania zamówienia. Transakcja została wycofana.");
    }
}

void KsiegowoscView::onAddProductToExistingOrderClicked()
{
    QString productName = editOrderItemProductSearchLineEdit->text().trimmed();
    int quantity = editOrderItemQuantitySpinBox->value();
    int orderId = editOrderIdLineEdit->text().toInt();

    if (orderId == 0) {
        QMessageBox::warning(this, "Błąd", "Proszę najpierw wybrać zamówienie do edycji.");
        return;
    }
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

    // Sprawdź, czy produkt już jest w liście pozycji edytowanego zamówienia
    for (int i = 0; i < editedOrderItemsModel->rowCount(); ++i) {
        if (editedOrderItemsModel->item(i, 0)->text().toInt() == productId) {
            QMessageBox::information(this, "Informacja", "Ten produkt jest już w zamówieniu. Zmień jego ilość zamiast dodawać ponownie.");
            return;
        }
    }

    QList<QStandardItem*> rowItems;
    rowItems << new QStandardItem(QString::number(productId));
    rowItems << new QStandardItem(productName);
    rowItems << new QStandardItem(QString::number(quantity));
    editedOrderItemsModel->appendRow(rowItems);

    QMessageBox::information(this, "Dodano", "Produkt '" + productName + "' dodano do edytowanego zamówienia. Pamiętaj, aby zapisać zmiany!");

    editOrderItemProductSearchLineEdit->clear();
    editOrderItemProductIdLabel->setText("ID:");
    editOrderItemProductNameLabel->setText("Nazwa:");
    editOrderItemQuantitySpinBox->setValue(1);
}

void KsiegowoscView::onDeleteOrderItemFromExistingOrderClicked()
{
    QModelIndex currentIndex = editedOrderItemsTableView->currentIndex();
    if (!currentIndex.isValid()) {
        QMessageBox::warning(this, "Błąd", "Proszę wybrać pozycję do usunięcia z listy.");
        return;
    }

    if (QMessageBox::question(this, "Potwierdź Usunięcie",
                              "Czy na pewno chcesz usunąć wybraną pozycję z zamówienia?",
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) {
        return;
    }

    editedOrderItemsModel->removeRow(currentIndex.row());
    QMessageBox::information(this, "Usunięto", "Pozycja została usunięta z listy zamówienia. Pamiętaj, aby zapisać zmiany!");
}

void KsiegowoscView::onUpdateOrderItemQuantityClicked()
{
    QModelIndex currentIndex = editedOrderItemsTableView->currentIndex();
    if (!currentIndex.isValid()) {
        QMessageBox::warning(this, "Błąd", "Proszę wybrać pozycję do aktualizacji ilości.");
        return;
    }

    int newQuantity = editOrderItemQuantitySpinBox->value();
    if (newQuantity <= 0) {
        QMessageBox::warning(this, "Błąd", "Nowa ilość musi być większa od zera.");
        return;
    }

    editedOrderItemsModel->item(currentIndex.row(), 2)->setText(QString::number(newQuantity));
    QMessageBox::information(this, "Zaktualizowano", "Ilość pozycji została zaktualizowana. Pamiętaj, aby zapisać zmiany!");

    editOrderItemQuantitySpinBox->setValue(1);
}
