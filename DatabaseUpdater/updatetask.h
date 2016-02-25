#ifndef UPDATETASK_H
#define UPDATETASK_H

#include <QUrl>
#include "updateenginecore.h"

class UpdateTask
{
	friend class UpdateEngine;

public:
	UpdateTask();
	virtual ~UpdateTask();

	virtual QUrl downloadUrl() const;
	virtual QString downloadText() const;

	virtual QString installText() const = 0;
	virtual bool run() = 0;

	virtual QList<UpdateTask*> newTasks() const;

protected:
	UpdateEngineCore * engine;
	QByteArray downloadData;
};

#endif // UPDATETASK_H
