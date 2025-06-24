#include "documentgeneratordialog.h"
#include <QDateTime>
#include <QSqlError>
#include <QSqlQuery>

DocumentGeneratorDialog::DocumentGeneratorDialog(systemWarehouse* system, QWidget *parent) :
    QDialog(parent),
    m_system(system),
    documentTypeLabel(nullptr),
    documentTypeComboBox(nullptr),
    documentDateLabel(nullptr),
    documentDateEdit(nullptr),
    kontrahentLabel(nullptr),
    kontrahentLineEdit(nullptr),
    generateButton(nullptr)
{
    setWindowTitle("Generator Dokumentów Magazynowych");
    setMinimumSize(400, 300);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // --- Typ dokumentu ---
    QHBoxLayout *typeLayout = new QHBoxLayout();
    documentTypeLabel = new QLabel("Typ dokumentu:", this);
    documentTypeComboBox = new QComboBox(this);
    documentTypeComboBox->addItem("PZ - Przyjęcie Zewnętrzne");
    documentTypeComboBox->addItem("WZ - Wydanie Zewnętrzne");
    documentTypeComboBox->addItem("PW - Przyjęcie Wewnętrzne");
    documentTypeComboBox->addItem("RW - Rozchód Wewnętrzny");
    documentTypeComboBox->addItem("MM - Przesunięcie Międzymagazynowe");
    typeLayout->addWidget(documentTypeLabel);
    typeLayout->addWidget(documentTypeComboBox);
    mainLayout->addLayout(typeLayout);

    // --- Data dokumentu ---
    QHBoxLayout *dateLayout = new QHBoxLayout();
    documentDateLabel = new QLabel("Data dokumentu:", this);
    documentDateEdit = new QDateEdit(this);
    documentDateEdit->setCalendarPopup(true);
    documentDateEdit->setDate(QDate::currentDate());
    dateLayout->addWidget(documentDateLabel);
    dateLayout->addWidget(documentDateEdit);
    mainLayout->addLayout(dateLayout);

    // --- Kontrahent (opcjonalnie, tylko dla PZ/WZ) ---
    QHBoxLayout *kontrahentLayout = new QHBoxLayout();
    kontrahentLabel = new QLabel("Kontrahent:", this);
    kontrahentLineEdit = new QLineEdit(this);
    kontrahentLineEdit->setPlaceholderText("Wpisz nazwę kontrahenta (dla PZ/WZ)");
    kontrahentLayout->addWidget(kontrahentLabel);
    kontrahentLayout->addWidget(kontrahentLineEdit);
    mainLayout->addLayout(kontrahentLayout);

    // --- Przycisk generowania ---
    generateButton = new QPushButton("Generuj Dokument", this);
    connect(generateButton, &QPushButton::clicked, this, &DocumentGeneratorDialog::onGenerateDocumentClicked);
    mainLayout->addWidget(generateButton);

    mainLayout->addStretch();
}

DocumentGeneratorDialog::~DocumentGeneratorDialog()
{
    // Obiekty są dziećmi QDialog, więc zostaną automatycznie usunięte.
}

void DocumentGeneratorDialog::onGenerateDocumentClicked()
{
    QString documentType = documentTypeComboBox->currentText();
    bool success = false;

    if (documentType == "PZ - Przyjęcie Zewnętrzne") {
        success = generatePZDocument();
    } else if (documentType == "WZ - Wydanie Zewnętrzne") {
        success = generateWZDocument();
    } else if (documentType == "PW - Przyjęcie Wewnętrzne") {
        success = generatePWDocument();
    } else if (documentType == "RW - Rozchód Wewnętrzny") {
        success = generateRWDocument();
    } else if (documentType == "MM - Przesunięcie Międzymagazynowe") {
        success = generateMMDocument();
    }

    if (success) {
        QMessageBox::information(this, "Sukces", "Dokument " + documentType + " został pomyślnie wygenerowany!");
    } else {
        QMessageBox::warning(this, "Błąd Generowania", "Nie udało się wygenerować dokumentu " + documentType + ".");
    }
}

// Przykładowe implementacje funkcji generujących dokumenty
// Te funkcje na razie nie używają DAO, ale mogłyby w przyszłości pobierać dane z bazy.
bool DocumentGeneratorDialog::generatePZDocument()
{
    QString kontrahent = kontrahentLineEdit->text().trimmed();
    if (kontrahent.isEmpty()) {
        QMessageBox::warning(this, "Błąd", "Dla dokumentu PZ, kontrahent nie może być pusty.");
        return false;
    }

    QString fileName = QFileDialog::getSaveFileName(this, "Zapisz dokument PZ",
                                                    QDir::currentPath() + "/PZ-" + QDateTime::currentDateTime().toString("yyyyMMddhhmmss") + ".txt",
                                                    "Pliki tekstowe (*.txt)");
    if (fileName.isEmpty()) {
        return false;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Błąd", "Nie można otworzyć pliku do zapisu.");
        return false;
    }

    QTextStream out(&file);
    out << "--- Dokument PZ - Przyjęcie Zewnętrzne ---\n";
    out << "Data: " << documentDateEdit->date().toString("yyyy-MM-dd") << "\n";
    out << "Kontrahent: " << kontrahent << "\n";
    out << "------------------------------------------\n";
    out << "Ten dokument PZ jest placeholderem. Pełna implementacja wymagałaby pobrania rzeczywistych danych produktów z przyjęcia.\n";
    file.close();
    return true;
}

bool DocumentGeneratorDialog::generateWZDocument()
{
    QString kontrahent = kontrahentLineEdit->text().trimmed();
    if (kontrahent.isEmpty()) {
        QMessageBox::warning(this, "Błąd", "Dla dokumentu WZ, kontrahent nie może być pusty.");
        return false;
    }

    QString fileName = QFileDialog::getSaveFileName(this, "Zapisz dokument WZ",
                                                    QDir::currentPath() + "/WZ-" + QDateTime::currentDateTime().toString("yyyyMMddhhmmss") + ".txt",
                                                    "Pliki tekstowe (*.txt)");
    if (fileName.isEmpty()) {
        return false;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Błąd", "Nie można otworzyć pliku do zapisu.");
        return false;
    }

    QTextStream out(&file);
    out << "--- Dokument WZ - Wydanie Zewnętrzne ---\n";
    out << "Data: " << documentDateEdit->date().toString("yyyy-MM-dd") << "\n";
    out << "Kontrahent: " << kontrahent << "\n";
    out << "------------------------------------------\n";
    out << "Ten dokument WZ jest placeholderem. Pełna implementacja wymagałaby pobrania rzeczywistych danych produktów z wydania.\n";
    file.close();
    return true;
}

bool DocumentGeneratorDialog::generatePWDocument()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Zapisz dokument PW",
                                                    QDir::currentPath() + "/PW-" + QDateTime::currentDateTime().toString("yyyyMMddhhmmss") + ".txt",
                                                    "Pliki tekstowe (*.txt)");
    if (fileName.isEmpty()) {
        return false;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Błąd", "Nie można otworzyć pliku do zapisu.");
        return false;
    }

    QTextStream out(&file);
    out << "--- Dokument PW - Przyjęcie Wewnętrzne ---\n";
    out << "Data: " << documentDateEdit->date().toString("yyyy-MM-dd") << "\n";
    out << "------------------------------------------\n";
    out << "Ten dokument PW jest placeholderem. Pełna implementacja wymagałaby danych z przyjęcia wewnętrznego.\n";
    file.close();
    return true;
}

bool DocumentGeneratorDialog::generateRWDocument()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Zapisz dokument RW",
                                                    QDir::currentPath() + "/RW-" + QDateTime::currentDateTime().toString("yyyyMMddhhmmss") + ".txt",
                                                    "Pliki tekstowe (*.txt)");
    if (fileName.isEmpty()) {
        return false;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Błąd", "Nie można otworzyć pliku do zapisu.");
        return false;
    }

    QTextStream out(&file);
    out << "--- Dokument RW - Rozchód Wewnętrzny ---\n";
    out << "Data: " << documentDateEdit->date().toString("yyyy-MM-dd") << "\n";
    out << "------------------------------------------\n";
    out << "Ten dokument RW jest placeholderem. Pełna implementacja wymagałaby danych z rozchodu wewnętrznego.\n";
    file.close();
    return true;
}

bool DocumentGeneratorDialog::generateMMDocument()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Zapisz dokument MM",
                                                    QDir::currentPath() + "/MM-" + QDateTime::currentDateTime().toString("yyyyMMddhhmmss") + ".txt",
                                                    "Pliki tekstowe (*.txt)");
    if (fileName.isEmpty()) {
        return false;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Błąd", "Nie można otworzyć pliku do zapisu.");
        return false;
    }

    QTextStream out(&file);
    out << "--- Dokument MM - Przesunięcie Międzymagazynowe ---\n";
    out << "Data: " << documentDateEdit->date().toString("yyyy-MM-dd") << "\n";
    out << "------------------------------------------\n";
    out << "Ten dokument MM jest placeholderem. Pełna implementacja wymagałaby danych z przesunięcia międzymagazynowego.\n";
    file.close();
    return true;
}
