#include "transferrecenttask.h"

QString TransferRecentTask::installText() const
{
	return UpdateEngineCore::tr("Transfering recently used elements from old database");
}

bool TransferRecentTask::execute(QSqlDatabase &newDB, QSqlDatabase &oldDB)
{
	QSqlQuery countRecentQuery(oldDB);
	countRecentQuery.prepare(QStringLiteral("SELECT COUNT(*) FROM Recent"));
	TRY_EXEC(countRecentQuery)
	if(countRecentQuery.first())
		this->engine->updateInstallMax(countRecentQuery.value(0).toInt());

	QSqlQuery recentQuery(oldDB);
	recentQuery.prepare(QStringLiteral("SELECT Code, SymCount FROM Recent"));
	TRY_EXEC(recentQuery)

	int prog = 0;
	while(recentQuery.next()) {
		CHECK_ABORT
		QSqlQuery transferQuery(newDB);
		uint code = recentQuery.value(0).toUInt();
		transferQuery.prepare(QStringLiteral("INSERT INTO Recent (Code, SymCount) VALUES(:code, :num)"));
		transferQuery.bindValue(QStringLiteral(":code"), code);
		transferQuery.bindValue(QStringLiteral(":num"), recentQuery.value(1));
		if(!transferQuery.exec()) {
			QString symbolCode = QStringLiteral("U+%1").arg(code, 4, 16, QChar('0'));
			this->engine->logError(UpdateEngineCore::tr("Recently used Symbol %1 could not be transfered!")
								   .arg(symbolCode.toUpper()));
		}

		this->engine->updateInstallValue(++prog);
	}

	return true;
}
