#include "transferemojistask.h"
#include <QHash>

QString TransferEmojisTask::installText() const
{
	return UpdateEngineCore::tr("Transfering emojis from old database");
}

bool TransferEmojisTask::execute(QSqlDatabase &newDB, QSqlDatabase &oldDB)
{
	//count
	int max = 0;
	QSqlQuery countGroupsQuery(oldDB);
	countGroupsQuery.prepare(QStringLiteral("SELECT COUNT(*) FROM EmojiGroups"));
	TRY_EXEC(countGroupsQuery)
	if(countGroupsQuery.first())
		max += countGroupsQuery.value(0).toInt();
	QSqlQuery countMappingsQuery(oldDB);
	countMappingsQuery.prepare(QStringLiteral("SELECT COUNT(*) FROM EmojiMapping"));
	TRY_EXEC(countMappingsQuery)
	if(countMappingsQuery.first())
		max += countMappingsQuery.value(0).toInt();

	this->engine->updateInstallMax(max);

	QSqlQuery loadGroupsQuery(oldDB);
	loadGroupsQuery.prepare(QStringLiteral("SELECT ID, Name, SortHint FROM EmojiGroups"));
	TRY_EXEC(loadGroupsQuery)

	int prog = 0;
	QHash<int, int> groupIDMapping;
	while(loadGroupsQuery.next()) {
		CHECK_ABORT
		QSqlQuery transferQuery(newDB);
		transferQuery.prepare(QStringLiteral("INSERT INTO EmojiGroups (Name, SortHint) VALUES(:name, :hint)"));
		transferQuery.bindValue(QStringLiteral(":name"), loadGroupsQuery.value(1));
		transferQuery.bindValue(QStringLiteral(":hint"), loadGroupsQuery.value(2));
		TRY_EXEC(transferQuery)
		groupIDMapping.insert(loadGroupsQuery.value(0).toInt(),
							  transferQuery.lastInsertId().toInt());

		this->engine->updateInstallValue(++prog);
	}

	QSqlQuery mappingQuery(oldDB);
	mappingQuery.prepare(QStringLiteral("SELECT GroupID, EmojiID, SortHint FROM EmojiMapping"));
	TRY_EXEC(mappingQuery)

	while(mappingQuery.next()) {
		CHECK_ABORT
		QSqlQuery transferQuery(newDB);
		uint code = mappingQuery.value(1).toUInt();
		transferQuery.prepare(QStringLiteral("INSERT INTO EmojiMapping (GroupID, EmojiID, SortHint) VALUES(:group, :emoji, :hint)"));
		transferQuery.bindValue(QStringLiteral(":group"), groupIDMapping.value(mappingQuery.value(0).toInt()));
		transferQuery.bindValue(QStringLiteral(":emoji"), code);
		transferQuery.bindValue(QStringLiteral(":hint"), mappingQuery.value(2));
		if(!transferQuery.exec()) {
			QString emojiCode = QStringLiteral("U+%1").arg(code, 4, 16, QChar('0'));
			this->engine->logError(UpdateEngineCore::tr("Emoji Symbol %1 could not be transfered!")
								   .arg(emojiCode.toUpper()));
		}

		this->engine->updateInstallValue(++prog);
	}

	return true;
}
