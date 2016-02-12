#include "databaseloader.h"
#include <QtSql>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QStandardItemModel>
#include "settingsdialog.h"

const QString DatabaseLoader::DBName = QStringLiteral("UnicodeDB");
const DatabaseLoader::Range DatabaseLoader::InvalidRange(UINT_MAX, 0);

bool DatabaseLoader::isRangeValid(const DatabaseLoader::Range &range)
{
	return (range.first <= range.second);
}

DatabaseLoader::DatabaseLoader(QObject *parent) :
	QObject(parent),
	mainDB()
{
	//load the database
	QDir sDir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
	sDir.mkpath(QStringLiteral("."));
	QString file = sDir.absoluteFilePath(QStringLiteral("unicode.db"));
	if(!QFile::exists(file)) {
		QFile::copy(QStringLiteral(":/data/mainDB.sqlite"), file);
		QFile::setPermissions(file, QFileDevice::ReadUser | QFileDevice::WriteUser);
	}

	this->mainDB = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"),
											 DatabaseLoader::DBName);
	this->mainDB.setDatabaseName(file);
	if(!this->mainDB.isValid() || !this->mainDB.open())
		qDebug() << "Failed to open DB";
}

DatabaseLoader::~DatabaseLoader()
{
	this->mainDB.close();
	this->mainDB = QSqlDatabase();
	QSqlDatabase::removeDatabase(DatabaseLoader::DBName);
}

QString DatabaseLoader::nameForSymbol(uint code) const
{
	QSqlQuery query(this->mainDB);
	query.prepare(QStringLiteral("SELECT Name FROM Symbols WHERE Code = :code"));
	query.bindValue(QStringLiteral(":code"), code);
	if(query.exec() && query.first())
		return query.value(0).toString();
	else
		return QString();
}

DatabaseLoader::SymbolInfoList DatabaseLoader::searchName(const QString &nameTerm, SearchFlags mode) const
{
	QSqlQuery query = this->searchNameQuery(nameTerm, mode);
	if(query.isActive()) {
		SymbolInfoList result;
		while(query.next())
			result += {query.value(0).toUInt(), query.value(1).toString()};
		return result;
	} else
		return SymbolInfoList();
}

QSqlQuery DatabaseLoader::searchNameQuery(const QString &nameTerm, SearchFlags mode) const
{
	QSqlQuery query(this->mainDB);
	query.prepare(QStringLiteral("SELECT Code, Name FROM Symbols WHERE Name Like :term"));
	query.bindValue(QStringLiteral(":term"),
					DatabaseLoader::prepareSearch(nameTerm, mode));
	if(query.exec())
		return query;
	else
		return QSqlQuery();
}

QSqlQuery DatabaseLoader::emptySearchQuery() const
{
	QSqlQuery query(this->mainDB);
	query.prepare(QStringLiteral("SELECT Code, Name FROM Symbols WHERE 1 = 0"));
	if(query.exec())
		return query;
	else
		return QSqlQuery();
}

DatabaseLoader::SymbolInfoList DatabaseLoader::createBlock(int blockID) const
{
	if(blockID == 0) {
		QSqlQuery query(this->mainDB);
		query.prepare(QStringLiteral("SELECT Recent.Code, Symbols.Name FROM Recent INNER JOIN Symbols ON Recent.Code = Symbols.Code ORDER BY Count DESC LIMIT 0, :limit"));
		query.bindValue(QStringLiteral(":limit"), SETTINGS_VALUE(SettingsDialog::maxRecent).toInt());
		if(query.exec()) {
			SymbolInfoList symbolList;
			while(query.next())
				symbolList += {query.value(0).toUInt(), query.value(1).toString()};
			return symbolList;
		} else
			return SymbolInfoList();
	} else {
		//get the blocks borders
		Range range = this->blockRange(blockID);
		if(DatabaseLoader::isRangeValid(range)) {
			QSqlQuery query(this->mainDB);
			query.prepare(QStringLiteral("SELECT Code, Name FROM Symbols WHERE Code >= :start AND Code <= :end"));
			query.bindValue(QStringLiteral(":start"), range.first);
			query.bindValue(QStringLiteral(":end"), range.second);
			if(query.exec()) {
				SymbolInfoList symbolList;
				while(query.next())
					symbolList += {query.value(0).toUInt(), query.value(1).toString()};
				return symbolList;
			}
		}

		return SymbolInfoList();
	}
}

int DatabaseLoader::findBlock(uint code) const
{
	QSqlQuery query(this->mainDB);
	query.prepare(QStringLiteral("SELECT ID FROM Blocks WHERE Start <= :code AND End >= :code"));
	query.bindValue(QStringLiteral(":code"), code);
	if(query.exec() && query.first())
		return query.value(0).toInt();
	else
		return -1;
}

QString DatabaseLoader::blockName(int blockID) const
{
	QSqlQuery query(this->mainDB);
	query.prepare(QStringLiteral("SELECT Name FROM Blocks WHERE ID = :id"));
	query.bindValue(QStringLiteral(":id"), blockID);
	if(query.exec() && query.first())
		return query.value(0).toString();
	else
		return QString();
}

DatabaseLoader::Range DatabaseLoader::blockRange(int blockID) const
{
	QSqlQuery query(this->mainDB);
	query.prepare(QStringLiteral("SELECT Start, End FROM Blocks WHERE ID = :id"));
	query.bindValue(QStringLiteral(":id"), blockID);
	if(query.exec() && query.first()) {
		bool ok1 = false, ok2 = false;
		Range range = {query.value(0).toUInt(&ok1), query.value(1).toUInt(&ok2)};
		if(ok1 && ok2)
			return range;
		else
			return DatabaseLoader::InvalidRange;
	} else
		return DatabaseLoader::InvalidRange;
}

QString DatabaseLoader::findBlockName(uint code) const
{
	int id = this->findBlock(code);
	if (id != -1)
		return this->blockName(id);
	else
		return QString();
}

QAbstractItemModel *DatabaseLoader::createBlockModel(QObject *modelParent) const
{
	QSqlQueryModel *model = new QSqlQueryModel(modelParent);
	model->setQuery(QStringLiteral("SELECT Name, Start, End, ID FROM Blocks"), this->mainDB);
	return model;
}

void DatabaseLoader::updateRecent(uint code) const
{
	QSqlQuery insertQuery(this->mainDB);
	insertQuery.prepare(QStringLiteral("INSERT OR IGNORE INTO Recent (Code) VALUES(:code)"));
	insertQuery.bindValue(QStringLiteral(":code"), code);
	if(insertQuery.exec()) {
		QSqlQuery updateQuery(this->mainDB);
		updateQuery.prepare(QStringLiteral("UPDATE Recent SET Count = Count + 1 WHERE Code = :code"));
		updateQuery.bindValue(QStringLiteral(":code"), code);
		if(!updateQuery.exec())
			qDebug() << "Failed to update recent entry";
	} else
		qDebug() << "Failed to insert/ignore recent entry";
}

QMap<int, QString> DatabaseLoader::listEmojiGroups() const
{
	QSqlQuery query(this->mainDB);
	query.prepare(QStringLiteral("SELECT ID, Name FROM EmojiGroups"));
	if(query.exec()) {
		QMap<int, QString> map;
		while(query.next())
			map.insert(query.value(0).toInt(), query.value(1).toString());
		return map;
	} else
		return QMap<int, QString>();
}

DatabaseLoader::SymbolInfoList DatabaseLoader::createEmojiGroup(int groupID) const
{
	QSqlQuery query(this->mainDB);
	query.prepare(QStringLiteral("SELECT EmojiMapping.EmojiID AS ID, Symbols.Name AS Name FROM (EmojiGroups INNER JOIN EmojiMapping ON EmojiGroups.ID = EmojiMapping.GroupID) AS TTbl INNER JOIN Symbols ON TTbl.EmojiID = Symbols.Code WHERE EmojiGroups.ID = :groupID"));
	query.bindValue(QStringLiteral(":groupID"), groupID);
	if(query.exec()) {
		SymbolInfoList emojiList;
		while(query.next())
			emojiList += {query.value(0).toUInt(), query.value(1).toString()};
		return emojiList;
	} else
		return SymbolInfoList();
}

QString DatabaseLoader::prepareSearch(QString term, SearchFlags flags)
{
	if(!flags.testFlag(StartsWith))
		term.prepend(QLatin1Char('%'));
	if(!flags.testFlag(EndsWith))
		term.append(QLatin1Char('%'));
	return term;
}
