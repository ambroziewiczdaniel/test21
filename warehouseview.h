#ifndef WAREHOUSEVIEW_H
#define WAREHOUSEVIEW_H

#include <QWidget>
#include <QSqlDatabase> // Może być usunięte, jeśli nie jest bezpośrednio używane
#include <QSqlQueryModel>
#include <QTableView>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QSqlError>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QSqlQuery>
#include <QVariant>

#include "systemWarehouse.h"

// --- NOWY DELEGAT DLA TABELI MAGAZYNU --- (BEZ ZMIAN)
class WarehouseItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit WarehouseItemDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        painter->save();

        const QAbstractItemModel *model = index.model();
        if (!model) {
            QStyledItemDelegate::paint(painter, option, index);
            painter->restore();
            return;
        }

        if (index.column() >= 0 && model->columnCount() > 3) {
            int minimalQuantity = model->data(model->index(index.row(), 2), Qt::DisplayRole).toInt();
            int totalActualQuantity = model->data(model->index(index.row(), 3), Qt::DisplayRole).toInt();

            if (totalActualQuantity < minimalQuantity) {
                QColor backgroundColor = QColor(255, 200, 200);
                painter->fillRect(option.rect, backgroundColor);
            }
        }

        painter->restore();
        QStyledItemDelegate::paint(painter, option, index);
    }
};
// --- KONIEC DELEGATA ---


class WarehouseView : public QWidget
{
    Q_OBJECT

public:
    explicit WarehouseView(systemWarehouse* System, QWidget *parent = nullptr);
    ~WarehouseView();

public slots:
    void refreshData();

private slots:
    void onSearchTextChanged(const QString &text);

private:
    QSqlQueryModel* model; // Zmieniono na QSqlQueryModel*
    QTableView *tableView;
    QLabel *searchLabel;
    QLineEdit *searchLineEdit;

    systemWarehouse* System;

    void setProductQuery(const QString &filterText = "");
};

#endif // WAREHOUSEVIEW_H
