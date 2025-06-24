#ifndef IINVENTORYMANAGEMENT_H
#define IINVENTORYMANAGEMENT_H

// INTERFEJS: Definiuje funkcjonalności związane z zarządzaniem inwentarzem
class IInventoryManagement
{
public:
    virtual ~IInventoryManagement() = default; // Ważne dla interfejsów
    // Tutaj możesz zadeklarować czyste wirtualne funkcje dla zarządzania inwentarzem (np. updateStock, recordInventory)
    // virtual bool updateStock(int productId, int quantityChange) = 0;
};

#endif // IINVENTORYMANAGEMENT_H
