#include "adminview.h"
#include <QDebug>
#include <QSqlRecord>
#include <QGridLayout>
#include <QInputDialog>

AdminView::AdminView(systemWarehouse *system, QWidget *parent) :
    QWidget(parent),
    m_system(system),
    tabWidget(nullptr),
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

    if (m_system->getProductDao()->productExists(productName)) {
        QMessageBox::warning(this, "Błąd", "Produkt o nazwie '" + productName + "' już istnieje.");
        return;
    }

    int newProductId = QRandomGenerator::global()->bounded(100000, 1000000);
    Product newProduct(newProductId, productName, minQuantity);

    if (m_system->getProductDao()->addProduct(newProduct)) {
        QMessageBox::information(this, "Sukces", "Produkt '" + productName + "' (ID: " + QString::number(newProductId) + ") został dodany.");
        productNameLineEdit->clear();
        minQuantitySpinBox->setValue(0);
        emit databaseUpdated();
        loadProductsTable();
    } else {
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

    // Sprawdzamy, czy użytkownik o danym loginie już istnieje
    if (m_system->getUserDao()->userExists(login)) {
        QMessageBox::warning(this, "Błąd", "Użytkownik o loginie '" + login + "' już istnieje.");
        return;
    }

    // Tworzymy obiekt User
    User newUser(-1, login, password, role); // ID -1, bo baza nada swoje (AUTOINCREMENT)

    // Używamy UserDao do dodania użytkownika
    if (m_system->getUserDao()->addUser(newUser)) {
        QMessageBox::information(this, "Sukces", "Użytkownik '" + login + "' (" + role + ") został dodany.");
        userLoginLineEdit->clear();
        userPasswordLineEdit->clear();
        userRoleComboBox->setCurrentIndex(0);
        emit databaseUpdated(); // Powiadom o zmianie danych
    } else {
        // Komunikat o błędzie już wyświetla UserDao
        qDebug() << "AdminView: Nie mozna dodac uzytkownika przez UserDao.";
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
    stockProductCompleterModel->setQuery("SELECT name FROM products", m_system->getDb()); // Zmieniono z m_system->db
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
    QSqlQuery query(m_system->getDb()); // Zmieniono z m_system->db
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

    int productId = m_system->getProductDao()->getProductIdByName(productName);
    if (productId == -1) {
        QMessageBox::warning(this, "Błąd", "Produkt '" + productName + "' nie znaleziono w bazie danych.");
        return;
    }

    if (m_system->getLocationDao()->updateProductLocationQuantity(productId, regal, polka, kolumna, newIlosc))
    {
        if (newIlosc > 0) {
            QMessageBox::information(this, "Sukces", "Ilość produktu w lokalizacji (R" + QString::number(regal) + "-P" + QString::number(polka) + "-K" + QString::number(kolumna) + ") została zaktualizowana na " + QString::number(newIlosc) + ".");
        } else {
            QMessageBox::information(this, "Sukces", "Lokalizacja (R" + QString::number(regal) + "-P" + QString::number(polka) + "-K" + QString::number(kolumna) + ") dla produktu '" + productName + "' została usunięta, ponieważ ilość ustawiono na 0.");
        }
        stockProductNameLineEdit->clear();
        regalSpinBox->setValue(0);
        polkaSpinBox->setValue(0);
        kolumnaSpinBox->setValue(0);
        newIloscSpinBox->setValue(0);
        currentLocationsLabel->setText("Bieżące lokalizacje dla produktu:");
        emit databaseUpdated();
    } else {
        qDebug() << "Błąd aktualizacji/dodawania lokalizacji przez LocationDao.";
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
    productsModel->setQuery("SELECT id, name, quantity FROM products", m_system->getDb()); // Zmieniono z m_system->db
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
        Product* product = m_system->getProductDao()->getProductById(productId);
        if (product) {
            editProductIdLineEdit->setText(QString::number(product->get_id()));
            editProductNameLineEdit->setText(product->get_name());
            editMinQuantitySpinBox->setValue(product->get_quantity());
            delete product;

            editProductButton->setEnabled(true);
            deleteProductButton->setEnabled(true);
        } else {
            QMessageBox::warning(this, "Błąd", "Nie można wczytać szczegółów produktu.");
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

    Product* existingProduct = m_system->getProductDao()->getProductById(productId);
    if (existingProduct && existingProduct->get_name() != newProductName) {
        if (m_system->getProductDao()->productExists(newProductName)) {
            QMessageBox::warning(this, "Błąd", "Produkt o nazwie '" + newProductName + "' już istnieje.");
            delete existingProduct;
            return;
        }
    }
    if (existingProduct) delete existingProduct;

    Product updatedProduct(productId, newProductName, newMinQuantity);

    if (m_system->getProductDao()->updateProduct(updatedProduct)) {
        QMessageBox::information(this, "Sukces", "Produkt ID " + QString::number(productId) + " został zaktualizowany.");
        loadProductsTable();
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

    if (m_system->getProductDao()->deleteProduct(productId)) {
        QMessageBox::information(this, "Sukces", "Produkt '" + productName + "' (ID: " + QString::number(productId) + ") został pomyślnie usunięty wraz z lokalizacjami.");
        loadProductsTable();
        emit databaseUpdated();
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
void AdminView::refreshData()
{
    qDebug() << "AdminView: Odświeżam dane.";
    loadProductsTable(); // Wywołuje prywatną metodę ładowania tabeli
}
