// pzdocumentgenerator.cpp
#include "pzdocumentgenerator.h"
#include <QDebug>
#include <QSqlDatabase>

// Poprawiona definicja konstruktora
PZDocumentGenerator::PZDocumentGenerator(QSqlDatabase &db, QObject *parent) :
    QObject(parent),
    m_db(db)
{
}

bool PZDocumentGenerator::generateDocument(const QList<PZProductData>& productsData)
{
    if (productsData.isEmpty()) {
        qDebug() << "PZDocumentGenerator: Brak danych produktów do wygenerowania dokumentu PZ.";
        return false;
    }

    QString currentTimestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString fileName = QDateTime::currentDateTime().toString("'PZ-'hhmmss-yyyyMMdd'.txt'");
    QFile file(fileName);

    QString productsSummary;
    for (const PZProductData &data : productsData) {
        productsSummary += QString("%1x %2; ").arg(data.quantityReceived).arg(data.name);
    }
    if (productsSummary.endsWith("; ")) {
        productsSummary.chop(2);
    }

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);

        out << "--- DOKUMENT PRZYJĘCIA ZEWNĘTRZNEGO (PZ) ---\n";
        out << "Data wygenerowania: " << currentTimestamp << "\n";
        out << "------------------------------------------\n\n";
        out << QString("%1 %2 %3 %4 %5 %6\n")
                   .arg("ID Prod.", -10)
                   .arg("Nazwa Prod.", -25)
                   .arg("Regał", -8)
                   .arg("Półka", -8)
                   .arg("Kolumna", -8)
                   .arg("Ilość Przyjęta", -15);
        out << "------------------------------------------\n";

        for (const PZProductData &data : productsData) {
            out << QString("%1 %2 %3 %4 %5 %6\n")
            .arg(QString::number(data.productId), -10)
                .arg(data.name, -25)
                .arg(QString::number(data.regal), -8)
                .arg(QString::number(data.polka), -8)
                .arg(QString::number(data.kolumna), -8)
                .arg(QString::number(data.quantityReceived), -15);
        }
        out << "\n------------------------------------------\n";
        out << "Dokument wygenerowany automatycznie." << "\n";
        file.close();

        QSqlQuery logQuery(m_db);
        logQuery.prepare("INSERT INTO document_log (document_type, file_name, generation_timestamp, associated_products_summary) "
                         "VALUES (:type, :file_name, :timestamp, :summary)");
        logQuery.bindValue(":type", "PZ");
        logQuery.bindValue(":file_name", fileName);
        logQuery.bindValue(":timestamp", currentTimestamp);
        logQuery.bindValue(":summary", productsSummary);

        if (!logQuery.exec()) {
            qDebug() << "PZDocumentGenerator: Błąd zapisu do logu dokumentów: " << logQuery.lastError().text();
        } else {
            qDebug() << "PZDocumentGenerator: Pomyślnie zalogowano dokument PZ do bazy danych.";
        }

        return true;
    } else {
        qDebug() << "PZDocumentGenerator: Nie można zapisać pliku PZ: " << file.errorString();
        return false;
    }
}
