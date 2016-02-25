#ifndef DATABASETASK_H
#define DATABASETASK_H

#include "updatetask.h"
#include <QSqlDatabase>

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

#endif // DATABASETASK_H
