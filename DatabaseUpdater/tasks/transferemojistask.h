#ifndef TRANSFEREMOJISTASK_H
#define TRANSFEREMOJISTASK_H

#include "databasetask.h"

class TransferEmojisTask : public DatabaseTransferTask
{
public:
	// DatabaseTransferTask interface
	QString installText() const Q_DECL_OVERRIDE;
	bool execute(QSqlDatabase &newDB, QSqlDatabase &oldDB) Q_DECL_OVERRIDE;
};

#endif // TRANSFEREMOJISTASK_H
