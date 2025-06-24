#ifndef USERACCOUNTANT_H
#define USERACCOUNTANT_H

#include "UserClass.h"
#include "IOrderFulfillment.h"   // Upewnij się, że ten plik istnieje
#include "IInventoryManagement.h" // Upewnij się, że ten plik istnieje

class userAccountant : public UserClass, public IOrderFulfillment, public IInventoryManagement
{
public:
    userAccountant();
    ~userAccountant();

    // Tutaj musisz zaimplementować czyste wirtualne funkcje z IOrderFulfillment i IInventoryManagement
    // Przykład:
    // virtual bool processOrder(int orderId) override;
    // virtual bool updateStock(int productId, int quantityChange) override;
};

#endif // USERACCOUNTANT_H
