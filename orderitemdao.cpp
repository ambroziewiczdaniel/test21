#include "OrderItemDao.h"

OrderItemDao::OrderItemDao(QSqlDatabase& db)
    : m_db(db)
{
}

bool OrderItemDao::addOrderItem(const OrderItem& item)
{
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO order_items (order_id, product_id, quantity) VALUES (:order_id, :product_id, :quantity)");
    query.bindValue(":order_id", item.getOrderId());
    query.bindValue(":product_id", item.getProductId());
    query.bindValue(":quantity", item.getQuantity());

    if (!query.exec()) {
        qDebug() << "OrderItemDao: Blad dodawania pozycji zamowienia:" << query.lastError().text();
        QMessageBox::critical(nullptr, "Błąd Bazy Danych", "Nie można dodać pozycji zamówienia: " + query.lastError().text());
        return false;
    }
    return true;
}

QVector<OrderItem*> OrderItemDao::getOrderItemsByOrderId(int orderId)
{
    QVector<OrderItem*> items;
    QSqlQuery query(m_db);
    query.prepare("SELECT oi.product_id, p.name, oi.quantity "
                  "FROM order_items oi "
                  "JOIN products p ON oi.product_id = p.id "
                  "WHERE oi.order_id = :order_id");
    query.bindValue(":order_id", orderId);

    if (!query.exec()) {
        qDebug() << "OrderItemDao: Blad ladowania pozycji zamowienia dla ID" << orderId << ":" << query.lastError().text();
        QMessageBox::critical(nullptr, "Błąd Bazy Danych", "Nie można załadować pozycji zamówienia: " + query.lastError().text());
        return items;
    }

    while (query.next()) {
        items.append(new OrderItem(
            orderId,
            query.value("product_id").toInt(),
            query.value("name").toString(),
            query.value("quantity").toInt()
        ));
    }
    return items;
}

bool OrderItemDao::updateOrderItemQuantity(int orderId, int productId, int newQuantity)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE order_items SET quantity = :quantity WHERE order_id = :order_id AND product_id = :product_id");
    query.bindValue(":quantity", newQuantity);
    query.bindValue(":order_id", orderId);
    query.bindValue(":product_id", productId);

    if (!query.exec()) {
        qDebug() << "OrderItemDao: Blad aktualizacji ilosci pozycji zamowienia:" << query.lastError().text();
        QMessageBox::critical(nullptr, "Błąd Bazy Danych", "Nie można zaktualizować ilości pozycji zamówienia: " + query.lastError().text());
        return false;
    }
    return true;
}

bool OrderItemDao::deleteOrderItem(int orderId, int productId)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM order_items WHERE order_id = :order_id AND product_id = :product_id");
    query.bindValue(":order_id", orderId);
    query.bindValue(":product_id", productId);

    if (!query.exec()) {
        qDebug() << "OrderItemDao: Blad usuwania pozycji zamowienia:" << query.lastError().text();
        QMessageBox::critical(nullptr, "Błąd Bazy Danych", "Nie można usunąć pozycji zamówienia: " + query.lastError().text());
        return false;
    }
    return true;
}

bool OrderItemDao::deleteOrderItemsByOrderId(int orderId)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM order_items WHERE order_id = :order_id");
    query.bindValue(":order_id", orderId);

    if (!query.exec()) {
        qDebug() << "OrderItemDao: Blad usuwania wszystkich pozycji dla zamowienia ID" << orderId << ":" << query.lastError().text();
        QMessageBox::critical(nullptr, "Błąd Bazy Danych", "Nie można usunąć wszystkich pozycji zamówienia: " + query.lastError().text());
        return false;
    }
    return true;
}