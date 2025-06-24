#ifndef ORDERDETAILSDIALOG_H
#define ORDERDETAILSDIALOG_H

#include <QDialog>
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

// Forward declaration for DispatchOrderDialog
class DispatchOrderDialog;

class OrderDetailsDialog : public QDialog
{
    Q_OBJECT

public:
    // Zmieniono konstruktor, aby przyjmował systemWarehouse*
    explicit OrderDetailsDialog(systemWarehouse* system, int orderId, QWidget *parent = nullptr);
    ~OrderDetailsDialog();

signals:
    void orderStatusChanged(int orderId); // Sygnał emitowany, gdy status zamówienia zostanie zmieniony (także po udanym wydaniu)

private slots:
    void onStatusChanged(const QString &newStatus);
    void onUpdateStatusButtonClicked();
    void onDispatchCompleted(bool success);

private:
    systemWarehouse* m_system; // NOWA ZMIENNA: wskaźnik do systemu
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
