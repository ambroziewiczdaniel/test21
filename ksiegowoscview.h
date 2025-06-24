#ifndef KSIEGOWOSCVIEW_H
#define KSIEGOWOSCVIEW_H

#include <QWidget>
// #include <QSqlDatabase> // USUNIĘTO!
#include <QLineEdit>
#include <QLabel>
#include <QSpinBox>
#include <QPushButton>
#include <QTableView>
#include <QStandardItemModel>
#include <QCompleter>
#include <QSqlQueryModel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime> // For current date/time
#include <QTabWidget> // Do zarządzania zakładkami w tym widoku
#include <QHeaderView>
#include <QComboBox>
#include <QDateTimeEdit>

#include "../systemWarehouse.h" // NOWY INCLUDE

class KsiegowoscView : public QWidget
{
    Q_OBJECT

public:
    explicit KsiegowoscView(systemWarehouse* system, QWidget *parent = nullptr); // Zmieniono QSqlDatabase& na systemWarehouse*
    ~KsiegowoscView();

signals:
    void databaseUpdated(); // Sygnał do MainWindow po utworzeniu zamówienia

private slots:
    // Slot do obsługi zmian tekstu w polu wyszukiwania produktu dla nowego zamówienia
    void onProductSearchTextChanged(const QString &text);
    // Slot wywoływany po wybraniu produktu z completera dla nowego zamówienia
    void onProductSelected(const QString &text);
    // Slot do dodawania produktu do tymczasowej listy nowego zamówienia
    void onAddProductToOrderClicked();
    // Slot do tworzenia nowego zamówienia w bazie danych
    void onCreateOrderClicked();

    // NOWE sloty dla zakładki edycji zamówień
    void onOrderSelectionChanged(const QModelIndex &current, const QModelIndex &previous);
    void onRefreshOrdersButtonClicked();
    void onSaveOrderChangesButtonClicked();
    void onDeleteOrderButtonClicked();
    void onAddProductToExistingOrderClicked();
    void onDeleteOrderItemFromExistingOrderClicked();
    void onUpdateOrderItemQuantityClicked();


private:
    systemWarehouse* m_system; // NOWA ZMIENNA: wskaźnik do systemu
    // QSqlDatabase &m_db; // USUNIĘTO!

    QTabWidget *tabWidget; // Do zarządzania zakładkami "Nowe Zamówienie" i "Edytuj Zamówienia"

    // --- Dane zamówienia ogólne (dla zakładki "Nowe Zamówienie") ---
    QLabel *kontrahentLabel;
    QLineEdit *kontrahentLineEdit;
    QLabel *orderDateLabel;

    // --- Wyszukiwanie i dodawanie produktów do zamówienia (dla zakładki "Nowe Zamówienie") ---
    QLabel *productSearchLabel;
    QLineEdit *productSearchLineEdit;
    QCompleter *productCompleter;
    QSqlQueryModel *productCompleterModel;
    QLabel *currentProductIdLabel;
    QLabel *currentProductNameLabel;
    QSpinBox *productQuantitySpinBox;
    QPushButton *addProductToOrderButton;

    // --- Tabela pozycji zamówienia (tymczasowa dla zakładki "Nowe Zamówienie") ---
    QLabel *orderItemsLabel;
    QTableView *orderItemsTableView;
    QStandardItemModel *orderItemsModel; // Model dla tymczasowych pozycji zamówienia

    // --- Przycisk utworzenia zamówienia (dla zakładki "Nowe Zamówienie") ---
    QPushButton *createOrderButton;

    // --- NOWE: Zmienne dla zakładki "Edytuj Zamówienia" ---
    QTableView *allOrdersTableView;
    QSqlQueryModel *allOrdersModel;

    QLineEdit *editOrderIdLineEdit;
    QLineEdit *editKontrahentLineEdit;
    QDateTimeEdit *editOrderDateTimeEdit;
    QComboBox *editOrderStatusComboBox;

    QTableView *editedOrderItemsTableView;
    QStandardItemModel *editedOrderItemsModel;

    QLineEdit *editOrderItemProductSearchLineEdit;
    QCompleter *editOrderItemProductCompleter;
    QSqlQueryModel *editOrderItemProductCompleterModel;
    QLabel *editOrderItemProductIdLabel;
    QLabel *editOrderItemProductNameLabel;
    QSpinBox *editOrderItemQuantitySpinBox;
    QPushButton *addProductToExistingOrderButton;

    QPushButton *deleteOrderItemFromExistingOrderButton;
    QPushButton *updateOrderItemQuantityButton;

    QPushButton *saveOrderChangesButton;
    QPushButton *deleteOrderButton;
    QPushButton *refreshOrdersButton;


    // Metody pomocnicze
    void setupCreateOrderTab(); // Nowa funkcja dla istniejącej zakładki
    void setupEditOrdersTab(); // Nowa funkcja do konfiguracji zakładki edycji
    void loadAllOrdersTable(); // Ładuje wszystkie zamówienia do tabeli
    void loadOrderDetails(int orderId