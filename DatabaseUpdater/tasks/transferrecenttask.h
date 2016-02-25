#ifndef TRANSFERRECENTTASK_H
#define TRANSFERRECENTTASK_H

#include "databasetask.h"

class TransferRecentTask : public DatabaseTransferTask
{
public:
	// DatabaseTransferTask interface
	QString installText() const Q_DECL_OVERRIDE;
	bool execute(QSqlDatabase &newDB, QSqlDatabase &oldDB) Q_DECL_OVERRIDE;
};

#endif // TRANSFERRECENTTASK_H
