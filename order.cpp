#include "Order.h"
#include "OrderItem.h" // Jeśli OrderItem.h jest dołączane do Order.h, usuń to.

Order::Order(int id, const QString& kontrahent, const QDateTime& data, const QString& status)
    : m_id(id),
      m_kontrahent(kontrahent),
      m_data(data),
      m_status(status)
{
}

Order::~Order()
{
    // Jeśli używasz QVector<OrderItem*> m_items, pamiętaj o dealokacji:
    // qDeleteAll(m_items);
}