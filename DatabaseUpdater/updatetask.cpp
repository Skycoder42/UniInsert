#include "updatetask.h"

UpdateTask::UpdateTask() :
	engine(Q_NULLPTR),
	downloadData()
{}

UpdateTask::~UpdateTask()
{}

QUrl UpdateTask::downloadUrl() const
{
	return QUrl();
}

QString UpdateTask::downloadText() const
{
	return UpdateEngineCore::tr("Downloading: %1").arg(this->downloadUrl().toString());
}

QList<UpdateTask *> UpdateTask::newTasks() const
{
	return QList<UpdateTask*>();
}
