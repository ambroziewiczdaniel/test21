#include "User.h"

User::User(int id, const QString& login, const QString& password, const QString& role)
    : m_id(id),
    m_login(login),
    m_password(password),
    m_role(role)
{
}

User::User(int id, const QString& login, const QString& role)
    : m_id(id),
    m_login(login),
    m_password(""), // Brak hasła, jeśli nie było w zapytaniu
    m_role(role)
{
}

User::~User()
{
    // Brak dynamicznie alokowanej pamięci do zwolnienia w tej klasie
}
