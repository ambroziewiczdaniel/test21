#include "OrderItem.h"

OrderItem::OrderItem(int orderId, int productId, const QString& productName, int quantity)
    : m_orderId(orderId),
      m_productId(productId),
      m_productName(productName),
      m_quantity(quantity)
{
}

OrderItem::~OrderItem()
{
    // Brak dynamicznie alokowanej pamięci do zwolnienia
}