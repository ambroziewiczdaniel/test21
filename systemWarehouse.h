#pragma once

#include <qapplication.h>
#include <qmessagebox.h>
#include <qdir.h>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <qsqltablemodel.h>

#include <QVector>
#include <vector>
#include "IAuthorizeUser.h"
#include "Product.h"
#include "UserClass.h"
#include "ProductDao.h"
#include "LocationDao.h"
#include "Order.h"
#include "OrderItem.h"
#include "OrderDao.h"
#include "OrderItemDao.h"

// Musisz mieć te pliki, nawet jeśli są puste
// #include "userWarehouseKeeper.h"
// #include "userAccountant.h"
// #include "userAdmin.h"

class systemWarehouse : public IAuthorizeUser
{
public:
    QVector<Product*> products;
    UserClass* Current_User;

public:
    systemWarehouse();
    ~systemWarehouse();

    QString authorize_logIn(std::string username, std::string password);
    void authorize_logOut() { delete(Current_User); Current_User = nullptr; };

    void loadProducts();

    QSqlQuery getWarehouseProductQuery(const QString& filterText = "");
    QSqlQuery getOrdersQuery(const QString& filterText = "");

    void closeDatabase() { if(db.isOpen()) db.close(); };
    bool openDatabase() ;
    bool Database_isOpen() { return db.isOpen(); }

    // Dostęp do DAO
    ProductDao* getProductDao() const { return m_productDao; }
    LocationDao* getLocationDao() const { return m_locationDao; }
    OrderDao* getOrderDao() const { return m_orderDao; }
    OrderItemDao* getOrderItemDao() const { return m_orderItemDao; }

    // NOWA METODA: Dodanie publicznego gettera dla QSqlDatabase (dla QSqlQueryModel itp.)
    QSqlDatabase& getDb() { return db; }

private:
    QSqlDatabase db;
    ProductDao* m_productDao;
    LocationDao* m_locationDao;
    OrderDao* m_orderDao;
    OrderItemDao* m_orderItemDao;
};
