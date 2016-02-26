#include "databasetask.h"
#include <QSqlQuery>
#include <QSqlError>

void DatabaseTask::error(const QSqlQuery &query)
{
	this->engine->failure(query.lastError().text());
}

uint DatabaseTask::symbolMax(QSqlDatabase &newDB)
{
	QSqlQuery query(newDB);
	query.prepare(QStringLiteral("SELECT Code FROM Symbols ORDER BY Code DESC LIMIT 0, 1"));
	if(query.exec()) {
		if(query.first())
			return query.value(0).toUInt();
		else
			return 0;
	} else {
		this->error(query);
		return -1;
	}
}

bool DatabaseTask::run()
{
	QSqlDatabase newDB = QSqlDatabase::database(UpdateEngineCore::newDB);
	if(!newDB.isValid() || !newDB.isOpen()) {
		this->engine->failure(UpdateEngineCore::tr("Update-Database is not open!"));
		return false;
	}

	if(!newDB.transaction()) {
		this->engine->failure(newDB.lastError().text());
		return false;
	}

	if(this->execute(newDB)) {
		if(newDB.commit())
			return true;
		else {
			this->engine->failure(newDB.lastError().text());
			return false;
		}
	} else {
		newDB.rollback();
		return false;
	}
}

void DatabaseTransferTask::error(const QSqlQuery &query)
{
	this->engine->failure(query.lastError().text());
}

uint DatabaseTransferTask::symbolMax(QSqlDatabase &newDB)
{
	QSqlQuery query(newDB);
	query.prepare(QStringLiteral("SELECT Code FROM Symbols ORDER BY Code DESC LIMIT 0, 1"));
	if(query.exec()) {
		if(query.first())
			return query.value(0).toUInt();
		else
			return 0;
	} else {
		this->error(query);
		return -1;
	}
}

bool DatabaseTransferTask::run()
{
	QSqlDatabase newDB = QSqlDatabase::database(UpdateEngineCore::newDB);
	if(!newDB.isValid() || !newDB.isOpen()) {
		this->engine->failure(UpdateEngineCore::tr("Update-Database is not open!"));
		return false;
	}

	if(!newDB.transaction()) {
		this->engine->failure(newDB.lastError().text());
		return false;
	}

	QSqlDatabase oldDB = QSqlDatabase::database(UpdateEngineCore::oldDB);
	if(oldDB.isValid()) {
		if(!oldDB.open()) {
			newDB.rollback();
			this->engine->failure(oldDB.lastError().text());
			return false;
		}
	} else {
		newDB.rollback();
		this->engine->logError(UpdateEngineCore::tr("Old Database does not exists! "
													"Transfer task \"%1\" cannot be executed")
							   .arg(this->installText()));
		return true;
	}

	if(this->execute(newDB, oldDB)) {
		if(oldDB.isOpen())
			oldDB.close();
		if(newDB.commit())
			return true;
		else {
			this->engine->failure(newDB.lastError().text());
			return false;
		}
	} else {
		if(oldDB.isOpen())
			oldDB.close();
		newDB.rollback();
		return false;
	}
}
