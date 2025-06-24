#include "loginwindow.h"
#include "mainwindow.h" // Potrzebne, aby po zalogowaniu otworzyć MainWindow
#include <QDebug>

LoginWindow::LoginWindow(systemWarehouse* system, QWidget *parent) :
    QDialog(parent),
    loginLineEdit(nullptr),
    passwordLineEdit(nullptr),
    statusLabel(nullptr),
    loginButton(nullptr),
    System(system) // Używamy 'System' w tej klasie, to zgodne z Twoją konwencją
{
    setWindowTitle("Logowanie do Systemu Magazynowego");
    setFixedSize(400, 200);

    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *loginLabel = new QLabel("Login:", this);
    loginLineEdit = new QLineEdit(this);
    loginLineEdit->setPlaceholderText("Wprowadź login");

    QLabel *passwordLabel = new QLabel("Hasło:", this);
    passwordLineEdit = new QLineEdit(this);
    passwordLineEdit->setEchoMode(QLineEdit::Password);
    passwordLineEdit->setPlaceholderText("Wprowadź hasło");

    loginButton = new QPushButton("Zaloguj", this);
    statusLabel = new QLabel("", this);

    layout->addWidget(loginLabel);
    layout->addWidget(loginLineEdit);
    layout->addWidget(passwordLabel);
    layout->addWidget(passwordLineEdit);
    layout->addWidget(loginButton);
    layout->addWidget(statusLabel);
    layout->addStretch();

    setLayout(layout);

    connect(loginButton, &QPushButton::clicked, this, &LoginWindow::onLoginButtonClicked);

    if (!System->Database_isOpen()) {
        QMessageBox::critical(this, "Błąd Bazy Danych", "Nie można otworzyć bazy danych dla logowania.");
        loginButton->setEnabled(false);
    }
}

LoginWindow::~LoginWindow()
{
    qDebug() << "LoginWindow zniszczony.";
}

void LoginWindow::onLoginButtonClicked()
{
    QString login = loginLineEdit->text();
    QString password = passwordLineEdit->text();

    // Wywołujemy authorize_logIn w systemWarehouse, które teraz użyje UserDao
    QString userRole = System->authorize_logIn(login.toStdString(), password.toStdString());

    if (!userRole.isEmpty()) { // Jeśli rola nie jest pusta, autoryzacja się powiodła
        statusLabel->setText("Zalogowano poprawnie jako " + userRole + "!");
        QMessageBox::information(this, "Logowanie", "Zalogowano poprawnie!");
        accept(); // Zamyka okno logowania z wynikiem QDialog::Accepted
    } else {
        statusLabel->setText("Niepoprawny login lub hasło!");
        QMessageBox::warning(this, "Logowanie", "Niepoprawny login lub hasło!");
    }
}
