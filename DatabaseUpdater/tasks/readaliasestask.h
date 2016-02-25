#ifndef READALIASESTASK_H
#define READALIASESTASK_H

#include "databasetask.h"

class ReadAliasesTask : public DatabaseTask
{
public:
	// DatabaseTask interface
	QUrl downloadUrl() const Q_DECL_OVERRIDE;
	QString installText() const Q_DECL_OVERRIDE;
	bool execute(QSqlDatabase &newDB) Q_DECL_OVERRIDE;
};

#endif // READALIASESTASK_H
