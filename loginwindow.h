#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QDir> // For current path in error messages

#include "../systemWarehouse.h"

class LoginWindow : public QDialog
{
    Q_OBJECT

public:
    explicit LoginWindow(systemWarehouse* system ,QWidget *parent = nullptr);
    ~LoginWindow();

private slots:
    void onLoginButtonClicked();

private:
    QLineEdit *loginLineEdit;
    QLineEdit *passwordLineEdit;
    QLabel *statusLabel;
    QPushButton *loginButton;

    systemWarehouse* System;
};

#endif // LOGINWINDOW_H