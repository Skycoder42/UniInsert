#include "createsymbolstask.h"
#include "global.h"

QUrl CreateSymbolsTask::downloadUrl() const
{
	return QUrl(QStringLiteral("ftp://ftp.unicode.org/Public/%1/ucd/UnicodeData.txt")
				.arg(ARG_UPDATE_VERSION));
}

QString CreateSymbolsTask::installText() const
{
	return UpdateEngineCore::tr("Creating unicode symbols");
}

bool CreateSymbolsTask::execute(QSqlDatabase &newDB)
{
	bool ok = false;

	uint symbolMax = 0;
	UpdateEngineCore::UniMatrix codeMatrix = this->engine->readDownload(this->downloadData, QLatin1Char(';'), 2);
	if(!codeMatrix.isEmpty())
		symbolMax = codeMatrix.last().first().toUInt(&ok, 16);
	if(!ok) {
		this->engine->failure(UpdateEngineCore::tr("Invalid UnicodeData.txt file! (Download corrupted?)"));
		return false;
	}

	this->engine->updateInstallMax(symbolMax);

	uint counter = 0;
	for(QStringList line : codeMatrix) {
		CHECK_ABORT
		uint code = line.first().toUInt(&ok, 16);
		if(!ok) {
			this->engine->failure(UpdateEngineCore::tr("Invalid UnicodeData.txt file! (Download corrupted?)"));
			return false;
		}

		for(; counter < code; counter++) {
			CHECK_ABORT
			QSqlQuery insertUnnamedSymbolQuery(newDB);
			insertUnnamedSymbolQuery.prepare(QStringLiteral("INSERT INTO Symbols (Code) VALUES(:code)"));
			insertUnnamedSymbolQuery.bindValue(QStringLiteral(":code"), counter);
			TRY_EXEC(insertUnnamedSymbolQuery)
			this->engine->updateInstallValue(counter + 1);
		}
		if(counter != code) {
			this->engine->failure(UpdateEngineCore::tr("Invalid UnicodeData.txt file! (Download corrupted?)"));
			return false;
		}

		QString name;
		QStringList aliases;
		this->findName(line, name, aliases);
		QSqlQuery insertSymbolQuery(newDB);
		if(name.isEmpty())
			insertSymbolQuery.prepare(QStringLiteral("INSERT INTO Symbols (Code) VALUES(:code)"));
		else {
			insertSymbolQuery.prepare(QStringLiteral("INSERT INTO Symbols (Code, Name) VALUES(:code, :name)"));
			insertSymbolQuery.bindValue(QStringLiteral(":name"), name.toUpper());
		}
		insertSymbolQuery.bindValue(QStringLiteral(":code"), counter);
		TRY_EXEC(insertSymbolQuery)

		for(QString alias : aliases) {
			QSqlQuery insertAliasQuery(newDB);
			insertAliasQuery.prepare(QStringLiteral("INSERT OR IGNORE INTO Aliases (Code, NameAlias) VALUES(:code, :alias)"));
			insertAliasQuery.bindValue(QStringLiteral(":code"), counter);
			insertAliasQuery.bindValue(QStringLiteral(":alias"), alias.toUpper());
			TRY_EXEC(insertAliasQuery)
		}

		this->engine->updateInstallValue(++counter);
	}

	return true;
}

void CreateSymbolsTask::findName(const QStringList &entry, QString &name, QStringList &aliases)
{
	name = entry[1];
	bool hasAlias = (entry.size() >= 11) && !entry[10].isEmpty();

	if(name == QLatin1String("<control>"))
		aliases += name;

	if(name.startsWith(QLatin1Char('<')) &&
	   name.endsWith(QLatin1Char('>')))
		name.clear();

	if(hasAlias) {
		if(name.isEmpty())
			name = entry[10];
		else
			aliases += entry[10];
	}

	if(!name.isEmpty())
		aliases += name;
}
