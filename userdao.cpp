#include "UserDao.h"

UserDao::UserDao(QSqlDatabase& db)
    : m_db(db)
{
}

bool UserDao::addUser(const User& user)
{
    QSqlQuery query(m_db);
    // Jeśli tabela 'users' ma AUTOINCREMENT dla ID, możesz pominąć kolumnę 'id' w INSERT
    query.prepare("INSERT INTO users (login, password, role) VALUES (:login, :password, :role)");
    query.bindValue(":login", user.getLogin());
    query.bindValue(":password", user.getPassword());
    query.bindValue(":role", user.getRole());

    if (!query.exec()) {
        qDebug() << "UserDao: Blad dodawania uzytkownika:" << query.lastError().text();
        QMessageBox::critical(nullptr, "Błąd Bazy Danych", "Nie można dodać użytkownika: " + query.lastError().text());
        return false;
    }
    return true;
}

User* UserDao::getUserById(int id)
{
    QSqlQuery query(m_db);
    // Zakładam, że masz kolumnę 'id' w tabeli users, jeśli nie, użyj 'login'
    query.prepare("SELECT id, login, role FROM users WHERE id = :id");
    query.bindValue(":id", id);

    if (query.exec() && query.next()) {
        return new User(
            query.value("id").toInt(),
            query.value("login").toString(),
            query.value("role").toString()
            );
    } else {
        qDebug() << "UserDao: Nie znaleziono uzytkownika o ID" << id << "lub blad:" << query.lastError().text();
        return nullptr;
    }
}

User* UserDao::getUserByLogin(const QString& login)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT id, login, role FROM users WHERE login = :login");
    query.bindValue(":login", login);

    if (query.exec() && query.next()) {
        return new User(
            query.value("id").toInt(),
            query.value("login").toString(),
            query.value("role").toString()
            );
    } else {
        qDebug() << "UserDao: Nie znaleziono uzytkownika o loginie" << login << "lub blad:" << query.lastError().text();
        return nullptr;
    }
}

std::vector<User*> UserDao::getAllUsers()
{
    std::vector<User*> users;
    QSqlQuery query("SELECT id, login, role FROM users", m_db); // Nie pobieramy hasła

    if (!query.exec()) {
        qDebug() << "UserDao: Blad ladowania wszystkich uzytkownikow:" << query.lastError().text();
        QMessageBox::critical(nullptr, "Błąd Bazy Danych", "Nie można załadować użytkowników: " + query.lastError().text());
        return users;
    }

    while (query.next()) {
        users.push_back(new User(
            query.value("id").toInt(),
            query.value("login").toString(),
            query.value("role").toString()
            ));
    }
    return users;
}

bool UserDao::updateUser(const User& user)
{
    QSqlQuery query(m_db);
    // Aktualizujemy tylko login i rolę. Hasło zmieniane jest osobno lub nie jest tu zarządzane.
    query.prepare("UPDATE users SET login = :login, role = :role WHERE id = :id");
    query.bindValue(":login", user.getLogin());
    query.bindValue(":role", user.getRole());
    query.bindValue(":id", user.getId());

    if (!query.exec()) {
        qDebug() << "UserDao: Blad aktualizacji uzytkownika:" << query.lastError().text();
        QMessageBox::critical(nullptr, "Błąd Bazy Danych", "Nie można zaktualizować użytkownika: " + query.lastError().text());
        return false;
    }
    return true;
}

bool UserDao::deleteUser(int id)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM users WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qDebug() << "UserDao: Blad usuwania uzytkownika:" << query.lastError().text();
        QMessageBox::critical(nullptr, "Błąd Bazy Danych", "Nie można usunąć użytkownika: " + query.lastError().text());
        return false;
    }
    return true;
}

bool UserDao::userExists(const QString& login)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT COUNT(*) FROM users WHERE login = :login");
    query.bindValue(":login", login);
    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }
    qDebug() << "UserDao: Blad sprawdzania istnienia uzytkownika po loginie:" << query.lastError().text();
    return false;
}

QString UserDao::authenticateUser(const QString& login, const QString& password)
{
    QSqlQuery query(m_db);
    query.prepare("SELECT role FROM users WHERE login = :login AND password = :password");
    query.bindValue(":login", login);
    query.bindValue(":password", password); // W realnej aplikacji hasła powinny być haszowane!

    if (query.exec() && query.next()) {
        return query.value("role").toString();
    } else {
        qDebug() << "UserDao: Blad uwierzytelniania lub niepoprawne dane logowania:" << query.lastError().text();
        return ""; // Pusta rola w przypadku błędu lub nieudanej autoryzacji
    }
}
