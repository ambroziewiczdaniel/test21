// inventorydocumentgenerator.cpp
#include "inventorydocumentgenerator.h"

InventoryDocumentGenerator::InventoryDocumentGenerator(QObject *parent) : QObject(parent)
{
}

bool InventoryDocumentGenerator::generateDocument(const QList<InventoryProductData>& productsData)
{
    if (productsData.isEmpty()) {
        // Ta walidacja jest też w InventoryView, ale dobrze mieć ją tu na wszelki wypadek
        return false;
    }

    QString fileName = QDateTime::currentDateTime().toString("'INW-'hhmmss-yyyyMMdd'.txt'");
    QFile file(fileName);

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << "--- DOKUMENT INWENTARYZACJI ---\n";
        out << "Data wygenerowania: " << QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss") << "\n";
        out << "------------------------------------------\n\n";
        out << QString("%1 %2 %3 %4\n")
                   .arg("ID Produktu", -15)
                   .arg("Nazwa Produktu", -30)
                   .arg("Ilość w systemie", -20)
                   .arg("Prawdziwa ilość");
        out << "------------------------------------------\n";

        for (const InventoryProductData &data : productsData) {
            out << QString("%1 %2 %3 %4\n")
            .arg(QString::number(data.id), -15)
                .arg(data.name, -30)
                .arg(QString::number(data.quantityInSystem), -20)
                .arg(QString::number(data.realQuantity));
        }
        out << "\n------------------------------------------\n";
        out << "Dokument wygenerowany automatycznie." << "\n";
        file.close();
        return true;
    } else {
        // Komunikat o błędzie zapisywania może być wyświetlony przez klasę wywołującą (InventoryView),
        // ale można go też tu zalogować lub zwrócić bardziej szczegółowy błąd.
        qDebug() << "Błąd zapisu pliku inwentaryzacji: " << file.errorString();
        return false;
    }
}
