#ifndef ISTOCKTAKING_H
#define ISTOCKTAKING_H

// INTERFEJS: Definiuje funkcjonalności związane z przeprowadzaniem inwentaryzacji
class IStockTaking
{
public:
    virtual ~IStockTaking() = default; // Ważne dla interfejsów
    // Tutaj możesz zadeklarować czyste wirtualne funkcje dla inwentaryzacji (np. startCounting, compareStock)
    // virtual bool conductStockTake() = 0;
};

#endif // ISTOCKTAKING_H
