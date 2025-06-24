#include "LocationDao.h"
#include <QMessageBox> // Dla komunikatów o błędach, tymczasowo

LocationDao::LocationDao(QSqlDatabase& db)
    : m_db(db)
{
}

QVector<WarehouseProductData> LocationDao::getWarehouseProducts(const QString& filterText)
{
    QVector<WarehouseProductData> data;
    QSqlQuery query(m_db);

    QString baseQuery =
        "SELECT "
        "p.id, "
        "p.name, "
        "p.quantity AS minimal_quantity, " // minimalna ilość z tabeli products
        "COALESCE(SUM(l.ilosc), 0) AS total_actual_quantity, " // całkowita ilość z lokalizacji (0 jeśli brak lokalizacji)
        "GROUP_CONCAT(l.regal || '-' || l.polka || '-' || l.kolumna || ' (Ilość: ' || l.ilosc || ')', '; ') AS locations_summary "
        "FROM products p "
        "LEFT JOIN locations l ON p.id = l.id "; // LEFT JOIN, aby pokazać produkty bez lokalizacji

    if (filterText.isEmpty()) {
        query.prepare(baseQuery + "GROUP BY p.id, p.name, p.quantity ORDER BY p.name;");
    } else {
        query.prepare(baseQuery +
                      "WHERE UPPER(p.name) LIKE UPPER(:filterText) "
                      "GROUP BY p.id, p.name, p.quantity ORDER BY p.name;");
        query.bindValue(":filterText", "%" + filterText + "%");
    }

    if (!query.exec()) {
        qDebug() << "LocationDao: Blad ladowania danych magazynu:" << query.lastError().text();
        QMessageBox::critical(nullptr, "Błąd Bazy Danych (Magazyn)",
                              "Błąd podczas ładowania danych magazynu: " + query.lastError().text() + "\nZapytanie: " + query.executedQuery());
        return data;
    }

    while (query.next()) {
        WarehouseProductData row;
        row.id = query.value("id").toInt();
        row.name = query.value("name").toString();
        row.minimalQuantity = query.value("minimal_quantity").toInt();
        row.totalActualQuantity = query.value("total_actual_quantity").toInt();
        row.locationsSummary = query.value("locations_summary").toString();
        data.append(row);
    }
    return data;
}

bool LocationDao::updateProductLocationQuantity(int productId, int regal, int polka, int kolumna, int newQuantity)
{
    // Sprawdzamy, czy lokalizacja istnieje
    QSqlQuery checkQuery(m_db);
    checkQuery.prepare("SELECT COUNT(*) FROM locations WHERE id = :productId AND regal = :regal AND polka = :polka AND kolumna = :kolumna");
    checkQuery.bindValue(":productId", productId);
    checkQuery.bindValue(":regal", regal);
    checkQuery.bindValue(":polka", polka);
    checkQuery.bindValue(":kolumna", kolumna);

    if (!checkQuery.exec()) {
        qDebug() << "LocationDao: Blad sprawdzania istnienia lokalizacji:" << checkQuery.lastError().text();
        QMessageBox::critical(nullptr, "Błąd Bazy Danych", "Błąd sprawdzania lokalizacji: " + checkQuery.lastError().text());
        return false;
    }
    checkQuery.next();
    bool locationExists = (checkQuery.value(0).toInt() > 0);

    QSqlQuery updateQuery(m_db);
    if (locationExists) {
        if (newQuantity > 0) {
            updateQuery.prepare("UPDATE locations SET ilosc = :newQuantity WHERE id = :productId AND regal = :regal AND polka = :polka AND kolumna = :kolumna");
        } else {
            // Jeśli nowa ilość to 0, usuwamy lokalizację
            updateQuery.prepare("DELETE FROM locations WHERE id = :productId AND regal = :regal AND polka = :polka AND kolumna = :kolumna");
        }
    } else {
        // Dodajemy nową lokalizację tylko jeśli nowa ilość > 0
        if (newQuantity > 0) {
            updateQuery.prepare("INSERT INTO locations (id, regal, polka, kolumna, ilosc) VALUES (:productId, :regal, :polka, :kolumna, :newQuantity)");
        } else {
            // Nie ma sensu dodawać lokalizacji z ilością 0, więc nic nie robimy
            return true; // Uznajemy za sukces, bo celem było osiągnięcie 0
        }
    }

    updateQuery.bindValue(":productId", productId);
    updateQuery.bindValue(":regal", regal);
    updateQuery.bindValue(":polka", polka);
    updateQuery.bindValue(":kolumna", kolumna);
    if (newQuantity > 0 || !locationExists) { // Bindowanie tylko jeśli jest użyte w UPDATE/INSERT
        updateQuery.bindValue(":newQuantity", newQuantity);
    }

    if (!updateQuery.exec()) {
        qDebug() << "LocationDao: Blad aktualizacji/dodawania/usuwania lokalizacji:" << updateQuery.lastError().text();
        QMessageBox::critical(nullptr, "Błąd Bazy Danych", "Nie można zaktualizować/dodać/usunąć ilości w lokalizacji: " + updateQuery.lastError().text());
        return false;
    }
    return true;
}

int LocationDao::getProductQuantityInLocation(int productId, int regal, int polka, int kolumna)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT ilosc FROM locations WHERE id = :productId AND regal = :regal AND polka = :polka AND kolumna = :kolumna");
    query.bindValue(":productId", productId);
    query.bindValue(":regal", regal);
    query.bindValue(":polka", polka);
    query.bindValue(":kolumna", kolumna);

    if (query.exec() && query.next()) {
        return query.value("ilosc").toInt();
    } else {
        qDebug() << "LocationDao: Nie znaleziono ilosci dla lokalizacji" << productId << regal << polka << kolumna << "lub blad:" << query.lastError().text();
        return 0; // Zwraca 0, jeśli brak lokalizacji lub błąd
    }
}