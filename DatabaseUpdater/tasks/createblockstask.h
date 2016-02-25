#ifndef CREATEBLOCKSTASK_H
#define CREATEBLOCKSTASK_H

#include "databasetask.h"

class CreateBlocksTask : public DatabaseTask
{
public:
	// DatabaseTask interface
	QUrl downloadUrl() const Q_DECL_OVERRIDE;
	QString installText() const Q_DECL_OVERRIDE;
	bool execute(QSqlDatabase &newDB) Q_DECL_OVERRIDE;
};

#endif // CREATEBLOCKSTASK_H
