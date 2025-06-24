#ifndef USERDAO_H
#define USERDAO_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <vector> // Używamy std::vector do zwracania kolekcji
#include <QMessageBox> // Tymczasowo dla komunikatów o błędach
#include <QDebug>

#include "User.h" // Potrzebujemy definicji klasy User

class UserDao
{
private:
    QSqlDatabase& m_db; // Referencja do instancji bazy danych

public:
    explicit UserDao(QSqlDatabase& db);

    // Metody CRUD dla użytkowników
    bool addUser(const User& user); // Dodaje nowego użytkownika
    User* getUserById(int id); // Pobiera użytkownika po ID
    User* getUserByLogin(const QString& login); // Pobiera użytkownika po loginie
    std::vector<User*> getAllUsers(); // Pobiera wszystkich użytkowników
    bool updateUser(const User& user); // Aktualizuje istniejącego użytkownika
    bool deleteUser(int id); // Usuwa użytkownika po ID

    // Metody pomocnicze/walidacyjne
    bool userExists(const QString& login); // Sprawdza, czy użytkownik o danym loginie istnieje
    QString authenticateUser(const QString& login, const QString& password); // Uwierzytelnia użytkownika i zwraca rolę
};

#endif // USERDAO_H