#ifndef USERCLASS_H
#define USERCLASS_H

#include <QString>

// Usuwamy dziedziczenie z IOrderFulfillment i IStockTaking
// #include "IOrderFulfillment.h" // Niepotrzebne tu
// #include "IStockTaking.h" // Niepotrzebne tu

class UserClass // Nie dziedziczy już z interfejsów ról
{
public:
    QString userType; // Może być prywatne z getterem

public:
    UserClass();
    UserClass(QString permission_type);
    ~UserClass();

    const QString get_user_type() const { return userType; }
};

#endif // USERCLASS_H
