#ifndef ORDERITEMDAO_H
#define ORDERITEMDAO_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVector>
#include <QMessageBox> // Tymczasowo dla komunikatów o błędach
#include <QDebug>

#include "OrderItem.h"

class OrderItemDao
{
private:
    QSqlDatabase& m_db;

public:
    explicit OrderItemDao(QSqlDatabase& db);

    // Metody CRUD dla pozycji zamówienia
    bool addOrderItem(const OrderItem& item);
    QVector<OrderItem*> getOrderItemsByOrderId(int orderId); // Pobiera pozycje dla konkretnego zamówienia
    bool updateOrderItemQuantity(int orderId, int productId, int newQuantity); // Aktualizuje ilość pozycji
    bool deleteOrderItem(int orderId, int productId); // Usuwa konkretną pozycję
    bool deleteOrderItemsByOrderId(int orderId); // Usuwa wszystkie pozycje dla danego zamówienia
};

#endif // ORDERITEMDAO_H