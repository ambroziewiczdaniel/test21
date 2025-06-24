#include "systemWarehouse.h"
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QPushButton>
#include <QVariant>
#include <QHBoxLayout> // To include jest używane w MainWindow, ale nie w tym pliku. Może być usunięte.

systemWarehouse::systemWarehouse()
{
    // Database Initialization
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("database.db");
    openDatabase();

    m_productDao = new ProductDao(db);
    m_locationDao = new LocationDao(db);
    m_orderDao = new OrderDao(db);
    m_orderItemDao = new OrderItemDao(db);
    m_userDao = new UserDao(db);

    loadProducts();
}

systemWarehouse::~systemWarehouse()
{
    qDeleteAll(products);
    delete m_productDao;
    delete m_locationDao;
    delete m_orderDao;
    delete m_orderItemDao;
    delete m_userDao;
}

void systemWarehouse::loadProducts()
{
    qDeleteAll(products);
    products.clear();

    products = m_productDao->getAllProducts();

    qDebug() << "Zaladowano" << products.size() << "produktow ;)" << "Przez ProductDao.";
}


QString systemWarehouse::authorize_logIn(std::string username, std::string password)
{
    // Używamy UserDao do autoryzacji
    QString role = m_userDao->authenticateUser(QString::fromStdString(username), QString::fromStdString(password));

    if (!role.isEmpty()) {
        Current_User = new UserClass(role); // UserClass powinien być inicjalizowany rolą
        return role;
    } else {
        return ""; // Nieudana autoryzacja
    }
}

QSqlQuery systemWarehouse::getWarehouseProductQuery(const QString& filterText)
{
    QSqlQuery query(db);
    QString baseQuery =
        "SELECT "
        "p.id, "
        "p.name, "
        "p.quantity AS minimal_quantity, "
        "COALESCE(SUM(l.ilosc), 0) AS total_actual_quantity, "
        "GROUP_CONCAT(l.regal || '-' || l.polka || '-' || l.kolumna || ' (Ilość: ' || l.ilosc || ')', '; ') AS locations_summary "
        "FROM products p "
        "LEFT JOIN locations l ON p.id = l.id ";

    if (filterText.isEmpty()) {
        query.prepare(baseQuery + "GROUP BY p.id, p.name, p.quantity ORDER BY p.name;");
    } else {
        query.prepare(baseQuery +
                      "WHERE UPPER(p.name) LIKE UPPER(:filterText) "
                      "GROUP BY p.id, p.name, p.quantity ORDER BY p.name;");
        query.bindValue(":filterText", "%" + filterText + "%");
    }
    return query;
}

QSqlQuery systemWarehouse::getOrdersQuery(const QString& filterText)
{
    QSqlQuery query(db);
    QString baseQuery =
        "SELECT "
        "id_zamowienia, "
        "kontrahent, "
        "data, "
        "status "
        "FROM zamowienia ";

    if (filterText.isEmpty()) {
        query.prepare(baseQuery + "ORDER BY data DESC;");
    } else {
        query.prepare(baseQuery +
                      "WHERE UPPER(kontrahent) LIKE UPPER(:filterText) "
                      "OR UPPER(status) LIKE UPPER(:filterText) "
                      "ORDER BY data DESC;");
        query.bindValue(":filterText", "%" + filterText + "%");
    }
    return query;
}

bool systemWarehouse::openDatabase()
{
    if (!db.open()) {
        QWidget* active_window = QApplication::activeWindow();
        QMessageBox::critical(active_window, "Błąd Bazy Danych", "Nie można otworzyć bazy danych\\n"
                                                                 "Upewnij się że istnieje sprawny plik: " + QDir::currentPath() + "/database.db \\n");

        qDebug() << "Błąd otwarcia database.db dla logowania:" << db.lastError().text();
    } else {
        qDebug() << "Baza danych otwarta dla logowania.";
    }
    return db.isOpen();
}
