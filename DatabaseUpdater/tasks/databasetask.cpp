#include "databasetask.h"
#include <QSqlQuery>
#include <QSqlError>

void DatabaseTask::error(const QSqlQuery &query)
{
	this->engine->failure(query.lastError().text());
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
	if(!oldDB.open() || !oldDB.isValid()) {
		newDB.rollback();
		this->engine->failure(oldDB.lastError().text());
		return false;
	}

	if(this->execute(newDB, oldDB)) {
		oldDB.close();
		if(newDB.commit())
			return true;
		else {
			this->engine->failure(newDB.lastError().text());
			return false;
		}
	} else {
		oldDB.close();
		newDB.rollback();
		return false;
	}
}
