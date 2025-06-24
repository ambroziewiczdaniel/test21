#include "mainwindow.h"
#include <QDebug>
#include <QDir>
#include <QLabel>
#include <QStyle>

MainWindow::MainWindow(systemWarehouse* system, QWidget *parent) :
    QMainWindow(parent),
    m_system(system), // Inicjalizujemy wskaźnik do systemu
    //m_userRole(userRole), // To już jest ustawiane z system->Current_User
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
    // Pobieramy rolę użytkownika z obiektu System
    if (m_system && m_system->Current_User) {
        m_userRole = m_system->Current_User->get_user_type();
    } else {
        m_userRole = "Nieznana"; // Domyślna rola, jeśli coś poszło nie tak
        QMessageBox::critical(this, "Błąd Rol", "Nie można pobrać roli użytkownika.");
        qDebug() << "Błąd: systemWarehouse lub Current_User jest nullptr w MainWindow.";
    }

    setWindowTitle("Zarządzanie Magazynem - " + m_userRole);
    setMinimumSize(1200, 800);

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    mainLayout = new QHBoxLayout(centralWidget);

    // --- Sidebar (lewa strona) ---
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


    // --- Połączenia sygnałów przycisków sidebara ze slotami ---
    connect(warehouseButton, &QPushButton::clicked, this, &MainWindow::onWarehouseClicked);
    connect(zamowieniaButton, &QPushButton::clicked, this, &MainWindow::onZamowieniaClicked);
    connect(receiptButton, &QPushButton::clicked, this, &MainWindow::onReceiptClicked);
    connect(inventoryButton, &QPushButton::clicked, this, &MainWindow::onInventoryClicked);
    connect(userRoleOptionButton, &QPushButton::clicked, this, &MainWindow::onUserRoleOptionClicked);

    // --- Centralny QStackedWidget dla zawartości ---
    stackedWidget = new QStackedWidget(this);

    // USUNIĘTO bezpośrednią inicjalizację bazy danych w MainWindow!
    // Baza danych jest teraz zarządzana przez systemWarehouse.
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
        // Możesz zdecydować, czy chcesz zamknąć aplikację w tym miejscu.
        // QApplication::quit();
        return; // Ważne, aby przerwać konstrukcję, jeśli baza danych nie działa
    }


    // --- Tworzenie i dodawanie widoków do QStackedWidget ---
    // WAŻNE: Teraz wszystkie widoki otrzymują systemWarehouse*
    warehouseView = new WarehouseView(m_system, this);
    zamowieniaView = new ZamowieniaView(m_system, this); // Będzie wymagało refaktoryzacji
    przyjecieView = new PrzyjecieView(m_system, this);   // Będzie wymagało refaktoryzacji
    inventoryView = new InventoryView(m_system, this);   // Będzie wymagało refaktoryzacji
    adminView = new AdminView(m_system, this); // Już zrefaktoryzowany
    ksiegowoscView = new KsiegowoscView(m_system, this); // Będzie wymagało refaktoryzacji
    emptyPlaceholderView = createPlaceholderView("To jest pusta zakładka. Możesz ją później wykorzystać.");

    stackedWidget->addWidget(warehouseView);        // Index 0: Magazyn
    stackedWidget->addWidget(zamowieniaView);       // Index 1: Zamówienia
    stackedWidget->addWidget(przyjecieView);        // Index 2: Przyjęcie
    stackedWidget->addWidget(inventoryView);        // Index 3: Inwentaryzacja
    stackedWidget->addWidget(adminView);            // Index 4: Administrator
    stackedWidget->addWidget(ksiegowoscView);       // Index 5: Księgowość (nowy)
    stackedWidget->addWidget(emptyPlaceholderView); // Index 6: Pusta zakładka (przesunięty index)


    // Ustaw początkowy widok na "Magazyn"
    stackedWidget->setCurrentIndex(0);

    // --- Dodaj sidebar i stacked widget do głównego układu ---
    mainLayout->addLayout(sidebarLayout);
    mainLayout->addWidget(stackedWidget);
    mainLayout->setStretch(1, 1);

    centralWidget->setLayout(mainLayout);

    // Ustaw uprawnienia po skonfigurowaniu UI
    setupUserPermissions();

    // --- Połączenie sygnałów aktualizacji bazy danych ---
    connect(adminView, &AdminView::databaseUpdated, this, &MainWindow::refreshDataViews);
    connect(przyjecieView, &PrzyjecieView::databaseUpdated, this, &MainWindow::refreshDataViews);
    connect(zamowieniaView, &ZamowieniaView::databaseUpdated, this, &MainWindow::refreshDataViews);
    connect(ksiegowoscView, &KsiegowoscView::databaseUpdated, this, &MainWindow::refreshDataViews);
}

MainWindow::~MainWindow()
{
    // Baza danych jest zarządzana przez systemWarehouse i powinna zostać zamknięta
    // przez systemWarehouse przy zakończeniu działania aplikacji.
    // Usunięto linię db.close();
    qDebug() << "MainWindow zniszczony.";
}

QWidget* MainWindow::createPlaceholderView(const QString& text) {
    Q_UNUSED(text); // Added to suppress unused parameter warning, if text is always fixed
    QWidget* placeholder = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(placeholder);
    QLabel* label = new QLabel("To jest pusta zakładka. Możesz ją później wykorzystać.", placeholder);
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
    // Tutaj musisz zdecydować, czy DocumentGeneratorDialog będzie również przyjmować systemWarehouse*
    // Jeśli tak, musisz zrefaktoryzować DocumentGeneratorDialog.
    // Na razie zostawiam z db, ale to kolejny punkt do poprawy.
    if (m_system->Database_isOpen()) { // Używamy teraz m_system->Database_isOpen()
        DocumentGeneratorDialog dialog(m_system->db, this); // Bezpośrednie użycie db z systemu
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
    // Metody refreshData muszą zostać dodane do wszystkich widoków,
    // jeśli jeszcze ich nie ma, aby widoki mogły się odświeżyć.
    // I powinny one używać m_system do pobierania nowych danych.
    warehouseView->refreshData();
    inventoryView->refreshData();
    zamowieniaView->refreshData();
    if (adminView) adminView->loadProductsTable(); // AdminView ma już loadProductsTable
    if (ksiegowoscView) ksiegowoscView->loadAllOrdersTable(); // Jeśli dodasz loadAllOrdersTable do KsiegowoscView
}