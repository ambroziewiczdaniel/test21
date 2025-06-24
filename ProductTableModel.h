#pragma once

#include <QTableView>
#include <QAbstractTableModel>
#include <vector>
#include <QString>

#include "Product.h"

class ProductTableModel : public QAbstractTableModel {
	Q_OBJECT

public:
    explicit ProductTableModel(const std::vector<Product>& products, QObject* parent = nullptr)
        : QAbstractTableModel(parent), products(products) {
    }

    int rowCount(const QModelIndex& parent = QModelIndex()) const override {
        Q_UNUSED(parent);
        return static_cast<int>(products.size());
    }

    int columnCount(const QModelIndex& parent = QModelIndex()) const override {
        Q_UNUSED(parent);
        return 3; // Number of attributes in Product: name, price, quantity
    }

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override {
        if (!index.isValid() || role != Qt::DisplayRole)
            return QVariant();

        const Product& product = products[index.row()];
        switch (index.column()) {
        case 0: return product.get_name();
        case 1: return product.get_id();
        case 2: return product.get_quantity();
        default: return QVariant();
        }
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override {
        if (role != Qt::DisplayRole)
            return QVariant();

        if (orientation == Qt::Horizontal) {
            switch (section) {
            case 0: return "Name";
            case 1: return "Id";
            case 2: return "Qantity";
            default: return QVariant();
            }
        }
        return QVariant();
    }

private:
    std::vector<Product> products;
};