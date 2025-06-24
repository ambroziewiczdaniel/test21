#include "OrderDao.h"

OrderDao::OrderDao(QSqlDatabase& db)
    : m_db(db)
{
}

int OrderDao::addOrder(const Order& order)
{
    QSqlQuery query(m_db);
    // Używamy id_zamowienia jako klucza głównego z autoincrement
    query.prepare("INSERT INTO zamowienia (kontrahent, data, status) VALUES (:kontrahent, :data, :status)");
    query.bindValue(":kontrahent", order.getKontrahent());
    query.bindValue(":data", order.getData().toString("yyyy-MM-dd hh:mm:ss"));
    query.bindValue(":status", order.getStatus());

    if (query.exec()) {
        return query.lastInsertId().toInt(); // Zwraca ID nowo dodanego zamówienia
    } else {
        qDebug() << "OrderDao: Blad dodawania zamowienia:" << query.lastError().text();
        QMessageBox::critical(nullptr, "Błąd Bazy Danych", "Nie można dodać zamówienia: " + query.lastError().text());
        return -1; // Wskazuje na błąd
    }
}

Order* OrderDao::getOrderById(int orderId)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT id_zamowienia, kontrahent, data, status FROM zamowienia WHERE id_zamowienia = :id_zamowienia");
    query.bindValue(":id_zamowienia", orderId);

    if (query.exec() && query.next()) {
        return new Order(
            query.value("id_zamowienia").toInt(),
            query.value("kontrahent").toString(),
            query.value("data").toDateTime(),
            query.value("status").toString()
        );
    } else {
        qDebug() << "OrderDao: Nie znaleziono zamowienia o ID" << orderId << "lub blad:" << query.lastError().text();
        return nullptr;
    }
}

QVector<Order*> OrderDao::getAllOrders()
{
    QVector<Order*> orders;
    QSqlQuery query("SELECT id_zamowienia, kontrahent, data, status FROM zamowienia ORDER BY data DESC", m_db);

    if (!query.exec()) {
        qDebug() << "OrderDao: Blad ladowania wszystkich zamowien:" << query.lastError().text();
        QMessageBox::critical(nullptr, "Błąd Bazy Danych", "Nie można załadować zamówień: " + query.lastError().text());
        return orders;
    }

    while (query.next()) {
        orders.append(new Order(
            query.value("id_zamowienia").toInt(),
            query.value("kontrahent").toString(),
            query.value("data").toDateTime(),
            query.value("status").toString()
        ));
    }
    return orders;
}

QVector<Order*> OrderDao::getFilteredOrders(const QString& filterText)
{
    QVector<Order*> orders;
    QSqlQuery query(m_db);
    query.prepare("SELECT id_zamowienia, kontrahent, data, status FROM zamowienia "
                  "WHERE UPPER(kontrahent) LIKE UPPER(:filterText) OR UPPER(status) LIKE UPPER(:filterText) "
                  "ORDER BY data DESC");
    query.bindValue(":filterText", "%" + filterText + "%");

    if (!query.exec()) {
        qDebug() << "OrderDao: Blad ladowania filtrowanych zamowien:" << query.lastError().text();
        QMessageBox::critical(nullptr, "Błąd Bazy Danych", "Nie można załadować filtrowanych zamówień: " + query.lastError().text());
        return orders;
    }

    while (query.next()) {
        orders.append(new Order(
            query.value("id_zamowienia").toInt(),
            query.value("kontrahent").toString(),
            query.value("data").toDateTime(),
            query.value("status").toString()
        ));
    }
    return orders;
}

bool OrderDao::updateOrder(const Order& order)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE zamowienia SET kontrahent = :kontrahent, data = :data, status = :status WHERE id_zamowienia = :id_zamowienia");
    query.bindValue(":kontrahent", order.getKontrahent());
    query.bindValue(":data", order.getData().toString("yyyy-MM-dd hh:mm:ss"));
    query.bindValue(":status", order.getStatus());
    query.bindValue(":id_zamowienia", order.getId());

    if (!query.exec()) {
        qDebug() << "OrderDao: Blad aktualizacji zamowienia:" << query.lastError().text();
        QMessageBox::critical(nullptr, "Błąd Bazy Danych", "Nie można zaktualizować zamówienia: " + query.lastError().text());
        return false;
    }
    return true;
}

bool OrderDao::deleteOrder(int orderId)
{
    // UWAGA: Zakładam, że masz CASCADE DELETE w FOREIGN KEY dla order_items
    // Jeśli nie, musisz najpierw usunąć pozycje zamówienia z order_items.
    // QSqlQuery deleteItemsQuery(m_db);
    // deleteItemsQuery.prepare("DELETE FROM order_items WHERE order_id = :orderId");
    // deleteItemsQuery.bindValue(":orderId", orderId);
    // if (!deleteItemsQuery.exec()) { ... return false; }

    QSqlQuery query(m_db);
    query.prepare("DELETE FROM zamowienia WHERE id_zamowienia = :id_zamowienia");
    query.bindValue(":id_zamowienia", orderId);

    if (!query.exec()) {
        qDebug() << "OrderDao: Blad usuwania zamowienia:" << query.lastError().text();
        QMessageBox::critical(nullptr, "Błąd Bazy Danych", "Nie można usunąć zamówienia: " + query.lastError().text());
        return false;
    }
    return true;
}

bool OrderDao::updateOrderStatus(int orderId, const QString& newStatus)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE zamowienia SET status = :status WHERE id_zamowienia = :id_zamowienia");
    query.bindValue(":status", newStatus);
    query.bindValue(":id_zamowienia", orderId);

    if (!query.exec()) {
        qDebug() << "OrderDao: Blad aktualizacji statusu zamowienia:" << query.lastError().text();
        QMessageBox::critical(nullptr, "Błąd Bazy Danych", "Nie można zaktualizować statusu zamówienia: " + query.lastError().text());
        return false;
    }
    return true;
}