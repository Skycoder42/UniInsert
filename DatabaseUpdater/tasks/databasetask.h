#ifndef DATABASETASK_H
#define DATABASETASK_H

#include "updatetask.h"
#include <QSqlDatabase>
#include <QSqlQuery>

class DatabaseTask : public UpdateTask
{
public:
	virtual bool execute(QSqlDatabase &newDB) = 0;

	void error(const QSqlQuery &query);

	// UpdateTask interface
	bool run() Q_DECL_OVERRIDE;
};

class DatabaseTransferTask : public UpdateTask
{
public:
	virtual bool execute(QSqlDatabase &newDB, QSqlDatabase &oldDB) = 0;

	void error(const QSqlQuery &query);

	// UpdateTask interface
	bool run() Q_DECL_OVERRIDE;
};

#define TRY_EXEC(query)  \
	if(!query.exec()) {\
		this->error(query);\
		return false;\
	}

#endif // DATABASETASK_H
