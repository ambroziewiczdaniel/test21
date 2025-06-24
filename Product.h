#pragma once

#include <QString>

class Product
{
    int id;
    QString name;
    int quantity;

public:
    Product(int id, const QString& name, int quantity);
    ~Product();

    int get_id() const { return id; }
    QString get_name() const { return name; };
    int get_quantity() const { return quantity;  }

};
