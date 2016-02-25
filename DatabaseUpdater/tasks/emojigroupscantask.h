#ifndef EMOJIGROUPSCANTASK_H
#define EMOJIGROUPSCANTASK_H

#include "updatetask.h"

class EmojiGroupScanTask : public UpdateTask
{
public:
	EmojiGroupScanTask();

	// UpdateTask interface
	QUrl downloadUrl() const Q_DECL_OVERRIDE;
	QString installText() const Q_DECL_OVERRIDE;
	bool run() Q_DECL_OVERRIDE;
	QList<UpdateTask *> newTasks() const Q_DECL_OVERRIDE;

private:
	QList<QUrl> downloadUrls;
};

#endif // EMOJIGROUPSCANTASK_H
