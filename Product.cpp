#include "Product.h"

Product::Product(int id, const QString& name, int quantity)
    : id(id),
    name(name),
    quantity(quantity)
{
}

Product::~Product()
{
}