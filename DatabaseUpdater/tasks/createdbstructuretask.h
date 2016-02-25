#ifndef CREATEDBSTRUCTURETASK_H
#define CREATEDBSTRUCTURETASK_H

#include "databasetask.h"

class CreateDBStructureTask : public DatabaseTask
{
public:
	// DatabaseTask interface
	QString installText() const Q_DECL_OVERRIDE;
	bool execute(QSqlDatabase &newDB) Q_DECL_OVERRIDE;
};

#endif // CREATEDBSTRUCTURETASK_H
