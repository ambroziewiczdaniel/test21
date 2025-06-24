#include "mainwindow.h"
#include <QDebug>
#include <QDir>
#include <QLabel>
#include <QStyle>

MainWindow::MainWindow(systemWarehouse* system, QWidget *parent) :
    QMainWindow(parent),
    m_system(system), //
    warehouseButton(nullptr),
    zamowieniaButton(nullptr),
    receiptButton(nullptr),
    inventoryButton(nullptr),
    userRoleOptionButton(nullptr),
    warehouseView(nullptr),
    zamowieniaView(nullptr),
    przyjecieView(nullptr),
    inventoryView(nullptr),
    adminView(nullptr),
    ksiegowoscView(nullptr),
    emptyPlaceholderView(nullptr)
{
    if (m_system && m_system->Current_User) {
        m_userRole = m_system->Current_User->get_user_type();
    } else {
        m_userRole = "Nieznana";
        QMessageBox::critical(this, "Błąd Rol", "Nie można pobrać roli użytkownika.");
        qDebug() << "Błąd: systemWarehouse lub Current_User jest nullptr w MainWindow.";
    }

    setWindowTitle("Zarządzanie Magazynem - " + m_userRole);
    setMinimumSize(1200, 800);

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    mainLayout = new QHBoxLayout(centralWidget);

    sidebarLayout = new QVBoxLayout();
    sidebarLayout->setSpacing(10);
    sidebarLayout->setContentsMargins(10, 10, 10, 10);

    warehouseButton = new QPushButton("Magazyn", this);
    warehouseButton->setMinimumHeight(50);

    zamowieniaButton = new QPushButton("Zamówienia", this);
    zamowieniaButton->setMinimumHeight(50);

    receiptButton = new QPushButton("Przyjęcie", this);
    receiptButton->setMinimumHeight(50);

    inventoryButton = new QPushButton("Inwentaryzacja", this);
    inventoryButton->setMinimumHeight(50);

    userRoleOptionButton = new QPushButton("Inna Opcja", this);
    userRoleOptionButton->setMinimumHeight(50);


    sidebarLayout->addWidget(warehouseButton);
    sidebarLayout->addWidget(zamowieniaButton);
    sidebarLayout->addWidget(receiptButton);
    sidebarLayout->addWidget(inventoryButton);
    sidebarLayout->addStretch();
    sidebarLayout->addWidget(userRoleOptionButton);


    connect(warehouseButton, &QPushButton::clicked, this, &MainWindow::onWarehouseClicked);
    connect(zamowieniaButton, &QPushButton::clicked, this, &MainWindow::onZamowieniaClicked);
    connect(receiptButton, &QPushButton::clicked, this, &MainWindow::onReceiptClicked);
    connect(inventoryButton, &QPushButton::clicked, this, &MainWindow::onInventoryClicked);
    connect(userRoleOptionButton, &QPushButton::clicked, this, &MainWindow::onUserRoleOptionClicked);

    stackedWidget = new QStackedWidget(this);

    if (!m_system->Database_isOpen()) {
        QMessageBox::critical(this, "Błąd Bazy Danych",
                              "Baza danych nie jest otwarta w systemWarehouse. "
                              "Upewnij się, że systemWarehouse poprawnie otwiera bazę danych.");
        qDebug() << "Błąd: Baza danych systemWarehouse nie jest otwarta.";
        warehouseButton->setEnabled(false);
        zamowieniaButton->setEnabled(false);
        receiptButton->setEnabled(false);
        inventoryButton->setEnabled(false);
        userRoleOptionButton->setEnabled(false);
        return;
    }


    warehouseView = new WarehouseView(m_system, this);
    zamowieniaView = new ZamowieniaView(m_system, this);
    przyjecieView = new PrzyjecieView(m_system, this);
    inventoryView = new InventoryView(m_system, this);
    adminView = new AdminView(m_system, this);
    ksiegowoscView = new KsiegowoscView(m_system, this);
    emptyPlaceholderView = createPlaceholderView("DEBUG: pusta zakładka.");

    stackedWidget->addWidget(warehouseView);
    stackedWidget->addWidget(zamowieniaView);
    stackedWidget->addWidget(przyjecieView);
    stackedWidget->addWidget(inventoryView);
    stackedWidget->addWidget(adminView);
    stackedWidget->addWidget(ksiegowoscView);
    stackedWidget->addWidget(emptyPlaceholderView);


    stackedWidget->setCurrentIndex(0);

    mainLayout->addLayout(sidebarLayout);
    mainLayout->addWidget(stackedWidget);
    mainLayout->setStretch(1, 1);

    centralWidget->setLayout(mainLayout);

    setupUserPermissions();

    connect(adminView, &AdminView::databaseUpdated, this, &MainWindow::refreshDataViews);
    connect(przyjecieView, &PrzyjecieView::databaseUpdated, this, &MainWindow::refreshDataViews);
    connect(zamowieniaView, &ZamowieniaView::databaseUpdated, this, &MainWindow::refreshDataViews);
    connect(ksiegowoscView, &KsiegowoscView::databaseUpdated, this, &MainWindow::refreshDataViews);
}

MainWindow::~MainWindow()
{
    qDebug() << "MainWindow zniszczony.";
}

QWidget* MainWindow::createPlaceholderView(const QString& text) {
    Q_UNUSED(text);
    QWidget* placeholder = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(placeholder);
    QLabel* label = new QLabel("DEBUG: pusta zakladka", placeholder);
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);
    placeholder->setLayout(layout);
    return placeholder;
}

void MainWindow::onWarehouseClicked()
{
    stackedWidget->setCurrentIndex(0);
}

void MainWindow::onZamowieniaClicked()
{
    stackedWidget->setCurrentIndex(1);
}

void MainWindow::onReceiptClicked()
{
    stackedWidget->setCurrentIndex(2);
}

void MainWindow::onInventoryClicked()
{
    stackedWidget->setCurrentIndex(3);
}

void MainWindow::onUserRoleOptionClicked()
{
    if (m_userRole == "Administrator") {
        handleAdministratorAction();
    } else if (m_userRole == "Ksiegowosc") {
        handleKsiegowoscAction();
    }
}

void MainWindow::onGenerateDocumentButtonClicked()
{
    if (m_system->Database_isOpen()) {
        DocumentGeneratorDialog dialog(m_system, this);
        dialog.exec();
    } else {
        QMessageBox::warning(this, "Błąd", "Baza danych nie jest otwarta. Nie można wygenerować dokumentu.");
    }
}

void MainWindow::setupUserPermissions()
{
    warehouseButton->setVisible(true);
    zamowieniaButton->setVisible(true);
    receiptButton->setVisible(true);
    inventoryButton->setVisible(true);
    userRoleOptionButton->setVisible(true);
    if (m_userRole == "Magazynier") {
        userRoleOptionButton->setVisible(false);
    } else if (m_userRole == "Ksiegowosc") {
        userRoleOptionButton->setText("Księgowość");
        receiptButton->setVisible(false);
        inventoryButton->setVisible(false);
    } else if (m_userRole == "Administrator") {
        userRoleOptionButton->setText("Administrator");
    } else {
        warehouseButton->setVisible(false);
        zamowieniaButton->setVisible(false);
        receiptButton->setVisible(false);
        inventoryButton->setVisible(false);
        userRoleOptionButton->setVisible(false);

        QMessageBox::warning(this, "Błąd Rol", "Nieznana rola użytkownika: " + m_userRole);
    }
}

void MainWindow::handleAdministratorAction() {
    qDebug() << "Administrator akcja - przełączam na widok AdminView";
    stackedWidget->setCurrentIndex(4);
}

void MainWindow::handleKsiegowoscAction() {
    qDebug() << "Księgowość akcja - przełączam na widok KsiegowoscView";
    stackedWidget->setCurrentIndex(5);
}

void MainWindow::refreshDataViews()
{
    qDebug() << "Odświeżam wszystkie widoki danych...";
    warehouseView->refreshData();
    inventoryView->refreshData();
    zamowieniaView->refreshData();
    if (adminView) adminView->refreshData(); 
    if (ksiegowoscView) ksiegowoscView->refreshData(); 
}
