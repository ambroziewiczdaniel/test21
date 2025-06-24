#ifndef USER_H
#define USER_H

#include <QString>

class User
{
private:
    int m_id;
    QString m_login;
    QString m_password; // W prawdziwej aplikacji hasło powinno być haszowane
    QString m_role;

public:
    // Konstruktor do tworzenia nowego użytkownika (ID może być -1 dla nowych rekordów z AUTOINCREMENT)
    User(int id, const QString& login, const QString& password, const QString& role);
    // Konstruktor dla istniejącego użytkownika (bez hasła, jeśli nie jest pobierane)
    User(int id, const QString& login, const QString& role);
    ~User();

    // Gettery
    int getId() const { return m_id; }
    QString getLogin() const { return m_login; }
    QString getPassword() const { return m_password; }
    QString getRole() const { return m_role; }

    // Settery (jeśli chcemy modyfikować obiekty User)
    void setId(int id) { m_id = id; } // Może być przydatne, jeśli ID jest generowane przez bazę
    void setLogin(const QString& login) { m_login = login; }
    void setPassword(const QString& password) { m_password = password; }
    void setRole(const QString& role) { m_role = role; }
};

#endif // USER_H