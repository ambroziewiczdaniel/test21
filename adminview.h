#ifndef ADMINVIEW_H
#define ADMINVIEW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QComboBox>
#include <QTabWidget>
#include <QMessageBox>
#include <QSqlQuery> // Nadal potrzebne dla tymczasowych zapytań (locations)
#include <QSqlError>
#include <QRandomGenerator>
#include <QCompleter>
#include <QSqlQueryModel>
#include <QTableView>
#include <QHeaderView>

#include "systemWarehouse.h" // NOWY INCLUDE

class AdminView : public QWidget
{
    Q_OBJECT

public:
    explicit AdminView(systemWarehouse *system, QWidget *parent = nullptr); // Zmieniono QSqlDatabase& na systemWarehouse*
    ~AdminView();

signals:
    void databaseUpdated();

private slots:
    void onAddProductButtonClicked();
    void onAddUserButtonClicked();
    void onProductForStockSelected(const QString &text);
    void onUpdateLocationStockButtonClicked();

    // Nowe sloty dla zakładki edycji produktów
    void onEditProductButtonClicked();
    void onDeleteProductButtonClicked();
    void onProductSelectionChanged(const QModelIndex &current, const QModelIndex &previous);
    void onRefreshProductsButtonClicked();

private:
    systemWarehouse *m_system; // NOWA ZMIENNA: wskaźnik do systemu
    // QSqlDatabase &m_db; // USUNIĘTO! Widok nie ma bezpośredniego dostępu do bazy danych
    QTabWidget *tabWidget;

    // Istniejące zmienne dla zakładki "Dodaj Produkt"
    QLineEdit *productNameLineEdit;
    QSpinBox *minQuantitySpinBox;
    QPushButton *addProductButton;

    // Istniejące zmienne dla zakładki "Dodaj Użytkownika"
    QLineEdit *userLoginLineEdit;
    QLineEdit *userPasswordLineEdit;
    QComboBox *userRoleComboBox;
    QPushButton *addUserButton;

    // Istniejące zmienne dla zakładki "Zmień Stan Magazynowy"
    QLineEdit *stockProductNameLineEdit;
    QCompleter *stockProductCompleter;
    QSqlQueryModel *stockProductCompleterModel; // Będzie używał ProductDao do pobierania danych
    QLabel *currentLocationsLabel;
    QSpinBox *regalSpinBox;
    QSpinBox *polkaSpinBox;
    QSpinBox *kolumnaSpinBox;
    QSpinBox *newIloscSpinBox;
    QPushButton *updateLocationStockButton;

    // Nowe zmienne dla zakładki "Edytuj Produkty"
    QTableView *productsTableView;
    QSqlQueryModel *productsModel; // Będzie używał ProductDao do pobierania danych
    QLineEdit *editProductIdLineEdit;
    QLineEdit *editProductNameLineEdit;
    QSpinBox *editMinQuantitySpinBox;
    QPushButton *editProductButton;
    QPushButton *deleteProductButton;
    QPushButton *refreshProductsButton;

    void setupAddProductTab();
    void setupAddUserTab();
    void setupChangeStockTab();
    void setupEditProductTab();
    void loadProductLocations(int productId);

    // loadProductsTable będzie teraz odświeżać model QSqlQueryModel,
    // który będzie ustawiał query poprzez ProductDao
    void loadProductsTable();
};

#endif // ADMINVIEW_H