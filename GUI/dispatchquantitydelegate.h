// GUI/dispatchquantitydelegate.h
#ifndef DISPATCHQUANTITYDELEGATE_H
#define DISPATCHQUANTITYDELEGATE_H

#include <QStyledItemDelegate>
#include <QSpinBox> // Needed for createEditor
#include <QObject>  // Base class for Q_OBJECT

class DispatchQuantityDelegate : public QStyledItemDelegate
{
    Q_OBJECT // This MUST be here

public:
    explicit DispatchQuantityDelegate(QObject *parent = nullptr);

    // Overridden methods from QStyledItemDelegate
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // DISPATCHQUANTITYDELEGATE_H
