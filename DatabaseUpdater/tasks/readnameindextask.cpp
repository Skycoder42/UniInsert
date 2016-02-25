#include "readnameindextask.h"
#include "global.h"

QUrl ReadNameIndexTask::downloadUrl() const
{
	return QUrl(QStringLiteral("ftp://ftp.unicode.org/Public/%1/ucd/Index.txt")
				.arg(ARG_UPDATE_VERSION));
}

QString ReadNameIndexTask::installText() const
{
	return UpdateEngineCore::tr("Adding new aliases from name index");
}

bool ReadNameIndexTask::execute(QSqlDatabase &newDB)
{
	UpdateEngineCore::UniMatrix nameMatrix = this->engine->readDownload(this->downloadData, QLatin1Char('\t'), 2);

	this->engine->updateInstallMax(nameMatrix.size());

	for(int i = 0, max = nameMatrix.size(); i < max; ++i) {
		CHECK_ABORT
		const QStringList &list = nameMatrix[i];
		bool ok = false;
		uint code = list[1].toUInt(&ok, 16);
		const QString &name = list[0];

		if(ok &&
		   (name == name.toUpper() ||
			name == name.toLower())) {
			QSqlQuery insertAliasQuery(newDB);
			insertAliasQuery.prepare(QStringLiteral("INSERT OR IGNORE INTO Aliases (Code, NameAlias) VALUES(:code, :alias)"));
			insertAliasQuery.bindValue(QStringLiteral(":code"), code);
			insertAliasQuery.bindValue(QStringLiteral(":alias"), name.toUpper());
			TRY_EXEC(insertAliasQuery)
		}

		this->engine->updateInstallValue(i + 1);
	}

	return true;
}
