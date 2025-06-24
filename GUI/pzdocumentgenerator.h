// pzdocumentgenerator.h
#ifndef PZDOCUMENTGENERATOR_H
#define PZDOCUMENTGENERATOR_H

#include <QObject>
#include <QList>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDir>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

struct PZProductData {
    QString name;
    int productId;
    int regal;
    int polka;
    int kolumna;
    int quantityReceived;
};

class PZDocumentGenerator : public QObject
{
    Q_OBJECT
public:
    // WAŻNE: Upewnij się, że ten konstruktor jest poprawny
    explicit PZDocumentGenerator(QSqlDatabase &db, QObject *parent = nullptr);

    bool generateDocument(const QList<PZProductData>& productsData);

private:
    QSqlDatabase &m_db;
};

#endif // PZDOCUMENTGENERATOR_H
