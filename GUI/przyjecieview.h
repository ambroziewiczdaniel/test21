#ifndef PRZYJECIEVIEW_H
#define PRZYJECIEVIEW_H

#include <QWidget>
// #include <QSqlDatabase> // USUNIĘTO!
#include <QSqlQueryModel>
#include <QTableView>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QSqlError>
#include <QCompleter>
#include <QSpinBox>
#include <QPushButton>
#include <QStandardItemModel>

#include "pzdocumentgenerator.h"
#include "../systemWarehouse.h" // NOWY INCLUDE

class PrzyjecieView : public QWidget
{
    Q_OBJECT

public:
    explicit PrzyjecieView(systemWarehouse* system, QWidget *parent = nullptr); // Zmieniono QSqlDatabase& na systemWarehouse*
    ~PrzyjecieView();

signals:
    void databaseUpdated();

private slots:
    void onSearchTextChanged(const QString &text);
    void onProductSelected(const QString &text);
    void onAddProductToReceiptClicked();
    void onGeneratePZClicked();

private:
    systemWarehouse* m_system; // NOWA ZMIENNA: wskaźnik do systemu
    // QSqlDatabase &m_db; // USUNIĘTO!

    QLabel *searchProductLabel;
    QLineEdit *productSearchLineEdit;
    QCompleter *productCompleter;
    QSqlQueryModel *productCompleterModel;

    QLabel *currentProductIdLabel;
    QLabel *currentProductQuantityLabel; // Będzie zawierać info o minimalnej ilości i lokalizacjach

    QLabel *regalLabel;
    QSpinBox *regalSpinBox;
    QLabel *polkaLabel;
    QSpinBox *polkaSpinBox;
    QLabel *kolumnaLabel;
    QSpinBox *kolumnaSpinBox;
    QLabel *iloscPrzyjetaLabel;
    QSpinBox *iloscPrzyjetaSpinBox;

    QPushButton *addProductToReceiptButton;

    QLabel *receiptItemsLabel;
    QTableView *receiptItemsTableView;
    QStandardItemModel *receiptItemsModel;

    QPushButton *generatePZButton;

    void loadProductDetailsAndLocations(int productId);
};

#endif // PRZYJECIEVIEW_H
