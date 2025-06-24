#ifndef PRODUCTDAO_H
#define PRODUCTDAO_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVector>
#include <QMessageBox>
#include <QDebug>

#include "Product.h"

// Klasa ProductDao będzie odpowiedzialna za operacje CRUD na tabeli 'products'
class ProductDao
{
private:
    QSqlDatabase& m_db; // Referencja do instancji bazy danych z systemWarehouse

public:
    explicit ProductDao(QSqlDatabase& db);

    // Metody do operacji CRUD
    bool addProduct(const Product& product);
    QVector<Product*> getAllProducts();
    QVector<Product*> getFilteredProducts(const QString& filterText);
    Product* getProductById(int id);
    bool updateProduct(const Product& product);
    bool deleteProduct(int id);

    // Metody pomocnicze do walidacji lub specyficznych zapytań związanych z produktami
    bool productExists(const QString& name);
    bool productExists(int id);
    int getProductIdByName(const QString& name);
};

#endif // PRODUCTDAO_H