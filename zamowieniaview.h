#ifndef ZAMOWIENIAVIEW_H
#define ZAMOWIENIAVIEW_H

#include <QWidget>
// #include <QSqlDatabase> // USUNIĘTO!
#include <QSqlQueryModel>
#include <QTableView>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QSqlError>
#include <QItemSelectionModel>

#include "orderdetailsdialog.h" // Do otwierania okna szczegółów zamówienia
#include <QSqlQuery>
#include <QVariant>

#include "../systemWarehouse.h" // NOWY INCLUDE

class ZamowieniaView : public QWidget
{
    Q_OBJECT

public:
    explicit ZamowieniaView(systemWarehouse* system, QWidget *parent = nullptr); // Zmieniono QSqlDatabase& na systemWarehouse*
    ~ZamowieniaView();

public slots:
    void refreshData();

signals:
    void databaseUpdated(); // Używany, aby MainWindow mogło odświeżyć inne widoki

private slots:
    void onSearchTextChanged(const QString &text);
    void onRowClicked(const QModelIndex &index);
    void onOrderStatusChanged(int orderId); // Nadal używany, ale będzie emitował databaseUpdated

private:
    systemWarehouse* m_system; // NOWA ZMIENNA: wskaźnik do systemu
    // QSqlDatabase &m_db; // USUNIĘTO!
    QSqlQueryModel *model;
    QTableView *tableView;
    QLabel *searchLabel;
    QLineEdit *searchLineEdit;

    void setOrderQuery(const QString &filterText = "");
};

#endif // ZAMOWIENIAVIEW_H