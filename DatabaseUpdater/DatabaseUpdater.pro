#-------------------------------------------------
#
# Project created by QtCreator 2016-02-21T16:50:15
#
#-------------------------------------------------

QT       += core gui sql network
CONFIG += C++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DatabaseUpdater
VERSION = 1.1.0
TEMPLATE = app

DEFINES += "TARGET=\\\"$$TARGET\\\""
DEFINES += "VERSION=\\\"$$VERSION\\\""

win32 {
	RC_ICONS += ./Fatcow-Farm-Fresh-Database-refresh.ico
	QMAKE_TARGET_COMPANY = "Skycoder42"
	QMAKE_TARGET_PRODUCT = "Unicode Utility - Database Updater"
	QMAKE_TARGET_DESCRIPTION = $$QMAKE_TARGET_PRODUCT

	DEFINES += "COMPANY=\"\\\"$$QMAKE_TARGET_COMPANY\\\"\""
	DEFINES += "DISPLAY_NAME=\"\\\"$$QMAKE_TARGET_PRODUCT\\\"\""

	QT += winextras
}

include(C:/C++Libraries/Qt/QSingleInstance/qsingleinstance.pri)
include(C:/C++Libraries/Qt/QProgressGroup/qprogressgroup.pri)
include(C:/C++Libraries/Qt/DialogMaster/dialogmaster.pri)

SOURCES += main.cpp\
        updaterwindow.cpp \
    basedownloader.cpp

HEADERS  += updaterwindow.h \
    global.h \
    basedownloader.h

FORMS    += updaterwindow.ui

RESOURCES += \
    databaseupdater_res.qrc
