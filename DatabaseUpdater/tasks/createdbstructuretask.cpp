#include "createdbstructuretask.h"
#include <QFile>
#include "global.h"

QString CreateDBStructureTask::installText() const
{
	return UpdateEngineCore::tr("Creating database structure");
}

bool CreateDBStructureTask::execute(QSqlDatabase &newDB)
{
	QFile queryFile(QStringLiteral(":/data/dbSetup.sql"));
	queryFile.open(QIODevice::ReadOnly);
	Q_ASSERT(queryFile.isOpen());
	QStringList queries = QString::fromUtf8(queryFile.readAll())
						  .split(QLatin1Char(';'), QString::SkipEmptyParts);
	queryFile.close();

	this->engine->updateInstallMax(queries.size());

	QString path = ARG_LOCAL_DB_PATH + QStringLiteral(".update");
	QFile::remove(path);

	for(int i = 0, max = queries.size() - 1; i < max; ++i) {//skip last -> empty because of ';'
		QSqlQuery setupQuery(newDB);
		if(!setupQuery.exec(queries[i])) {
			this->error(setupQuery);
			return false;
		}
		this->engine->updateInstallValue(i + 1);
	}

	QSqlQuery versionQuery(newDB);
	versionQuery.prepare(QStringLiteral("INSERT INTO Meta (UnicodeVersion) VALUES(:version)"));
	versionQuery.bindValue(QStringLiteral(":version"), ARG_UPDATE_VERSION);
	if(versionQuery.exec())
		this->engine->updateInstallValue(queries.size());
	else {
		this->error(versionQuery);
		return false;
	}

	return true;
}
