TEMPLATE = app
TARGET = qtecg_app

QT = core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

SOURCES += \
    GLEcgCanvas.cpp \
    MainWindow.cpp \
    Main.cpp

HEADERS += \
    GLEcgCanvas.h \
    MainWindow.h
