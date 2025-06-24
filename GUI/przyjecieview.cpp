#include "przyjecieview.h"
#include <QDebug>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>

PrzyjecieView::PrzyjecieView(systemWarehouse* system, QWidget *parent) :
    QWidget(parent),
    m_system(system), // Inicjalizujemy wskaźnik do systemu
    searchProductLabel(nullptr),
    productSearchLineEdit(nullptr),
    productCompleter(nullptr),
    productCompleterModel(nullptr),
    currentProductIdLabel(nullptr),
    currentProductQuantityLabel(nullptr),
    regalLabel(nullptr),
    regalSpinBox(nullptr),
    polkaLabel(nullptr),
    polkaSpinBox(nullptr),
    kolumnaLabel(nullptr),
    kolumnaSpinBox(nullptr),
    iloscPrzyjetaLabel(nullptr),
    iloscPrzyjetaSpinBox(nullptr),
    addProductToReceiptButton(nullptr),
    receiptItemsLabel(nullptr),
    receiptItemsTableView(nullptr),
    receiptItemsModel(nullptr),
    generatePZButton(nullptr)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QHBoxLayout *productSearchLayout = new QHBoxLayout();
    searchProductLabel = new QLabel("Wyszukaj produkt (nazwa):", this);
    productSearchLineEdit = new QLineEdit(this);
    productSearchLineEdit->setPlaceholderText("Wpisz nazwę produktu, aby go rozlokować");

    productSearchLayout->addWidget(searchProductLabel);
    productSearchLayout->addWidget(productSearchLineEdit);
    mainLayout->addLayout(productSearchLayout);

    productCompleterModel = new QSqlQueryModel(this);
    productCompleterModel->setQuery("SELECT name FROM products", m_system->getDb()); // Używamy m_system->getDb()
    if (productCompleterModel->lastError().isValid()) {
        QMessageBox::warning(this, "Błąd Bazy Danych",
                             "Nie można załadować nazw produktów do autouzupełniania (Przyjęcie): " + productCompleterModel->lastError().text());
        qDebug() << "Błąd QCompleter SQL (Przyjęcie):" << productCompleterModel->lastError().text();
    }
    productCompleter = new QCompleter(productCompleterModel, this);
    productCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    productCompleter->setCompletionColumn(0);
    productSearchLineEdit->setCompleter(productCompleter);

    connect(productCompleter, QOverload<const QString &>::of(&QCompleter::activated),
            this, &PrzyjecieView::onProductSelected);
    connect(productSearchLineEdit, &QLineEdit::textChanged, this, &PrzyjecieView::onSearchTextChanged);


    currentProductIdLabel = new QLabel("ID Produktu: ", this);
    currentProductQuantityLabel = new QLabel("Minimalna dopuszczalna ilość: ", this);
    mainLayout->addWidget(currentProductIdLabel);
    mainLayout->addWidget(currentProductQuantityLabel);

    mainLayout->addWidget(new QLabel("Lokalizacja i ilość przyjęcia:", this));
    QHBoxLayout *locationInputLayout = new QHBoxLayout();
    regalLabel = new QLabel("Regał:", this);
    regalSpinBox = new QSpinBox(this);
    regalSpinBox->setRange(0, 999);
    polkaLabel = new QLabel("Półka:", this);
    polkaSpinBox = new QSpinBox(this);
    polkaSpinBox->setRange(0, 999);
    kolumnaLabel = new QLabel("Kolumna:", this);
    kolumnaSpinBox = new QSpinBox(this);
    kolumnaSpinBox->setRange(0, 999);
    iloscPrzyjetaLabel = new QLabel("Ilość przyjęta:", this);
    iloscPrzyjetaSpinBox = new QSpinBox(this);
    iloscPrzyjetaSpinBox->setRange(1, 99999);
    iloscPrzyjetaSpinBox->setValue(1);

    locationInputLayout->addWidget(regalLabel);
    locationInputLayout->addWidget(regalSpinBox);
    locationInputLayout->addWidget(polkaLabel);
    locationInputLayout->addWidget(polkaSpinBox);
    locationInputLayout->addWidget(kolumnaLabel);
    locationInputLayout->addWidget(kolumnaSpinBox);
    locationInputLayout->addWidget(iloscPrzyjetaLabel);
    locationInputLayout->addWidget(iloscPrzyjetaSpinBox);
    mainLayout->addLayout(locationInputLayout);

    addProductToReceiptButton = new QPushButton("Dodaj produkt do listy przyjęcia", this);
    connect(addProductToReceiptButton, &QPushButton::clicked, this, &PrzyjecieView::onAddProductToReceiptClicked);
    mainLayout->addWidget(addProductToReceiptButton);

    receiptItemsLabel = new QLabel("Produkty do wygenerowania PZ:", this);
    receiptItemsTableView = new QTableView(this);
    receiptItemsModel = new QStandardItemModel(0, 6, this);
    receiptItemsModel->setHeaderData(0, Qt::Horizontal, "Nazwa Produktu");
    receiptItemsModel->setHeaderData(1, Qt::Horizontal, "ID Produktu");
    receiptItemsModel->setHeaderData(2, Qt::Horizontal, "Regał");
    receiptItemsModel->setHeaderData(3, Qt::Horizontal, "Półka");
    receiptItemsModel->setHeaderData(4, Qt::Horizontal, "Kolumna");
    receiptItemsModel->setHeaderData(5, Qt::Horizontal, "Ilość Przyjęta");
    receiptItemsTableView->setModel(receiptItemsModel);
    receiptItemsTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    receiptItemsTableView->horizontalHeader()->setStretchLastSection(true);
    receiptItemsTableView->verticalHeader()->setVisible(false);
    mainLayout->addWidget(receiptItemsLabel);
    mainLayout->addWidget(receiptItemsTableView);

    generatePZButton = new QPushButton("Generuj Dokument PZ", this);
    connect(generatePZButton, &QPushButton::clicked, this, &PrzyjecieView::onGeneratePZClicked);
    mainLayout->addWidget(generatePZButton);

    mainLayout->addStretch();
}

PrzyjecieView::~PrzyjecieView()
{
    // Obiekty będące dziećmi PrzyjecieView zostaną automatycznie usunięte.
}

void PrzyjecieView::onSearchTextChanged(const QString &text)
{
    Q_UNUSED(text);
}

void PrzyjecieView::onProductSelected(const QString &text)
{
    int productId = m_system->getProductDao()->getProductIdByName(text);
    if (productId != -1) {
        loadProductDetailsAndLocations(productId);
    } else {
        currentProductIdLabel->setText("ID Produktu: Nie znaleziono");
        currentProductQuantityLabel->setText("Minimalna dopuszczalna ilość: N/A");
        qDebug() << "PrzyjecieView: Produkt '" << text << "' nie znaleziono w bazie danych lub błąd SQL (poprzez DAO).";
    }
}

void PrzyjecieView::loadProductDetailsAndLocations(int productId) {
    Product* product = m_system->getProductDao()->getProductById(productId);
    if (product) {
        currentProductIdLabel->setText("ID Produktu: " + QString::number(product->get_id()));
        QString productInfo = "Minimalna dopuszczalna ilość: " + QString::number(product->get_quantity());
        delete product;

        QVector<WarehouseProductData> warehouseData = m_system->getLocationDao()->getWarehouseProducts();
        QString locationsSummary = "Lokalizacje:<br>";
        bool foundLocations = false;
        for (const auto& data : warehouseData) {
            if (data.id == productId) {
                if (!data.locationsSummary.isEmpty()) {
                    locationsSummary += data.locationsSummary;
                    locationsSummary.replace("; ", "<br>");
                    locationsSummary += "<br>";
                }
                foundLocations = true;
                break;
            }
        }
        if (!foundLocations || locationsSummary == "Lokalizacje:<br>") {
            locationsSummary = "Lokalizacje: Brak w magazynie.";
        }

        currentProductQuantityLabel->setText(productInfo + "<br>" + locationsSummary);
    } else {
        currentProductIdLabel->setText("ID Produktu: Błąd");
        currentProductQuantityLabel->setText("Minimalna dopuszczalna ilość: Błąd / Brak");
        qDebug() << "PrzyjecieView: Nie można załadować szczegółów produktu ID" << productId;
    }
}

void PrzyjecieView::onAddProductToReceiptClicked()
{
    QString productName = productSearchLineEdit->text().trimmed();
    int regal = regalSpinBox->value();
    int polka = polkaSpinBox->value();
    int kolumna = kolumnaSpinBox->value();
    int iloscPrzyjeta = iloscPrzyjetaSpinBox->value();

    if (productName.isEmpty()) {
        QMessageBox::warning(this, "Błąd", "Proszę wybrać lub wpisać nazwę produktu.");
        return;
    }
    if (iloscPrzyjeta <= 0) {
        QMessageBox::warning(this, "Błąd", "Ilość przyjęta musi być większa od zera.");
        return;
    }

    int productId = m_system->getProductDao()->getProductIdByName(productName);
    if (productId == -1) {
        QMessageBox::warning(this, "Błąd", "Produkt '" + productName + "' nie istnieje w bazie danych.");
        return;
    }

    for (int i = 0; i < receiptItemsModel->rowCount(); ++i) {
        int existingProductId = receiptItemsModel->item(i, 1)->text().toInt();
        int existingRegal = receiptItemsModel->item(i, 2)->text().toInt();
        int existingPolka = receiptItemsModel->item(i, 3)->text().toInt();
        int existingKolumna = receiptItemsModel->item(i, 4)->text().toInt();

        if (existingProductId == productId &&
            existingRegal == regal &&
            existingPolka == polka &&
            existingKolumna == kolumna) {
            QMessageBox::warning(this, "Duplikat", "Ten produkt w tej lokalizacji jest już na liście przyjęcia. Edytuj istniejącą pozycję lub wybierz inną lokalizację.");
            return;
        }
    }

    QList<QStandardItem*> rowItems;
    rowItems << new QStandardItem(productName);
    rowItems << new QStandardItem(QString::number(productId));
    rowItems << new QStandardItem(QString::number(regal));
    rowItems << new QStandardItem(QString::number(polka));
    rowItems << new QStandardItem(QString::number(kolumna));
    rowItems << new QStandardItem(QString::number(iloscPrzyjeta));
    receiptItemsModel->appendRow(rowItems);

    QMessageBox::information(this, "Dodano do listy", "Produkt '" + productName + "' dodano do listy przyjęcia.");

    productSearchLineEdit->clear();
    currentProductIdLabel->setText("ID Produktu: ");
    currentProductQuantityLabel->setText("Minimalna dopuszczalna ilość: ");
    regalSpinBox->setValue(0);
    polkaSpinBox->setValue(0);
    kolumnaSpinBox->setValue(0);
    iloscPrzyjetaSpinBox->setValue(1);

    productCompleterModel->setQuery("SELECT name FROM products", m_system->getDb()); // Używamy m_system->getDb()
}

void PrzyjecieView::onGeneratePZClicked()
{
    if (receiptItemsModel->rowCount() == 0) {
        QMessageBox::warning(this, "Błąd", "Brak produktów do wygenerowania dokumentu PZ. Dodaj produkty do listy.");
        return;
    }

    QList<PZProductData> productsToGenerate;
    bool allUpdatesSuccessful = true;

    if (!m_system->getDb().transaction()) { // Używamy m_system->getDb()
        QMessageBox::critical(this, "Błąd Bazy Danych", "Nie można rozpocząć transakcji: " + m_system->getDb().lastError().text());
        qDebug() << "PrzyjecieView: Błąd rozpoczęcia transakcji:" << m_system->getDb().lastError().text();
        return;
    }

    for (int i = 0; i < receiptItemsModel->rowCount(); ++i) {
        QString productName = receiptItemsModel->item(i, 0)->text();
        int productId = receiptItemsModel->item(i, 1)->text().toInt();
        int regal = receiptItemsModel->item(i, 2)->text().toInt();
        int polka = receiptItemsModel->item(i, 3)->text().toInt();
        int kolumna = receiptItemsModel->item(i, 4)->text().toInt();
        int iloscPrzyjeta = receiptItemsModel->item(i, 5)->text().toInt();

        // Używamy LocationDao do aktualizacji ilości w lokalizacji
        if (!m_system->getLocationDao()->updateProductLocationQuantity(productId, regal, polka, kolumna,
                                                                       m_system->getLocationDao()->getProductQuantityInLocation(productId, regal, polka, kolumna) + iloscPrzyjeta))
        {
            QMessageBox::critical(this, "Błąd Magazynu", "Nie można zaktualizować/dodać ilości dla " + productName + " w lokalizacji: " + m_system->getDb().lastError().text()); // Error from db, because DAO uses db directly and may not propagate specific error messages.
            allUpdatesSuccessful = false;
            break;
        }

        PZProductData data;
        data.name = productName;
        data.productId = productId;
        data.regal = regal;
        data.polka = polka;
        data.kolumna = kolumna;
        data.quantityReceived = iloscPrzyjeta;
        productsToGenerate.append(data);
    }

    if (allUpdatesSuccessful) {
        m_system->getDb().commit(); // Używamy m_system->getDb()

        // PZDocumentGenerator może przyjmować systemWarehouse* zamiast QSqlDatabase&
        PZDocumentGenerator generator(m_system->getDb()); // Przekazujemy m_system->getDb() dla generatora
        if (generator.generateDocument(productsToGenerate)) {
            QMessageBox::information(this, "Sukces", "Zaktualizowano stany magazynowe i wygenerowano plik PZ.");
            receiptItemsModel->clear();
            receiptItemsModel->setHeaderData(0, Qt::Horizontal, "Nazwa Produktu");
            receiptItemsModel->setHeaderData(1, Qt::Horizontal, "ID Produktu");
            receiptItemsModel->setHeaderData(2, Qt::Horizontal, "Regał");
            receiptItemsModel->setHeaderData(3, Qt::Horizontal, "Półka");
            receiptItemsModel->setHeaderData(4, Qt::Horizontal, "Kolumna");
            receiptItemsModel->setHeaderData(5, Qt::Horizontal, "Ilość Przyjęta");
            emit databaseUpdated();
        } else {
            QMessageBox::critical(this, "Błąd Generowania", "Błąd podczas generowania pliku PZ lub rejestracji w bazie danych. Zmiany w magazynie mogły zostać zapisane.");
        }
    } else {
        m_system->getDb().rollback(); // Używamy m_system->getDb()
        QMessageBox::critical(this, "Błąd Transakcji", "Błąd podczas aktualizacji stanów magazynowych. Operacja przyjęcia została przerwana.");
    }
}
