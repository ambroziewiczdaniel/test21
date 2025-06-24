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
#include <QSqlQuery>
#include <QSqlError>
#include <QRandomGenerator>
#include <QCompleter>
#include <QSqlQueryModel>
#include <QTableView>
#include <QHeaderView>

#include "../systemWarehouse.h"

class AdminView : public QWidget
{
    Q_OBJECT

public:
    explicit AdminView(systemWarehouse *system, QWidget *parent = nullptr);
    ~AdminView();

public slots: // <--- DODANO public slots:
    void refreshData(); // <--- NOWY PUBLICZNY SLOT
signals:
    void databaseUpdated();

private slots:
    void onAddProductButtonClicked();
    void onAddUserButtonClicked();
    void onProductForStockSelected(const QString &text);
    void onUpdateLocationStockButtonClicked();

    void onEditProductButtonClicked();
    void onDeleteProductButtonClicked();
    void onProductSelectionChanged(const QModelIndex &current, const QModelIndex &previous);
    void onRefreshProductsButtonClicked();

private:
    systemWarehouse *m_system;
    QTabWidget *tabWidget;

    QLineEdit *productNameLineEdit;
    QSpinBox *minQuantitySpinBox;
    QPushButton *addProductButton;

    QLineEdit *userLoginLineEdit;
    QLineEdit *userPasswordLineEdit;
    QComboBox *userRoleComboBox;
    QPushButton *addUserButton;

    QLineEdit *stockProductNameLineEdit;
    QCompleter *stockProductCompleter;
    QSqlQueryModel *stockProductCompleterModel;
    QLabel *currentLocationsLabel;
    QSpinBox *regalSpinBox;
    QSpinBox *polkaSpinBox;
    QSpinBox *kolumnaSpinBox;
    QSpinBox *newIloscSpinBox;
    QPushButton *updateLocationStockButton;

    QTableView *productsTableView;
    QSqlQueryModel *productsModel;
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
    void loadProductsTable();
};
#endif // ADMINVIEW_H <--- UPEWNIJ SIĘ, ŻE TO JEST NA KOŃCU PLIKU!
