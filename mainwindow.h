#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QDebug> // Dodane dla qDebug

#include "../systemWarehouse.h"

// Dołączanie widoków
#include "warehouseview.h"
#include "inventoryview.h"
#include "documentgeneratordialog.h" // Jeśli DocumentGeneratorDialog też wymaga refaktoryzacji, to osobny krok
#include "zamowieniaview.h"
#include "loginwindow.h" // Nie bezpośrednio potrzebne tu, ale zostawmy
#include "adminview.h"
#include "przyjecieview.h"
#include "ksiegowoscview.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(systemWarehouse* system, QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void refreshDataViews();

private slots:
    void onWarehouseClicked();
    void onZamowieniaClicked();
    void onReceiptClicked();
    void onInventoryClicked();
    void onUserRoleOptionClicked();
    void onGenerateDocumentButtonClicked(); // Ta metoda może wymagać refaktoryzacji na systemWarehouse*

    void handleAdministratorAction();
    void handleKsiegowoscAction();

private:
    systemWarehouse* m_system; // NOWA ZMIENNA: Wskaźnik do systemu
    // QSqlDatabase db; // USUNIĘTO: Baza danych jest zarządzana przez systemWarehouse
    QString m_userRole;

    QHBoxLayout *mainLayout;
    QVBoxLayout *sidebarLayout;
    QStackedWidget *stackedWidget;

    QPushButton *warehouseButton;
    QPushButton *zamowieniaButton;
    QPushButton *receiptButton;
    QPushButton *inventoryButton;
    QPushButton *userRoleOptionButton;

    // Zmieniono typ konstruktora dla widoków, które już zrefaktoryzowaliśmy
    WarehouseView *warehouseView;
    ZamowieniaView *zamowieniaView;
    PrzyjecieView *przyjecieView;
    InventoryView *inventoryView;
    AdminView *adminView; // Jest już zrefaktoryzowany do systemWarehouse*
    KsiegowoscView *ksiegowoscView; // Będzie zrefaktoryzowany do systemWarehouse*
    QWidget *emptyPlaceholderView;

    QWidget* createPlaceholderView(const QString& text);
    void setupUserPermissions();
};

#endif // MAINWINDOW_H