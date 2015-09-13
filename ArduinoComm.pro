#-------------------------------------------------
#
# Project created by QtCreator 2015-09-13T12:09:07
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ArduinoComm
TEMPLATE = app


SOURCES += \
    io.cpp \
    main.cpp \
    mainwindow.cpp \
    text.cpp

HEADERS  += \
    commands.h \
    io.h \
    mainwindow.h \
    text.h

FORMS += \
    mainwindow.ui

QMAKE_LFLAGS += /INCREMENTAL:NO
