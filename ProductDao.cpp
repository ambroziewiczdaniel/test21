#include "ProductDao.h"

ProductDao::ProductDao(QSqlDatabase& db)
    : m_db(db)
{
    // Konstruktor DAO. Połączenie z bazą danych jest zarządzane przez systemWarehouse.
}

bool ProductDao::addProduct(const Product& product)
{
    QSqlQuery query(m_db);
    // Używamy nazwy kolumny 'id' w tabeli 'products'. Jeśli masz inną nazwę, zmień tu.
    query.prepare("INSERT INTO products (id, name, quantity) VALUES (:id, :name, :quantity)");
    query.bindValue(":id", product.get_id());
    query.bindValue(":name", product.get_name());
    query.bindValue(":quantity", product.get_quantity());

    if (!query.exec()) {
        qDebug() << "ProductDao: Blad dodawania produktu:" << query.lastError().text();
        QMessageBox::critical(nullptr, "Błąd Bazy Danych", "Nie można dodać produktu: " + query.lastError().text());
        return false;
    }
    return true;
}

QVector<Product*> ProductDao::getAllProducts()
{
    QVector<Product*> products;
    QSqlQuery query("SELECT id, name, quantity FROM products", m_db); // Query to select all products

    // Crucial: Execute the query here
    if (!query.exec()) {
        qDebug() << "ProductDao: Blad ladowania wszystkich produktow (exec):" << query.lastError().text();
        QMessageBox::critical(nullptr, "Błąd Bazy Danych", "Nie można załadować produktów: " + query.lastError().text());
        return products; // Return empty vector if execution failed
    }

    // Loop through results only if query.exec() was successful
    while (query.next()) {
        products.append(new Product(
            query.value("id").toInt(),
            query.value("name").toString(),
            query.value("quantity").toInt()
            ));
    }
    return products;
}

QVector<Product*> ProductDao::getFilteredProducts(const QString& filterText)
{
    QVector<Product*> products;
    QSqlQuery query(m_db);
    query.prepare("SELECT id, name, quantity FROM products WHERE UPPER(name) LIKE UPPER(:filterText) ORDER BY name");
    query.bindValue(":filterText", "%" + filterText + "%");

    if (!query.exec()) {
        qDebug() << "ProductDao: Blad ladowania filtrowanych produktow:" << query.lastError().text();
        QMessageBox::critical(nullptr, "Błąd Bazy Danych", "Nie można załadować filtrowanych produktów: " + query.lastError().text());
        return products;
    }

    while (query.next()) {
        products.append(new Product(
            query.value("id").toInt(),
            query.value("name").toString(),
            query.value("quantity").toInt()
        ));
    }
    return products;
}

Product* ProductDao::getProductById(int id)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT id, name, quantity FROM products WHERE id = :id");
    query.bindValue(":id", id);

    if (query.exec() && query.next()) {
        return new Product(
            query.value("id").toInt(),
            query.value("name").toString(),
            query.value("quantity").toInt()
        );
    } else {
        qDebug() << "ProductDao: Nie znaleziono produktu o ID" << id << "lub blad:" << query.lastError().text();
        return nullptr;
    }
}

bool ProductDao::updateProduct(const Product& product)
{
    QSqlQuery query(m_db);
    query.prepare("UPDATE products SET name = :name, quantity = :quantity WHERE id = :id");
    query.bindValue(":name", product.get_name());
    query.bindValue(":quantity", product.get_quantity());
    query.bindValue(":id", product.get_id());

    if (!query.exec()) {
        qDebug() << "ProductDao: Blad aktualizacji produktu:" << query.lastError().text();
        QMessageBox::critical(nullptr, "Błąd Bazy Danych", "Nie można zaktualizować produktu: " + query.lastError().text());
        return false;
    }
    return true;
}

bool ProductDao::deleteProduct(int id)
{
    // Przed usunięciem produktu, usuwamy powiązane lokalizacje (jeśli istnieją)
    // UWAGA: Lepszym rozwiązaniem jest CASCADE DELETE w definicji FOREIGN KEY tabeli 'locations'
    // Jeśli masz CASCADE DELETE w bazie danych, poniższy blok jest zbędny.
    QSqlQuery deleteLocationsQuery(m_db);
    deleteLocationsQuery.prepare("DELETE FROM locations WHERE id = :productId");
    deleteLocationsQuery.bindValue(":productId", id);
    if (!deleteLocationsQuery.exec()) {
        qDebug() << "ProductDao: Blad usuwania lokalizacji dla produktu ID" << id << ":" << deleteLocationsQuery.lastError().text();
        QMessageBox::critical(nullptr, "Błąd Bazy Danych", "Nie można usunąć lokalizacji dla produktu: " + deleteLocationsQuery.lastError().text());
        return false;
    }

    QSqlQuery query(m_db);
    query.prepare("DELETE FROM products WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qDebug() << "ProductDao: Blad usuwania produktu:" << query.lastError().text();
        QMessageBox::critical(nullptr, "Błąd Bazy Danych", "Nie można usunąć produktu: " + query.lastError().text());
        return false;
    }
    return true;
}

bool ProductDao::productExists(const QString& name)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT COUNT(*) FROM products WHERE name = :name");
    query.bindValue(":name", name);
    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }
    qDebug() << "ProductDao: Blad sprawdzania istnienia produktu po nazwie:" << query.lastError().text();
    return false;
}

bool ProductDao::productExists(int id)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT COUNT(*) FROM products WHERE id = :id");
    query.bindValue(":id", id);
    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }
    qDebug() << "ProductDao: Blad sprawdzania istnienia produktu po ID:" << query.lastError().text();
    return false;
}

int ProductDao::getProductIdByName(const QString& name)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT id FROM products WHERE name = :name");
    query.bindValue(":name", name);
    if (query.exec() && query.next()) {
        return query.value("id").toInt();
    }
    qDebug() << "ProductDao: Nie znaleziono ID dla produktu o nazwie:" << name << "lub blad:" << query.lastError().text();
    return -1;
}
