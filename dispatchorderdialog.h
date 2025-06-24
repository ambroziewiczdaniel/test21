#ifndef DISPATCHORDERDIALOG_H
#define DISPATCHORDERDIALOG_H

#include <QDialog>
#include <QSqlDatabase> // Będzie zmienione na systemWarehouse*
#include <QSqlQueryModel>
#include <QTableView>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery> // Potrzebne do zapytan
#include <QVariant>
#include <QStandardItemModel> // Do tabeli edytowalnej

#include "../systemWarehouse.h" // NOWY INCLUDE

class DispatchOrderDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DispatchOrderDialog(systemWarehouse* system, int orderId, QWidget *parent = nullptr); // Zmieniono QSqlDatabase& na systemWarehouse*
    ~DispatchOrderDialog();

signals:
    void dispatchCompleted(bool success); // Sygnał po zakończeniu wydania (true dla sukcesu)
    void dispatchCancelled(); // Sygnał, gdy dialog zostanie zamknięty bez wydania

private slots:
    void onConfirmDispatchClicked();
    void onCancelClicked();
    void onQuantityChanged(const QModelIndex &index); // Slot do reagowania na zmiany ilości w tabeli

private:
    systemWarehouse* m_system; // NOWA ZMIENNA: wskaźnik do systemu
    // QSqlDatabase &m_db; // USUNIĘTO!
    int m_orderId;

    QLabel *orderInfoLabel; // Wyświetla ID zamówienia
    QStandardItemModel *dispatchProductsModel; // Będzie zawierać produkty do wydania z możliwością edycji ilości
    QTableView *dispatchProductsTableView;
    QPushButton *confirmDispatchButton;
    QPushButton *cancelButton;

    void loadProductsForDispatch();
};

#endif // DISPATCHORDERDIALOG_H