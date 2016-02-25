#ifndef READNAMEINDEXTASK_H
#define READNAMEINDEXTASK_H

#include "databasetask.h"

class ReadNameIndexTask : public DatabaseTask
{
public:
	// DatabaseTask interface
	QUrl downloadUrl() const Q_DECL_OVERRIDE;
	QString installText() const Q_DECL_OVERRIDE;
	bool execute(QSqlDatabase &newDB) Q_DECL_OVERRIDE;
};

#endif // READNAMEINDEXTASK_H
