// inventorydocumentgenerator.h
#ifndef INVENTORYDOCUMENTGENERATOR_H
#define INVENTORYDOCUMENTGENERATOR_H

#include <QObject>
#include <QList>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDir>
#include <QMessageBox> // Do komunikatów o błędach/sukcesie

// Struktura do przechowywania danych produktu dla dokumentu inwentaryzacji
struct InventoryProductData {
    int id;
    QString name;
    int quantityInSystem;
    int realQuantity; // Rzeczywista ilość wpisana przez użytkownika
};

class InventoryDocumentGenerator : public QObject
{
    Q_OBJECT
public:
    explicit InventoryDocumentGenerator(QObject *parent = nullptr);

    // Metoda do generowania dokumentu inwentaryzacji
    bool generateDocument(const QList<InventoryProductData>& productsData);
};

#endif // INVENTORYDOCUMENTGENERATOR_H
