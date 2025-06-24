#ifndef INVENTORYVIEW_H
#define INVENTORYVIEW_H

#include <QWidget>
#include <QSqlQueryModel>
#include <QTableView>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QSqlError>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QApplication>
#include <QStyleOptionButton>
#include <QMouseEvent>
#include <QHash>
#include <QStandardItemModel>
#include <QSpinBox>
#include <QSqlQuery>
#include <QVariant>

#include "inventorydocumentgenerator.h"
#include "../systemWarehouse.h"

// --- Forward declarations dla delegatów ---
class InventoryView;
class InventoryCheckboxDelegate;
class EditableQuantityDelegate;

// --- DELEGAT DLA CHECKBOXÓW W GŁÓWNEJ TABELI INWENTARYZACJI ---
class InventoryCheckboxDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit InventoryCheckboxDelegate(QObject *parent, QHash<int, bool> *checkboxStates, InventoryView *view);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;

private:
    QHash<int, bool> *m_checkboxStates;
    InventoryView *m_inventoryView;
};

// --- DELEGAT DLA EDYTOWALNEJ KOLUMNY "Prawdziwa ilość w magazynie" ---
class EditableQuantityDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit EditableQuantityDelegate(QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};


class InventoryView : public QWidget
{
    Q_OBJECT

public:
    explicit InventoryView(systemWarehouse* system, QWidget *parent = nullptr);
    ~InventoryView();

public slots:
    void refreshData(); // Publiczny slot do odświeżania

public:
    void updateSelectedProductsTable(int row, bool checked);

private slots:
    void onSearchTextChanged(const QString &text);
    void onGenerateInventoryDocumentClicked();

private:
    systemWarehouse* m_system;

    QSqlQueryModel *mainTableModel;
    QTableView *mainTableView;
    QLabel *searchLabel;
    QLineEdit *searchLineEdit;

    QHash<int, bool> m_checkboxStates;
    QHash<int, int> m_realQuantities;

    QLabel *selectedProductsLabel;
    QTableView *selectedProductsTableView;
    QStandardItemModel *selectedProductsModel;
    QPushButton *generateInventoryDocumentButton;

    void setProductQuery(const QString &filterText = "");
};

#endif // INVENTORYVIEW_H
