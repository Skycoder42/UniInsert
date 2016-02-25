#include "readaliasestask.h"
#include "global.h"

QUrl ReadAliasesTask::downloadUrl() const
{
	return QUrl(QStringLiteral("ftp://ftp.unicode.org/Public/%1/ucd/NameAliases.txt")
				.arg(ARG_UPDATE_VERSION));
}

QString ReadAliasesTask::installText() const
{
	return UpdateEngineCore::tr("Adding new aliases from alias list");
}

bool ReadAliasesTask::execute(QSqlDatabase &newDB)
{
	UpdateEngineCore::UniMatrix aliasMatrix = this->engine->readDownload(this->downloadData, QLatin1Char(';'), 2);

	this->engine->updateInstallMax(aliasMatrix.size());

	for(int i = 0, max = aliasMatrix.size(); i < max; ++i) {
		CHECK_ABORT
		const QStringList &list = aliasMatrix[i];
		bool ok = false;
		uint code = list[0].toUInt(&ok, 16);
		if(ok) {
			QSqlQuery insertAliasQuery(newDB);
			insertAliasQuery.prepare(QStringLiteral("INSERT OR IGNORE INTO Aliases (Code, NameAlias) VALUES(:code, :alias)"));
			insertAliasQuery.bindValue(QStringLiteral(":code"), code);
			insertAliasQuery.bindValue(QStringLiteral(":alias"), list[1].toUpper());
			TRY_EXEC(insertAliasQuery)
		}

		this->engine->updateInstallValue(i + 1);
	}

	return true;
}
