#ifndef ORDERITEM_H
#define ORDERITEM_H

#include <QString>

class OrderItem
{
private:
    int m_orderId;   // Klucz obcy do tabeli zamowienia (id_zamowienia)
    int m_productId; // Klucz obcy do tabeli products (id)
    QString m_productName; // Dla wygody wyświetlania (nie jest w bazie w order_items)
    int m_quantity;

public:
    OrderItem(int orderId, int productId, const QString& productName, int quantity);
    ~OrderItem();

    // Gettery
    int getOrderId() const { return m_orderId; }
    int getProductId() const { return m_productId; }
    QString getProductName() const { return m_productName; }
    int getQuantity() const { return m_quantity; }

    // Settery
    void setQuantity(int quantity) { m_quantity = quantity; }
};

#endif // ORDERITEM_H