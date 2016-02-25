#ifndef CREATESYMBOLSTASK_H
#define CREATESYMBOLSTASK_H

#include "databasetask.h"

class CreateSymbolsTask : public DatabaseTask
{
public:
	// DatabaseTask interface
	QUrl downloadUrl() const Q_DECL_OVERRIDE;
	QString installText() const Q_DECL_OVERRIDE;
	bool execute(QSqlDatabase &newDB) Q_DECL_OVERRIDE;

private:
	void findName(const QStringList &entry, QString &name, QStringList &aliases);
};

#endif // CREATESYMBOLSTASK_H
