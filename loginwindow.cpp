#include "loginwindow.h"
#include "mainwindow.h" // Potrzebne, aby po zalogowaniu otworzyć MainWindow
#include <QDebug> // Dodane dla qDebug

LoginWindow::LoginWindow(systemWarehouse* system, QWidget *parent) :
    QDialog(parent),
    loginLineEdit(nullptr),
    passwordLineEdit(nullptr),
    statusLabel(nullptr),
    loginButton(nullptr),
    System(system)
{
    setWindowTitle("Logowanie do Systemu Magazynowego");
    setFixedSize(400, 200); // Stały rozmiar okna logowania

    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *loginLabel = new QLabel("Login:", this);
    loginLineEdit = new QLineEdit(this);
    loginLineEdit->setPlaceholderText("Wprowadź login");

    QLabel *passwordLabel = new QLabel("Hasło:", this);
    passwordLineEdit = new QLineEdit(this);
    passwordLineEdit->setEchoMode(QLineEdit::Password); // Ukryj wpisywane hasło
    passwordLineEdit->setPlaceholderText("Wprowadź hasło");

    loginButton = new QPushButton("Zaloguj", this);
    statusLabel = new QLabel("", this); // Etykieta do wyświetlania statusu logowania

    layout->addWidget(loginLabel);
    layout->addWidget(loginLineEdit);
    layout->addWidget(passwordLabel);
    layout->addWidget(passwordLineEdit);
    layout->addWidget(loginButton);
    layout->addWidget(statusLabel);
    layout->addStretch(); // Rozciągnij, aby elementy były na górze

    setLayout(layout);

    // Połącz sygnał przycisku logowania ze slotem
    connect(loginButton, &QPushButton::clicked, this, &LoginWindow::onLoginButtonClicked);

    // Upewniamy się, że baza danych jest otwarta na początku działania LoginWindow
    // Opcjonalnie: możesz przenieść openDatabase() do konstruktora systemWarehouse,
    // jeśli baza ma być zawsze otwarta, gdy systemWarehouse istnieje.
    // Z Twojego pliku systemWarehouse.cpp wynika, że już to robisz, więc jest OK.
    if (!System->Database_isOpen()) {
        QMessageBox::critical(this, "Błąd Bazy Danych", "Nie można otworzyć bazy danych dla logowania.");
        loginButton->setEnabled(false); // Wyłącz przycisk, jeśli baza niedostępna
    }
}

LoginWindow::~LoginWindow()
{
    // Baza danych jest zarządzana przez systemWarehouse i powinna zostać zamknięta
    // przez systemWarehouse przy zakończeniu działania aplikacji.
    // Usunięcie linii System->closeDatabase() jest poprawne.
    qDebug() << "LoginWindow zniszczony.";
}

void LoginWindow::onLoginButtonClicked()
{
    QString login = loginLineEdit->text();
    QString password = passwordLineEdit->text();

    QString userRole = System->authorize_logIn(login.toStdString(), password.toStdString());

    if (!userRole.isEmpty() && System->Current_User) { // Sprawdzamy rolę i czy użytkownik został utworzony
        statusLabel->setText("Zalogowano poprawnie jako " + userRole + "!");
        QMessageBox::information(this, "Logowanie", "Zalogowano poprawnie!");
        accept(); // Zamyka okno logowania z wynikiem QDialog::Accepted
    } else {
        statusLabel->setText("Niepoprawny login lub hasło!");
        QMessageBox::warning(this, "Logowanie", "Niepoprawny login lub hasło!");
    }
}