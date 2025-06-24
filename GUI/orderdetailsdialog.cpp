#include "orderdetailsdialog.h"
#include "dispatchorderdialog.h"
#include <QDebug>
#include <QVariant>
#include <QHeaderView>

OrderDetailsDialog::OrderDetailsDialog(systemWarehouse* system, int orderId, QWidget *parent) :
    QDialog(parent),
    m_system(system),
    m_orderId(orderId)
{
    setWindowTitle("Szczegóły Zamówienia ID: " + QString::number(m_orderId));
    setMinimumSize(700, 500);

    qDebug() << "OrderDetailsDialog: Otwieram dialog dla zamówienia ID:" << m_orderId;

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QHBoxLayout *infoLayout = new QHBoxLayout();
    kontrahentLabel = new QLabel("Kontrahent: ", this);
    dataLabel = new QLabel("Data: ", this);
    statusLabel = new QLabel("Status: ", this);

    infoLayout->addWidget(kontrahentLabel);
    infoLayout->addWidget(dataLabel);
    infoLayout->addWidget(statusLabel);
    mainLayout->addLayout(infoLayout);

    QHBoxLayout *statusControlLayout = new QHBoxLayout();
    QLabel *changeStatusLabel = new QLabel("Zmień status na:", this);
    statusComboBox = new QComboBox(this);
    statusComboBox->addItem("Nowe");
    statusComboBox->addItem("W realizacji");
    statusComboBox->addItem("Zrealizowane");
    statusComboBox->addItem("Anulowane");

    updateStatusButton = new QPushButton("Aktualizuj Status", this);
    connect(updateStatusButton, &QPushButton::clicked, this, &OrderDetailsDialog::onUpdateStatusButtonClicked);

    statusControlLayout->addWidget(changeStatusLabel);
    statusControlLayout->addWidget(statusComboBox);
    statusControlLayout->addWidget(updateStatusButton);
    statusControlLayout->addStretch();
    mainLayout->addLayout(statusControlLayout);

    mainLayout->addWidget(new QLabel("Produkty w zamówieniu:", this));
    productsModel = new QSqlQueryModel(this);
    productsTableView = new QTableView(this);
    productsTableView->setModel(productsModel);
    productsTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    productsTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    productsTableView->setSelectionMode(QAbstractItemView::NoSelection);
    productsTableView->horizontalHeader()->setStretchLastSection(true);
    productsTableView->verticalHeader()->setVisible(false);
    mainLayout->addWidget(productsTableView);

    loadOrderDetails();
    loadOrderProducts();
}

OrderDetailsDialog::~OrderDetailsDialog()
{

}

void OrderDetailsDialog::loadOrderDetails()
{
    Order* order = m_system->getOrderDao()->getOrderById(m_orderId);

    if (order) {
        kontrahentLabel->setText("Kontrahent: " + order->getKontrahent());
        dataLabel->setText("Data: " + order->getData().toString("yyyy-MM-dd hh:mm:ss"));
        QString currentStatus = order->getStatus();
        statusLabel->setText("Status: " + currentStatus);

        int index = statusComboBox->findText(currentStatus);
        if (index != -1) {
            statusComboBox->setCurrentIndex(index);
        }
        delete order;
        qDebug() << "OrderDetailsDialog: Szczegóły zamówienia załadowane pomyślnie (przez DAO).";
    } else {
        QMessageBox::critical(this, "Błąd", "Nie można załadować szczegółów zamówienia (przez DAO).");
        qDebug() << "OrderDetailsDialog: Błąd ładowania szczegółów zamówienia dla ID " << m_orderId << ": nie znaleziono lub błąd DAO.";
    }
}

void OrderDetailsDialog::loadOrderProducts()
{
    QSqlQuery query(m_system->getDb());
    query.prepare(
        "SELECT "
        "p.id, "
        "p.name, "
        "oi.quantity "
        "FROM order_items oi "
        "JOIN products p ON oi.product_id = p.id "
        "WHERE oi.order_id = :orderId"
        );
    query.bindValue(":orderId", m_orderId);

    if (!query.exec()) {
        QMessageBox::critical(this, "Błąd", "Nie można wykonać zapytania produktów zamówienia: " + query.lastError().text());
        qDebug() << "OrderDetailsDialog: Błąd wykonania zapytania produktów zamówienia dla ID " << m_orderId << ":" << query.lastError().text();
        return;
    }

    productsModel->setQuery(std::move(query));

    if (productsModel->lastError().isValid()) {
        QMessageBox::critical(this, "Błąd", "Błąd modelu QSqlQueryModel po załadowaniu produktów zamówienia: " + productsModel->lastError().text());
        qDebug() << "OrderDetailsDialog: Błąd modelu QSqlQueryModel po załadowaniu produktów zamówienia dla ID " << m_orderId << ":" << productsModel->lastError().text();
    } else {
        qDebug() << "OrderDetailsDialog: Załadowano " << productsModel->rowCount() << " wierszy produktów dla zamówienia ID " << m_orderId;
        if (productsModel->rowCount() == 0) {
            qDebug() << "OrderDetailsDialog: Brak pozycji produktów dla zamówienia ID " << m_orderId << ". Sprawdź dane w tabeli order_items.";
        }
    }

    productsModel->setHeaderData(0, Qt::Horizontal, "ID Produktu");
    productsModel->setHeaderData(1, Qt::Horizontal, "Nazwa Produktu");
    productsModel->setHeaderData(2, Qt::Horizontal, "Ilość w zamówieniu");
}

void OrderDetailsDialog::onUpdateStatusButtonClicked()
{
    QString newStatus = statusComboBox->currentText();
    QString currentStatus = statusLabel->text().replace("Status: ", "");

    if (newStatus == currentStatus) {
        QMessageBox::information(this, "Status", "Status zamówienia jest już ustawiony na '" + newStatus + "'.");
        return;
    }

    if (newStatus == "Zrealizowane") {
        DispatchOrderDialog dispatchDialog(m_system, m_orderId, this); // Przekazujemy m_system
        connect(&dispatchDialog, &DispatchOrderDialog::dispatchCompleted, this, &OrderDetailsDialog::onDispatchCompleted);
        connect(&dispatchDialog, &DispatchOrderDialog::dispatchCancelled, this, [this]() {
            loadOrderDetails();
        });

        dispatchDialog.exec();
    } else {
        updateOrderStatusInDb(newStatus);
    }
}

void OrderDetailsDialog::onDispatchCompleted(bool success) {
    if (success) {
        updateOrderStatusInDb("Zrealizowane");
        QMessageBox::information(this, "Wydanie Zrealizowane", "Wydanie produktów zakończone pomyślnie. Status zamówienia zmieniono na 'Zrealizowane'.");
    } else {
        QMessageBox::warning(this, "Wydanie Anulowane/Nieudane", "Operacja wydania została anulowana lub zakończyła się niepowodzeniem. Status zamówienia nie został zmieniony.");
        loadOrderDetails();
    }
}

void OrderDetailsDialog::updateOrderStatusInDb(const QString& newStatus) {
    if (m_system->getOrderDao()->updateOrderStatus(m_orderId, newStatus)) {
        statusLabel->setText("Status: " + newStatus);
        emit orderStatusChanged(m_orderId);
        qDebug() << "OrderDetailsDialog: Status zamówienia ID " << m_orderId << " zmieniono na " << newStatus << " (przez DAO).";
    } else {
        qDebug() << "OrderDetailsDialog: Błąd aktualizacji statusu zamówienia (przez DAO).";
    }
}

void OrderDetailsDialog::onStatusChanged(const QString &newStatus)
{
    Q_UNUSED(newStatus);
}
