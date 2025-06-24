#ifndef ORDERDAO_H
#define ORDERDAO_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVector>
#include <QMessageBox> // Tymczasowo dla komunikatów o błędach
#include <QDebug>

#include "Order.h"
#include "OrderItem.h" // OrderDao może potrzebować dostępu do OrderItemDao lub ładować OrderItems

class OrderDao
{
private:
    QSqlDatabase& m_db;

public:
    explicit OrderDao(QSqlDatabase& db);

    // Metody CRUD dla zamówień
    int addOrder(const Order& order); // Zwraca ID nowo dodanego zamówienia
    Order* getOrderById(int orderId); // Pobiera pojedyncze zamówienie
    QVector<Order*> getAllOrders(); // Pobiera wszystkie zamówienia
    QVector<Order*> getFilteredOrders(const QString& filterText); // Filtrowane zamówienia
    bool updateOrder(const Order& order); // Aktualizuje zamówienie
    bool deleteOrder(int orderId); // Usuwa zamówienie (i powiązane pozycje, jeśli CASCADE DELETE)

    // Metody do obsługi statusu (można przenieść do OrderService, ale na razie tu)
    bool updateOrderStatus(int orderId, const QString& newStatus);
};

#endif // ORDERDAO_H