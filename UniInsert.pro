#-------------------------------------------------
#
# Project created by QtCreator 2016-02-01T17:49:54
#
#-------------------------------------------------

QT       += core gui sql
CONFIG += C++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = UniInsert
TEMPLATE = app

LIBS += -lUser32

include(C:/C++Libraries/Qt/QHotkey/qhotkey.pri)

SOURCES += main.cpp\
        symbolselectdialog.cpp \
    getcodedialog.cpp \
    unicoder.cpp \
    blockselectdialog.cpp \
    popupdialog.cpp \
    settingsdialog.cpp \
    symbolinserter.cpp \
    advancedsearchdialog.cpp \
    emojidialog.cpp \
    databaseloader.cpp \
    unicodermodels.cpp

HEADERS  += symbolselectdialog.h \
    getcodedialog.h \
    unicoder.h \
    blockselectdialog.h \
    popupdialog.h \
    settingsdialog.h \
    symbolinserter.h \
    advancedsearchdialog.h \
    emojidialog.h \
    databaseloader.h \
    unicodermodels.h

FORMS    += symbolselectdialog.ui \
    getcodedialog.ui \
    blockselectdialog.ui \
    settingsdialog.ui \
    advancedsearchdialog.ui \
    emojidialog.ui

RESOURCES += \
    uniinsert_res.qrc

DISTFILES += \
    LICENSE \
    README.md
