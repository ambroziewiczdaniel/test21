#ifndef ORDER_H
#define ORDER_H

#include <QString>
#include <QDateTime>
#include <QVector> // Jeśli będziemy przechowywać OrderItems bezpośrednio w Order

class OrderItem; // Forward declaration

class Order
{
private:
    int m_id;
    QString m_kontrahent;
    QDateTime m_data;
    QString m_status;
    // Opcjonalnie: QVector<OrderItem*> m_items; // Jeśli chcesz, aby Order "zawierał" swoje pozycje

public:
    Order(int id, const QString& kontrahent, const QDateTime& data, const QString& status);
    // Jeśli używasz m_items, dodaj Order(int id, const QString& kontrahent, const QDateTime& data, const QString& status, const QVector<OrderItem*>& items);
    ~Order();

    // Gettery
    int getId() const { return m_id; }
    QString getKontrahent() const { return m_kontrahent; }
    QDateTime getData() const { return m_data; }
    QString getStatus() const { return m_status; }
    // const QVector<OrderItem*>& getItems() const { return m_items; } // Jeśli używasz m_items

    // Settery (jeśli encja ma być modyfikowalna, inaczej tworzymy nowe obiekty Product/Order)
    void setKontrahent(const QString& kontrahent) { m_kontrahent = kontrahent; }
    void setData(const QDateTime& data) { m_data = data; }
    void setStatus(const QString& status) { m_status = status; }
    // void setItems(const QVector<OrderItem*>& items) { m_items = items; } // Jeśli używasz m_items
};

#endif // ORDER_H