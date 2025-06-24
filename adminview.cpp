#include "adminview.h"
#include <QDebug>
#include <QSqlRecord>
#include <QGridLayout>
#include <QInputDialog> // Nadal może być przydatne

// Zmieniono konstruktor, aby przyjmował systemWarehouse*
AdminView::AdminView(systemWarehouse *system, QWidget *parent) :
    QWidget(parent),
    m_system(system), // Inicjalizujemy wskaźnik do systemu
    tabWidget(nullptr),
    // Inicjalizacja pozostałych pól...
    productNameLineEdit(nullptr),
    minQuantitySpinBox(nullptr),
    addProductButton(nullptr),
    userLoginLineEdit(nullptr),
    userPasswordLineEdit(nullptr),
    userRoleComboBox(nullptr),
    addUserButton(nullptr),
    stockProductNameLineEdit(nullptr),
    stockProductCompleter(nullptr),
    stockProductCompleterModel(nullptr),
    currentLocationsLabel(nullptr),
    regalSpinBox(nullptr),
    polkaSpinBox(nullptr),
    kolumnaSpinBox(nullptr),
    newIloscSpinBox(nullptr),
    updateLocationStockButton(nullptr),
    productsTableView(nullptr),
    productsModel(nullptr),
    editProductIdLineEdit(nullptr),
    editProductNameLineEdit(nullptr),
    editMinQuantitySpinBox(nullptr),
    editProductButton(nullptr),
    deleteProductButton(nullptr),
    refreshProductsButton(nullptr)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    tabWidget = new QTabWidget(this);
    mainLayout->addWidget(tabWidget);

    setupAddProductTab();
    setupAddUserTab();
    setupChangeStockTab();
    setupEditProductTab();
}

AdminView::~AdminView()
{
    // Obiekty będące dziećmi AdminView zostaną automatycznie usunięte.
}

void AdminView::setupAddProductTab()
{
    QWidget *addProductTab = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(addProductTab);

    layout->addWidget(new QLabel("Nazwa produktu:", this));
    productNameLineEdit = new QLineEdit(this);
    productNameLineEdit->setPlaceholderText("Wpisz nazwę nowego produktu");
    layout->addWidget(productNameLineEdit);

    layout->addWidget(new QLabel("Minimalna dopuszczalna ilość:", this));
    minQuantitySpinBox = new QSpinBox(this);
    minQuantitySpinBox->setMinimum(0);
    minQuantitySpinBox->setMaximum(99999);
    minQuantitySpinBox->setValue(0);
    layout->addWidget(minQuantitySpinBox);

    addProductButton = new QPushButton("Dodaj Produkt", this);
    connect(addProductButton, &QPushButton::clicked, this, &AdminView::onAddProductButtonClicked);
    layout->addWidget(addProductButton);

    layout->addStretch();
    addProductTab->setLayout(layout);
    tabWidget->addTab(addProductTab, "Dodaj Produkt");
}

void AdminView::onAddProductButtonClicked()
{
    QString productName = productNameLineEdit->text().trimmed();
    int minQuantity = minQuantitySpinBox->value();

    if (productName.isEmpty()) {
        QMessageBox::warning(this, "Błąd", "Nazwa produktu nie może być pusta.");
        return;
    }

    // Sprawdzamy, czy produkt o danej nazwie już istnieje za pomocą ProductDao
    if (m_system->getProductDao()->productExists(productName)) {
        QMessageBox::warning(this, "Błąd", "Produkt o nazwie '" + productName + "' już istnieje.");
        return;
    }

    // Generujemy ID (lub pozwalamy bazie na AUTOINCREMENT, jeśli tak jest skonfigurowana)
    // Jeśli 'id' w tabeli 'products' jest AUTOINCREMENT, możesz pominąć generowanie ID i przekazać np. 0.
    int newProductId = QRandomGenerator::global()->bounded(100000, 1000000);
    Product newProduct(newProductId, productName, minQuantity);

    // Używamy ProductDao do dodania produktu
    if (m_system->getProductDao()->addProduct(newProduct)) {
        QMessageBox::information(this, "Sukces", "Produkt '" + productName + "' (ID: " + QString::number(newProductId) + ") został dodany.");
        productNameLineEdit->clear();
        minQuantitySpinBox->setValue(0);
        emit databaseUpdated();
        loadProductsTable(); // Odśwież tabelę produktów w zakładce edycji
    } else {
        // Komunikat o błędzie już wyświetla ProductDao, więc tu tylko debug
        qDebug() << "AdminView: Nie mozna dodac produktu przez ProductDao.";
    }
}

void AdminView::setupAddUserTab()
{
    QWidget *addUserTab = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(addUserTab);

    layout->addWidget(new QLabel("Login:", this));
    userLoginLineEdit = new QLineEdit(this);
    userLoginLineEdit->setPlaceholderText("Wpisz login nowego użytkownika");
    layout->addWidget(userLoginLineEdit);

    layout->addWidget(new QLabel("Hasło:", this));
    userPasswordLineEdit = new QLineEdit(this);
    userPasswordLineEdit->setEchoMode(QLineEdit::Password);
    userPasswordLineEdit->setPlaceholderText("Wpisz hasło");
    layout->addWidget(userPasswordLineEdit);

    layout->addWidget(new QLabel("Rola:", this));
    userRoleComboBox = new QComboBox(this);
    userRoleComboBox->addItem("Magazynier");
    userRoleComboBox->addItem("Ksiegowosc");
    userRoleComboBox->addItem("Administrator");
    layout->addWidget(userRoleComboBox);

    addUserButton = new QPushButton("Dodaj Użytkownika", this);
    connect(addUserButton, &QPushButton::clicked, this, &AdminView::onAddUserButtonClicked);
    layout->addWidget(addUserButton);

    layout->addStretch();
    addUserTab->setLayout(layout);
    tabWidget->addTab(addUserTab, "Dodaj Użytkownika");
}

void AdminView::onAddUserButtonClicked()
{
    QString login = userLoginLineEdit->text().trimmed();
    QString password = userPasswordLineEdit->text();
    QString role = userRoleComboBox->currentText();

    if (login.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Błąd", "Login i hasło nie mogą być puste.");
        return;
    }

    // TODO: Tutaj powinna być integracja z UserDAO/UserService, ale na razie zostaje SQL
    QSqlQuery query(m_system->db); // Używamy m_system->db
    query.prepare("INSERT INTO users (login, password, role) VALUES (:login, :password, :role)");
    query.bindValue(":login", login);
    query.bindValue(":password", password);
    query.bindValue(":role", role);

    if (query.exec()) {
        QMessageBox::information(this, "Sukces", "Użytkownik '" + login + "' (" + role + ") został dodany.");
        userLoginLineEdit->clear();
        userPasswordLineEdit->clear();
        userRoleComboBox->setCurrentIndex(0);
        emit databaseUpdated();
    } else {
        QMessageBox::critical(this, "Błąd Bazy Danych", "Nie można dodać użytkownika: " + query.lastError().text());
        qDebug() << "Błąd dodawania użytkownika:" << query.lastError().text();
    }
}

void AdminView::setupChangeStockTab()
{
    QWidget *changeStockTab = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(changeStockTab);

    layout->addWidget(new QLabel("Nazwa produktu:", this));
    stockProductNameLineEdit = new QLineEdit(this);
    stockProductNameLineEdit->setPlaceholderText("Wpisz nazwę produktu do edycji stanu");
    layout->addWidget(stockProductNameLineEdit);

    stockProductCompleterModel = new QSqlQueryModel(this);
    // Używamy ProductDao do ładowania danych dla completera
    stockProductCompleterModel->setQuery("SELECT name FROM products", m_system->db); // Bezpośrednie SQL na razie zostaje
    if (stockProductCompleterModel->lastError().isValid()) {
        QMessageBox::warning(this, "Błąd Bazy Danych",
                             "Nie można załadować nazw produktów do autouzupełniania (Admin): " + stockProductCompleterModel->lastError().text());
        qDebug() << "Błąd QCompleter SQL (Admin):" << stockProductCompleterModel->lastError().text();
    }
    stockProductCompleter = new QCompleter(stockProductCompleterModel, this);
    stockProductCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    stockProductCompleter->setCompletionColumn(0);
    stockProductNameLineEdit->setCompleter(stockProductCompleter);

    connect(stockProductCompleter, QOverload<const QString &>::of(&QCompleter::activated),
            this, &AdminView::onProductForStockSelected);


    currentLocationsLabel = new QLabel("Bieżące lokalizacje dla produktu:", this);
    currentLocationsLabel->setWordWrap(true);
    layout->addWidget(currentLocationsLabel);

    QHBoxLayout *locationLayout = new QHBoxLayout();
    locationLayout->addWidget(new QLabel("Regał:", this));
    regalSpinBox = new QSpinBox(this);
    regalSpinBox->setRange(0, 999);
    locationLayout->addWidget(regalSpinBox);

    locationLayout->addWidget(new QLabel("Półka:", this));
    polkaSpinBox = new QSpinBox(this);
    polkaSpinBox->setRange(0, 999);
    locationLayout->addWidget(polkaSpinBox);

    locationLayout->addWidget(new QLabel("Kolumna:", this));
    kolumnaSpinBox = new QSpinBox(this);
    kolumnaSpinBox->setRange(0, 999);
    locationLayout->addWidget(kolumnaSpinBox);
    layout->addLayout(locationLayout);

    layout->addWidget(new QLabel("Nowa ilość w tej lokalizacji:", this));
    newIloscSpinBox = new QSpinBox(this);
    newIloscSpinBox->setRange(0, 99999);
    layout->addWidget(newIloscSpinBox);

    updateLocationStockButton = new QPushButton("Aktualizuj/Dodaj Ilość w Lokalizacji", this);
    connect(updateLocationStockButton, &QPushButton::clicked, this, &AdminView::onUpdateLocationStockButtonClicked);
    layout->addWidget(updateLocationStockButton);

    layout->addStretch();
    changeStockTab->setLayout(layout);
    tabWidget->addTab(changeStockTab, "Zmień Stan Magazynowy");
}

void AdminView::onProductForStockSelected(const QString &text)
{
    // Używamy ProductDao do pobrania ID produktu
    int productId = m_system->getProductDao()->getProductIdByName(text);

    if (productId != -1) {
        loadProductLocations(productId);
    } else {
        currentLocationsLabel->setText("Bieżące lokalizacje dla produktu: (Produkt nie znaleziony lub brak lokalizacji)");
        qDebug() << "AdminView: Nie znaleziono produktu o nazwie:" << text;
    }
}

void AdminView::loadProductLocations(int productId)
{
    QSqlQuery query(m_system->db); // Bezpośrednie SQL na razie zostaje
    query.prepare("SELECT regal, polka, kolumna, ilosc FROM locations WHERE id = :productId ORDER BY regal, polka, kolumna");
    query.bindValue(":productId", productId);

    QString locationsSummary;
    if (query.exec()) {
        if (query.next()) {
            locationsSummary = "Bieżące lokalizacje dla produktu (ID: " + QString::number(productId) + "):<br>";
            do {
                locationsSummary += QString("Regał: %1, Półka: %2, Kolumna: %3, Ilość: %4<br>")
                                         .arg(query.value("regal").toInt())
                                         .arg(query.value("polka").toInt())
                                         .arg(query.value("kolumna").toInt())
                                         .arg(query.value("ilosc").toInt());
            } while (query.next());
        } else {
            locationsSummary = "Bieżące lokalizacje dla produktu (ID: " + QString::number(productId) + "): Brak";
        }
    } else {
        locationsSummary = "Bieżące lokalizacje dla produktu (ID: " + QString::number(productId) + "): Błąd ładowania: " + query.lastError().text();
        qDebug() << "AdminView: Błąd ładowania lokalizacji dla produktu ID" << productId << ":" << query.lastError().text();
    }
    currentLocationsLabel->setText(locationsSummary);
}

void AdminView::onUpdateLocationStockButtonClicked()
{
    QString productName = stockProductNameLineEdit->text().trimmed();
    int regal = regalSpinBox->value();
    int polka = polkaSpinBox->value();
    int kolumna = kolumnaSpinBox->value();
    int newIlosc = newIloscSpinBox->value();

    if (productName.isEmpty()) {
        QMessageBox::warning(this, "Błąd", "Wybierz produkt.");
        return;
    }

    // Używamy ProductDao do pobrania ID produktu
    int productId = m_system->getProductDao()->getProductIdByName(productName);
    if (productId == -1) {
        QMessageBox::warning(this, "Błąd", "Produkt '" + productName + "' nie znaleziono w bazie danych.");
        return;
    }

    // TODO: To wymagałoby LocationDao do pełnej obiektowości
    QSqlQuery checkLocationQuery(m_system->db); // Bezpośrednie SQL na razie zostaje
    checkLocationQuery.prepare("SELECT COUNT(*) FROM locations WHERE id = :productId AND regal = :regal AND polka = :polka AND kolumna = :kolumna");
    checkLocationQuery.bindValue(":productId", productId);
    checkLocationQuery.bindValue(":regal", regal);
    checkLocationQuery.bindValue(":polka", polka);
    checkLocationQuery.bindValue(":kolumna", kolumna);

    if (!checkLocationQuery.exec()) {
        QMessageBox::critical(this, "Błąd Bazy Danych", "Błąd sprawdzania lokalizacji: " + checkLocationQuery.lastError().text());
        return;
    }
    checkLocationQuery.next();
    bool locationExists = (checkLocationQuery.value(0).toInt() > 0);

    QSqlQuery updateQuery(m_system->db); // Bezpośrednie SQL na razie zostaje
    if (locationExists) {
        updateQuery.prepare("UPDATE locations SET ilosc = :newIlosc WHERE id = :productId AND regal = :regal AND polka = :polka AND kolumna = :kolumna");
        QMessageBox::information(this, "Sukces", "Ilość produktu w lokalizacji (R" + QString::number(regal) + "-P" + QString::number(polka) + "-K" + QString::number(kolumna) + ") została zaktualizowana na " + QString::number(newIlosc) + ".");
    } else {
        updateQuery.prepare("INSERT INTO locations (id, regal, polka, kolumna, ilosc) VALUES (:productId, :regal, :polka, :kolumna, :newIlosc)");
        QMessageBox::information(this, "Sukces", "Dodano nową lokalizację dla produktu (R" + QString::number(regal) + "-P" + QString::number(polka) + "-K" + QString::number(kolumna) + ") z ilością " + QString::number(newIlosc) + ".");
    }

    updateQuery.bindValue(":productId", productId);
    updateQuery.bindValue(":regal", regal);
    updateQuery.bindValue(":polka", polka);
    updateQuery.bindValue(":kolumna", kolumna);
    updateQuery.bindValue(":newIlosc", newIlosc);

    if (updateQuery.exec()) {
        stockProductNameLineEdit->clear();
        regalSpinBox->setValue(0);
        polkaSpinBox->setValue(0);
        kolumnaSpinBox->setValue(0);
        newIloscSpinBox->setValue(0);
        currentLocationsLabel->setText("Bieżące lokalizacje dla produktu:");
        emit databaseUpdated();
    } else {
        QMessageBox::critical(this, "Błąd Aktualizacji", "Nie można zaktualizować/dodać ilości w lokalizacji: " + updateQuery.lastError().text());
        qDebug() << "Błąd aktualizacji/dodawania lokalizacji:" << updateQuery.lastError().text();
    }
}


void AdminView::setupEditProductTab()
{
    QWidget *editProductTab = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(editProductTab);

    mainLayout->addWidget(new QLabel("Wybierz produkt z listy, aby edytować lub usunąć:", this));
    productsTableView = new QTableView(this);
    productsModel = new QSqlQueryModel(this);
    productsTableView->setModel(productsModel);
    productsTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    productsTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    productsTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    productsTableView->horizontalHeader()->setStretchLastSection(true);
    mainLayout->addWidget(productsTableView);

    QGridLayout *editLayout = new QGridLayout();
    editLayout->addWidget(new QLabel("ID Produktu:", this), 0, 0);
    editProductIdLineEdit = new QLineEdit(this);
    editProductIdLineEdit->setReadOnly(true);
    editLayout->addWidget(editProductIdLineEdit, 0, 1);

    editLayout->addWidget(new QLabel("Nazwa produktu:", this), 1, 0);
    editProductNameLineEdit = new QLineEdit(this);
    editProductNameLineEdit->setPlaceholderText("Nazwa produktu");
    editLayout->addWidget(editProductNameLineEdit, 1, 1);

    editLayout->addWidget(new QLabel("Minimalna dopuszczalna ilość:", this), 2, 0);
    editMinQuantitySpinBox = new QSpinBox(this);
    editMinQuantitySpinBox->setMinimum(0);
    editMinQuantitySpinBox->setMaximum(99999);
    editLayout->addWidget(editMinQuantitySpinBox, 2, 1);

    mainLayout->addLayout(editLayout);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    editProductButton = new QPushButton("Zapisz Zmiany", this);
    editProductButton->setEnabled(false);
    connect(editProductButton, &QPushButton::clicked, this, &AdminView::onEditProductButtonClicked);
    buttonLayout->addWidget(editProductButton);

    deleteProductButton = new QPushButton("Usuń Produkt", this);
    deleteProductButton->setEnabled(false);
    connect(deleteProductButton, &QPushButton::clicked, this, &AdminView::onDeleteProductButtonClicked);
    buttonLayout->addWidget(deleteProductButton);

    refreshProductsButton = new QPushButton("Odśwież Listę Produktów", this);
    connect(refreshProductsButton, &QPushButton::clicked, this, &AdminView::onRefreshProductsButtonClicked);
    buttonLayout->addWidget(refreshProductsButton);

    mainLayout->addLayout(buttonLayout);

    mainLayout->addStretch();
    editProductTab->setLayout(mainLayout);
    tabWidget->addTab(editProductTab, "Edytuj Produkty");

    connect(productsTableView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &AdminView::onProductSelectionChanged);

    loadProductsTable();
}

void AdminView::loadProductsTable()
{
    // Odświeżamy QSqlQueryModel, aby pobierał dane bezpośrednio z ProductDao poprzez systemWarehouse
    // Ustawiamy zapytanie, które pobiera ID, nazwę i quantity (minimalną ilość)
    // ProductDao::getAllProducts() zwraca QVector<Product*>, więc nie możemy go bezpośrednio użyć z QSqlQueryModel.
    // Najprostszym sposobem na utrzymanie QSqlQueryModel jest, aby DAO mogło zwracać QSqlQuery
    // lub aby SystemWarehouse miało metodę, która ustawia QSqlQueryModel.
    // Na razie, dla uproszczenia refaktoryzacji, QSqlQueryModel będzie nadal używać QSqlDatabase,
    // ale możemy to później ulepszyć, tworząc QueryBuilder w DAO.
    productsModel->setQuery("SELECT id, name, quantity FROM products", m_system->db); // Używamy m_system->db
    if (productsModel->lastError().isValid()) {
        QMessageBox::warning(this, "Błąd Bazy Danych",
                             "Nie można załadować produktów: " + productsModel->lastError().text());
        qDebug() << "Błąd ładowania produktów (AdminView):" << productsModel->lastError().text();
    }

    productsModel->setHeaderData(0, Qt::Horizontal, "ID");
    productsModel->setHeaderData(1, Qt::Horizontal, "Nazwa");
    productsModel->setHeaderData(2, Qt::Horizontal, "Min. Ilość");
}

void AdminView::onProductSelectionChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);
    if (current.isValid()) {
        int productId = productsModel->data(productsModel->index(current.row(), 0)).toInt();
        // Pobieramy dane produktu za pomocą ProductDao
        Product* product = m_system->getProductDao()->getProductById(productId);
        if (product) {
            editProductIdLineEdit->setText(QString::number(product->get_id()));
            editProductNameLineEdit->setText(product->get_name());
            editMinQuantitySpinBox->setValue(product->get_quantity());
            delete product; // Zwolnij pamięć po pobranym obiekcie

            editProductButton->setEnabled(true);
            deleteProductButton->setEnabled(true);
        } else {
            QMessageBox::warning(this, "Błąd", "Nie można wczytać szczegółów produktu.");
            // Wyczyść pola
            editProductIdLineEdit->clear();
            editProductNameLineEdit->clear();
            editMinQuantitySpinBox->setValue(0);
            editProductButton->setEnabled(false);
            deleteProductButton->setEnabled(false);
        }
    } else {
        editProductIdLineEdit->clear();
        editProductNameLineEdit->clear();
        editMinQuantitySpinBox->setValue(0);

        editProductButton->setEnabled(false);
        deleteProductButton->setEnabled(false);
    }
}

void AdminView::onEditProductButtonClicked()
{
    int productId = editProductIdLineEdit->text().toInt();
    QString newProductName = editProductNameLineEdit->text().trimmed();
    int newMinQuantity = editMinQuantitySpinBox->value();

    if (newProductName.isEmpty()) {
        QMessageBox::warning(this, "Błąd", "Nazwa produktu nie może być pusta.");
        return;
    }

    // Sprawdzamy, czy nowa nazwa produktu nie koliduje z inną istniejącą (jeśli zmieniono nazwę)
    Product* existingProduct = m_system->getProductDao()->getProductById(productId);
    if (existingProduct && existingProduct->get_name() != newProductName) {
        if (m_system->getProductDao()->productExists(newProductName)) {
            QMessageBox::warning(this, "Błąd", "Produkt o nazwie '" + newProductName + "' już istnieje.");
            delete existingProduct;
            return;
        }
    }
    delete existingProduct; // Zwolnij pamięć

    Product updatedProduct(productId, newProductName, newMinQuantity);

    // Używamy ProductDao do aktualizacji produktu
    if (m_system->getProductDao()->updateProduct(updatedProduct)) {
        QMessageBox::information(this, "Sukces", "Produkt ID " + QString::number(productId) + " został zaktualizowany.");
        loadProductsTable(); // Odśwież tabelę
        emit databaseUpdated();
    } else {
        qDebug() << "AdminView: Nie mozna zaktualizowac produktu przez ProductDao.";
    }
}

void AdminView::onDeleteProductButtonClicked()
{
    int productId = editProductIdLineEdit->text().toInt();
    QString productName = editProductNameLineEdit->text();

    if (productId == 0) {
        QMessageBox::warning(this, "Błąd", "Nie wybrano produktu do usunięcia.");
        return;
    }

    if (QMessageBox::question(this, "Potwierdź Usunięcie",
                              "Czy na pewno chcesz usunąć produkt '" + productName + "' (ID: " + QString::number(productId) + ")? "
                              "Spowoduje to również usunięcie wszystkich powiązanych lokalizacji w magazynie.",
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) {
        return;
    }

    // Używamy ProductDao do usunięcia produktu
    if (m_system->getProductDao()->deleteProduct(productId)) {
        QMessageBox::information(this, "Sukces", "Produkt '" + productName + "' (ID: " + QString::number(productId) + ") został pomyślnie usunięty wraz z lokalizacjami.");
        loadProductsTable(); // Odśwież tabelę
        emit databaseUpdated();
        // Wyczyść pola edycji po usunięciu
        editProductIdLineEdit->clear();
        editProductNameLineEdit->clear();
        editMinQuantitySpinBox->setValue(0);
        editProductButton->setEnabled(false);
        deleteProductButton->setEnabled(false);
    } else {
        qDebug() << "AdminView: Nie mozna usunac produktu przez ProductDao.";
    }
}

void AdminView::onRefreshProductsButtonClicked()
{
    loadProductsTable();
    QMessageBox::information(this, "Odświeżono", "Lista produktów została odświeżona.");
}