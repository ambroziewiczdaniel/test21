#ifndef DOCUMENTGENERATORDIALOG_H
#define DOCUMENTGENERATORDIALOG_H

#include <QDialog>
#include <QMessageBox>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QDateEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QFileDialog>
#include <QTextStream>
#include <QFile>

#include "../systemWarehouse.h"

class DocumentGeneratorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DocumentGeneratorDialog(systemWarehouse* system, QWidget *parent = nullptr); // Zmieniono QSqlDatabase& na systemWarehouse*
    ~DocumentGeneratorDialog();

private slots:
    void onGenerateDocumentClicked();

private:
    systemWarehouse* m_system;

    QLabel *documentTypeLabel;
    QComboBox *documentTypeComboBox;
    QLabel *documentDateLabel;
    QDateEdit *documentDateEdit;
    QLabel *kontrahentLabel;
    QLineEdit *kontrahentLineEdit;
    QPushButton *generateButton;

    // Metody do generowania konkretnych typów dokumentów
    bool generatePZDocument();
    bool generateWZDocument();
    bool generatePWDocument();
    bool generateRWDocument();
    bool generateMMDocument();
};

#endif // DOCUMENTGENERATORDIALOG_H
