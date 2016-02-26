#ifndef PARSEEMOJIGROUPTASK_H
#define PARSEEMOJIGROUPTASK_H

#include "databasetask.h"

class ParseEmojiGroupTask : public DatabaseTask
{
public:
	ParseEmojiGroupTask(const QUrl &downloadUrl);

	// DatabaseTask interface
	QUrl downloadUrl() const Q_DECL_OVERRIDE;
	QString installText() const Q_DECL_OVERRIDE;
	bool execute(QSqlDatabase &newDB) Q_DECL_OVERRIDE;

private:
	const QUrl url;
};

#endif // PARSEEMOJIGROUPTASK_H
