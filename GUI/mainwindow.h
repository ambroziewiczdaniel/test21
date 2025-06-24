#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QDebug>

#include "../systemWarehouse.h"

#include "warehouseview.h"
#include "inventoryview.h"
#include "documentgeneratordialog.h"
#include "zamowieniaview.h"
#include "loginwindow.h"
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
    void onGenerateDocumentButtonClicked();

    void handleAdministratorAction();
    void handleKsiegowoscAction();

private:
    systemWarehouse* m_system; // <--- DODAJ TĘ LINIĘ!
    QString m_userRole;

    QHBoxLayout *mainLayout;
    QVBoxLayout *sidebarLayout;
    QStackedWidget *stackedWidget;

    QPushButton *warehouseButton;
    QPushButton *zamowieniaButton;
    QPushButton *receiptButton;
    QPushButton *inventoryButton;
    QPushButton *userRoleOptionButton;

    WarehouseView *warehouseView;
    ZamowieniaView *zamowieniaView;
    PrzyjecieView *przyjecieView;
    InventoryView *inventoryView;
    AdminView *adminView;
    KsiegowoscView *ksiegowoscView;
    QWidget *emptyPlaceholderView;

    QWidget* createPlaceholderView(const QString& text);
    void setupUserPermissions();
};

#endif // MAINWINDOW_H <--- UPEWNIJ SIĘ, ŻE TO JEST NA KOŃCU PLIKU!
