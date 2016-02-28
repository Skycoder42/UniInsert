#-------------------------------------------------
#
# Project created by QtCreator 2016-02-01T17:49:54
#
#-------------------------------------------------

QT	   += core gui sql
CONFIG += C++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = UnicodeUtil
VERSION = 1.1.0
TEMPLATE = app

translate.target = translate
translate.commands += lupdate.exe -locations relative -recursive $$_PRO_FILE_ $$escape_expand(\n\t)
translate.commands += lrelease.exe -compress -nounfinished $$_PRO_FILE_
QMAKE_EXTRA_TARGETS += translate

DEFINES += "TARGET=\\\"$$TARGET\\\""
DEFINES += "VERSION=\\\"$$VERSION\\\""
DEFINES += QT_USE_STRINGBUILDER

win32 {
	RC_ICONS += ./icons/logo_black.ico
	QMAKE_TARGET_COMPANY = "Skycoder42"
	QMAKE_TARGET_PRODUCT = "Unicode Utility"
	QMAKE_TARGET_DESCRIPTION = $$QMAKE_TARGET_PRODUCT

	DEFINES += "COMPANY=\"\\\"$$QMAKE_TARGET_COMPANY\\\"\""
	DEFINES += "DISPLAY_NAME=\"\\\"$$QMAKE_TARGET_PRODUCT\\\"\""

	LIBS += -lUser32
}

include(C:/C++Libraries/Qt/QHotkey/qhotkey.pri)
include(C:/C++Libraries/Qt/QSingleInstance/qsingleinstance.pri)
include(C:/C++Libraries/Qt/DialogMaster/dialogmaster.pri)

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
	unicodermodels.cpp \
    resetdatabasedialog.cpp

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
	unicodermodels.h \
    resetdatabasedialog.h

FORMS	+= symbolselectdialog.ui \
	getcodedialog.ui \
	blockselectdialog.ui \
	settingsdialog.ui \
	advancedsearchdialog.ui \
	emojidialog.ui \
    resetdatabasedialog.ui

RESOURCES += \
	unicodeutil_res.qrc
