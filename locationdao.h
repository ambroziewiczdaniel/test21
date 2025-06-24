#ifndef LOCATIONDAO_H
#define LOCATIONDAO_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVector>
#include <QDebug>
#include <QVariant>
#include <QStringList> // Potrzebne do parsowania GROUP_CONCAT

// Struktura do przechowywania danych o produkcie i jego lokalizacjach
// To będzie nasz "obiekt domenowy" dla złożonych danych magazynowych
struct WarehouseProductData {
    int id;
    QString name;
    int minimalQuantity; // Ilość z tabeli products
    int totalActualQuantity; // Suma z tabeli locations
    QString locationsSummary; // Zsumowane lokalizacje
};

// Klasa LocationDao będzie odpowiedzialna za operacje na tabeli 'locations'
// oraz za złożone zapytania łączące 'products' i 'locations'.
class LocationDao
{
private:
    QSqlDatabase& m_db;

public:
    explicit LocationDao(QSqlDatabase& db);

    // Metody do pobierania danych o stanie magazynowym
    QVector<WarehouseProductData> getWarehouseProducts(const QString& filterText = "");

    // Metody do zarządzania lokalizacjami konkretnego produktu
    // (można je rozbudować w przyszłości o dodawanie/aktualizowanie/usuwanie konkretnych lokalizacji)
    bool updateProductLocationQuantity(int productId, int regal, int polka, int kolumna, int newQuantity);
    int getProductQuantityInLocation(int productId, int regal, int polka, int kolumna);
};

#endif // LOCATIONDAO_H