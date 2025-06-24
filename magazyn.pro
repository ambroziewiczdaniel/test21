QT       += core gui sql widgets

# Jeśli używasz Qt 6 lub nowszego, dodaj też modules:
# QT += core gui sql widgets

TARGET = magazyn
CONFIG   += console c++17

# Lista wszystkich plików .cpp
SOURCES += \
    GUI/adminview.cpp \
    GUI/dispatchquantitydelegate.cpp \
    main.cpp \
    systemWarehouse.cpp \
    Product.cpp \
    ProductDao.cpp \
    LocationDao.cpp \
    Order.cpp \
    OrderItem.cpp \
    OrderDao.cpp \
    OrderItemDao.cpp \
    GUI/loginwindow.cpp \
    GUI/mainwindow.cpp \
    GUI/warehouseview.cpp \
    GUI/inventoryview.cpp \
    GUI/zamowieniaview.cpp \
    GUI/przyjecieview.cpp \
    GUI/ksiegowoscview.cpp \
    GUI/orderdetailsdialog.cpp \
    GUI/dispatchorderdialog.cpp \
    GUI/documentgeneratordialog.cpp \
    GUI/inventorydocumentgenerator.cpp \
    GUI/pzdocumentgenerator.cpp \
    UserClass.cpp \
    UserAdmin.cpp \
    UserAccountant.cpp \
    UserWarehouseKeeper.cpp

# Lista wszystkich plików .h
HEADERS += \
    .h \
    GUI/adminview.h \
    GUI/dispatchquantitydelegate.h \
    IStockTaking.h \
    iinventorymanagement.h \
    systemWarehouse.h \
    Product.h \
    ProductDao.h \
    LocationDao.h \
    Order.h \
    OrderItem.h \
    OrderDao.h \
    OrderItemDao.h \
    GUI/loginwindow.h \
    GUI/mainwindow.h \
    GUI/warehouseview.h \
    GUI/inventoryview.h \
    GUI/zamowieniaview.h \
    GUI/przyjecieview.h \
    GUI/ksiegowoscview.h \
    GUI/orderdetailsdialog.h \
    GUI/dispatchorderdialog.h \
    GUI/documentgeneratordialog.h \
    GUI/inventorydocumentgenerator.h \
    GUI/pzdocumentgenerator.h \
    IAuthorizeUser.h \
    UserClass.h \
    UserAdmin.h \
    UserAccountant.h \
    UserWarehouseKeeper.h