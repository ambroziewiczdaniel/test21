QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = warehouse_app
CONFIG   += console
CONFIG   -= app_bundle

SOURCES += \
    adminview.cpp \
    dispatchorderdialog.cpp \
    inventorydocumentgenerator.cpp \
    ksiegowoscview.cpp \
    loginwindow.cpp \
    main.cpp \
    mainwindow.cpp \
    documentgeneratordialog.cpp \
    orderdetailsdialog.cpp \
    przyjecieview.cpp \
    pzdocumentgenerator.cpp \
    warehouseview.cpp \
    inventoryview.cpp \
    zamowieniaview.cpp

HEADERS += \
    adminview.h \
    dispatchorderdialog.h \
    inventorydocumentgenerator.h \
    ksiegowoscview.h \
    loginwindow.h \
    mainwindow.h \
    documentgeneratordialog.h \
    orderdetailsdialog.h \
    przyjecieview.h \
    pzdocumentgenerator.h \
    warehouseview.h \
    inventoryview.h \
    zamowieniaview.h

FORMS +=
