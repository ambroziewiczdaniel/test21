#ifndef DOCUMENTGENERATORDIALOG_H
#define DOCUMENTGENERATORDIALOG_H

#include <QDialog>
#include <QSqlDatabase> // Będzie zmienione na systemWarehouse*
#include <QMessageBox>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QDateEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QFileDialog> // Do wyboru miejsca zapisu pliku
#include <QTextStream> // Do zapisu do pliku
#include <QFile> // Do operacji na plikach

// Forward declaration, jeśli DocumentGenerator jest osobną klasą
class DocumentGenerator; // <--- DODANO

#include "../systemWarehouse.h" // NOWY INCLUDE

class DocumentGeneratorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DocumentGeneratorDialog(systemWarehouse* system, QWidget *parent = nullptr); // Zmieniono QSqlDatabase& na systemWarehouse*
    ~DocumentGeneratorDialog();

private slots:
    void onGenerateDocumentClicked();

private:
    systemWarehouse* m_system; // NOWA ZMIENNA: wskaźnik do systemu
    // QSqlDatabase &m_db; // USUNIĘTO!

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

    // Dodatkowo, jeśli chcesz mieć ogólną klasę do generowania dokumentów
    // zamiast logiki w dialogu, możesz stworzyć DocumentGeneratorService
    // i przekazać do niego m_system.
};

#endif // DOCUMENTGENERATORDIALOG_H