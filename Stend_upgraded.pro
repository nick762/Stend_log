#-------------------------------------------------
#
# Project created by QtCreator 2016-10-31T17:22:18
#
#-------------------------------------------------

QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = bat
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    qcustomplot.cpp \
    json.cpp \
    port.cpp

HEADERS  += mainwindow.h \
    qcustomplot.h \
    json.h \
    port.h

FORMS    += \
    material_batt.ui

RESOURCES += \
    res.qrc
RC_FILE += batt.rc
QMAKE_CXXFLAGS += -std=c++0x
