#ifndef ORDERDETAILSDIALOG_H
#define ORDERDETAILSDIALOG_H

#include <QDialog>
// #include <QSqlDatabase> // USUNIĘTO!
#include <QSqlQueryModel>
#include <QTableView>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSqlError>
#include <QSqlQuery>
#include <QMessageBox>

#include "../systemWarehouse.h" // NOWY INCLUDE

// Forward declaration for the new dialog (będzie również używać systemWarehouse*)
class DispatchOrderDialog;

class OrderDetailsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OrderDetailsDialog(systemWarehouse* system, int orderId, QWidget *parent = nullptr); // Zmieniono QSqlDatabase& na systemWarehouse*
    ~OrderDetailsDialog();

signals:
    void orderStatusChanged(int orderId);

private slots:
    void onStatusChanged(const QString &newStatus);
    void onUpdateStatusButtonClicked();
    void onDispatchCompleted(bool success);

private:
    systemWarehouse* m_system; // NOWA ZMIENNA: wskaźnik do systemu
    // QSqlDatabase &m_db; // USUNIĘTO!
    int m_orderId;

    QLabel *kontrahentLabel;
    QLabel *dataLabel;
    QLabel *statusLabel;
    QComboBox *statusComboBox;
    QPushButton *updateStatusButton;

    QSqlQueryModel *productsModel; // Nadal QSqlQueryModel, ale będzie używać query z DAO/System
    QTableView *productsTableView;

    void loadOrderDetails();
    void loadOrderProducts();
    void updateOrderStatusInDb(const QString& newStatus);
};

#endif // ORDERDETAILSDIALOG_H