// GUI/dispatchorderdialog.h
#ifndef DISPATCHORDERDIALOG_H
#define DISPATCHORDERDIALOG_H

#include <QDialog>
#include <QSqlQueryModel>
#include <QTableView>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>
#include <QStandardItemModel>
#include <QSpinBox> // Still needed if you use QSpinBox directly in this class

#include "../systemWarehouse.h"
#include "dispatchquantitydelegate.h" // NOWY INCLUDE dla delegata!

// Removed: class DispatchQuantityDelegate; // No longer needed here

class DispatchOrderDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DispatchOrderDialog(systemWarehouse* system, int orderId, QWidget *parent = nullptr);
    ~DispatchOrderDialog();

signals:
    void dispatchCompleted(bool success);
    void dispatchCancelled();

private slots:
    void onConfirmDispatchClicked();
    void onCancelClicked();
    void onQuantityChanged(const QModelIndex &index);

private:
    systemWarehouse* m_system;
    int m_orderId;

    QLabel *orderInfoLabel;
    QStandardItemModel *dispatchProductsModel;
    QTableView *dispatchProductsTableView;
    QPushButton *confirmDispatchButton;
    QPushButton *cancelButton;

    void loadProductsForDispatch();
};

#endif // DISPATCHORDERDIALOG_H
