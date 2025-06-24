// main.cpp - Zaktualizowany, aby zawierać mainwindow.h
#include "GUI/loginwindow.h"
#include "GUI/mainwindow.h" // <--- DODAJ TĘ LINIĘ!
#include <QApplication>

#include "systemWarehouse.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    systemWarehouse* System = new systemWarehouse();

    LoginWindow loginWindow{System};
    if (loginWindow.exec() == QDialog::Accepted) {
        // Po udanym zalogowaniu (LoginWindow::accept() zostało wywołane),
        // tworzymy i pokazujemy główne okno aplikacji.
        // Przekazujemy rolę użytkownika do MainWindow
        //MainWindow w(loginWindow.getUserRole());
        MainWindow w(System);
        w.show();
        return a.exec();
    } else {
        // Użytkownik zamknął okno logowania lub anulował logowanie
        return 0;
    }
}
