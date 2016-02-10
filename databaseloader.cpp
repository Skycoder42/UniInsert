#include "databaseloader.h"
#include <QSql>
#include <QFile>
#include <QDir>
#include <QStandardPaths>

const QString DatabaseLoader::dbName = QStringLiteral("UnicodeDB");

DatabaseLoader::DatabaseLoader(QObject *parent) :
	QObject(parent),
	mainDB()
{
	//load the database
	QDir sDir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
	sDir.mkpath(QStringLiteral("."));
	QString file = sDir.absoluteFilePath(QStringLiteral("unicode.db"));
	if(!QFile::exists(file))
		QFile::copy(QStringLiteral(":/data/mainDB.sqlite"), file);

	this->mainDB = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"),
											 DatabaseLoader::dbName);
	this->mainDB.setDatabaseName(file);
	if(!this->mainDB.isValid() || !this->mainDB.open()) {
		qDebug() << "Failed to open DB";
	} else {

	}
}

DatabaseLoader::~DatabaseLoader()
{
	this->mainDB.close();
	this->mainDB = QSqlDatabase();
	QSqlDatabase::removeDatabase(DatabaseLoader::dbName);
}

QString DatabaseLoader::nameForSymbol(uint code)
{

}
