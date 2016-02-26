#include "databaseloader.h"
#include <QtSql>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QStandardItemModel>
#include "unicodermodels.h"
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
	else
		this->mainDB.exec(QStringLiteral("PRAGMA foreign_keys = ON"));
}

DatabaseLoader::~DatabaseLoader()
{
	this->mainDB.close();
	this->mainDB = QSqlDatabase();
	QSqlDatabase::removeDatabase(DatabaseLoader::DBName);
}

void DatabaseLoader::reset()
{
	QDir sDir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
	sDir.mkpath(QStringLiteral("."));
	QString file = sDir.absoluteFilePath(QStringLiteral("unicode.db"));
	QFile::remove(file);
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

QStringList DatabaseLoader::symbolAliases(uint code) const
{
	QSqlQuery query(this->mainDB);
	query.prepare(QStringLiteral("SELECT NameAlias FROM Aliases WHERE Code = :code"));
	query.bindValue(QStringLiteral(":code"), code);
	if(query.exec()) {
		QStringList aliases;
		while(query.next())
			aliases += query.value(0).toString();
		return aliases;
	} else
		return QStringList();
}

QSqlQueryModel *DatabaseLoader::createSearchModel(QObject *modelParent, bool aliases) const
{
	QSqlQueryModel *model = new QSqlQueryModel(modelParent);
	this->clearSearchModel(model, aliases);
	return model;
}

bool DatabaseLoader::searchName(const QString &nameTerm, SearchFlags mode, bool aliases, QSqlQueryModel *model) const
{
	QSqlQuery query(this->mainDB);
	if(aliases)
		query.prepare(QStringLiteral("SELECT Code, Code, NameAlias FROM Aliases WHERE NameAlias LIKE :term"));
	else
		query.prepare(QStringLiteral("SELECT Code, Code, Name FROM Symbols WHERE Name LIKE :term"));
	query.bindValue(QStringLiteral(":term"),
					DatabaseLoader::prepareSearch(nameTerm, mode));
	if(query.exec()){
		model->setQuery(query);
		model->setHeaderData(0, Qt::Horizontal, tr("Preview"));
		model->setHeaderData(1, Qt::Horizontal, tr("Code"));
		model->setHeaderData(2, Qt::Horizontal, aliases ? tr("Name/Alias") : tr("Name"));
		return true;
	} else
		return false;
}

void DatabaseLoader::clearSearchModel(QSqlQueryModel *model, bool aliases) const
{
	model->setQuery(QStringLiteral("SELECT Code, Code, Name FROM Symbols WHERE 1 = 0 LIMIT 0, 1"), this->mainDB);
	model->setHeaderData(0, Qt::Horizontal, tr("Preview"));
	model->setHeaderData(1, Qt::Horizontal, tr("Code"));
	model->setHeaderData(2, Qt::Horizontal, aliases ? tr("Name/Alias") : tr("Name"));
}

SymbolListModel *DatabaseLoader::createBlock(int blockID, QObject *modelParent) const
{
	SymbolListModel *updateModel = new SymbolListModel(modelParent);
	if(this->createBlock(blockID, updateModel))
		return updateModel;
	else {
		delete updateModel;
		return Q_NULLPTR;
	}
}

bool DatabaseLoader::createBlock(int blockID, SymbolListModel *updateModel) const
{
	if(blockID == 0) {
		QSqlQuery query(this->mainDB);
		query.prepare(QStringLiteral("SELECT Recent.Code, Symbols.Name FROM Recent INNER JOIN Symbols ON Recent.Code = Symbols.Code ORDER BY Recent.SymCount DESC LIMIT 0, :limit"));
		query.bindValue(QStringLiteral(":limit"), SETTINGS_VALUE(SettingsDialog::maxRecent).toInt());
		if(query.exec()) {
			updateModel->setQuery(query);
			return true;
		} else
			return false;
	} else {
		//get the blocks borders
		Range range = this->blockRange(blockID);
		if(DatabaseLoader::isRangeValid(range)) {
			QSqlQuery query(this->mainDB);
			query.prepare(QStringLiteral("SELECT Code, Name FROM Symbols WHERE Code >= :start AND Code <= :end"));
			query.bindValue(QStringLiteral(":start"), range.first);
			query.bindValue(QStringLiteral(":end"), range.second);
			if(query.exec()) {
				updateModel->setQuery(query);
				return true;
			}
		}

		return false;
	}
}

int DatabaseLoader::findBlock(uint code) const
{
	QSqlQuery query(this->mainDB);
	query.prepare(QStringLiteral("SELECT ID FROM Blocks WHERE BlockStart <= :code AND BlockEnd >= :code"));
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
	query.prepare(QStringLiteral("SELECT BlockStart, BlockEnd FROM Blocks WHERE ID = :id"));
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

QSqlQueryModel *DatabaseLoader::createBlockModel(QObject *modelParent) const
{
	QSqlQueryModel *model = new QSqlQueryModel(modelParent);
	model->setQuery(QStringLiteral("SELECT Name, BlockStart, BlockEnd, ID FROM Blocks"), this->mainDB);
	return model;
}

void DatabaseLoader::updateRecent(uint code)
{
	QSqlQuery insertQuery(this->mainDB);
	insertQuery.prepare(QStringLiteral("INSERT OR IGNORE INTO Recent (Code) VALUES(:code)"));
	insertQuery.bindValue(QStringLiteral(":code"), code);
	if(insertQuery.exec()) {
		QSqlQuery updateQuery(this->mainDB);
		updateQuery.prepare(QStringLiteral("UPDATE Recent SET SymCount = SymCount + 1 WHERE Code = :code"));
		updateQuery.bindValue(QStringLiteral(":code"), code);
		if(!updateQuery.exec())
			qDebug() << "Failed to update recent entry";
	} else
		qDebug() << "Failed to insert/ignore recent entry";
}

void DatabaseLoader::removeRecent(uint code)
{
	QSqlQuery query(this->mainDB);
	query.prepare(QStringLiteral("DELETE FROM Recent WHERE Code = :code"));
	query.bindValue(QStringLiteral(":code"), code);
	query.exec();
}

void DatabaseLoader::resetRecent()
{
	QSqlQuery query(this->mainDB);
	query.prepare(QStringLiteral("DELETE FROM Recent"));
	query.exec();
}

QList<DatabaseLoader::EmojiGroupInfo> DatabaseLoader::listEmojiGroups() const
{
	QSqlQuery query(this->mainDB);
	query.prepare(QStringLiteral("SELECT ID, Name FROM EmojiGroups ORDER BY SortHint ASC"));
	if(query.exec()) {
		QList<EmojiGroupInfo> list;
		while(query.next())
			list += {query.value(0).toInt(), query.value(1).toString()};
		return list;
	} else
		return QList<EmojiGroupInfo>();
}

SymbolListModel *DatabaseLoader::createEmojiGroupModel(int groupID, QObject *modelParent) const
{
	SymbolListModel *updateModel = new SymbolListModel(modelParent, true);
	if(this->createEmojiGroupModel(groupID, updateModel))
		return updateModel;
	else {
		delete updateModel;
		return Q_NULLPTR;
	}
}

bool DatabaseLoader::createEmojiGroupModel(int groupID, SymbolListModel *updateModel) const
{
	QSqlQuery query(this->mainDB);
	query.prepare(QStringLiteral("SELECT EmojiMapping.EmojiID AS ID, Symbols.Name AS Name FROM (EmojiGroups INNER JOIN EmojiMapping ON EmojiGroups.ID = EmojiMapping.GroupID) AS TTbl INNER JOIN Symbols ON TTbl.EmojiID = Symbols.Code WHERE EmojiGroups.ID = :groupID ORDER BY EmojiMapping.SortHint ASC"));
	query.bindValue(QStringLiteral(":groupID"), groupID);
	if(query.exec()) {
		updateModel->setQuery(query);
		return true;
	} else
		return false;
}

bool DatabaseLoader::addEmoji(int groupID, uint code)
{
	QSqlQuery query(this->mainDB);
	query.prepare(QStringLiteral("INSERT INTO EmojiMapping (GroupID, EmojiID, SortHint) VALUES(:group, :emoji, (SELECT IFNULL(MAX(SortHint), 0) + 1 FROM EmojiMapping WHERE GroupID = :group))"));
	query.bindValue(QStringLiteral(":group"), groupID);
	query.bindValue(QStringLiteral(":emoji"), code);
	return query.exec();
}

bool DatabaseLoader::removeEmoji(int groupID, uint code)
{
	QSqlQuery query(this->mainDB);
	query.prepare(QStringLiteral("DELETE FROM EmojiMapping WHERE GroupID = :group AND EmojiID = :emoji"));
	query.bindValue(QStringLiteral(":group"), groupID);
	query.bindValue(QStringLiteral(":emoji"), code);
	return query.exec();
}

bool DatabaseLoader::moveEmoji(int groupID, uint code, uint before)
{
	QSqlQuery listQuery(this->mainDB);
	listQuery.prepare(QStringLiteral("SELECT EmojiMapping.EmojiID, EmojiMapping.SortHint FROM EmojiGroups INNER JOIN EmojiMapping ON EmojiGroups.ID = EmojiMapping.GroupID WHERE EmojiGroups.ID = :groupID ORDER BY EmojiMapping.SortHint ASC"));
	listQuery.bindValue(QStringLiteral(":groupID"), groupID);
	if(!listQuery.exec())
		return false;

	QList<uint> codeList;
	int readState = 0;
	int startHint = -1;
	while(listQuery.next() && readState < 2) {
		uint listCode = listQuery.value(0).toUInt();
		if(listCode == code || listCode == before) {
			if(readState == 0)
				startHint = listQuery.value(1).toInt();
			++readState;
		}
		if(readState > 0)
			codeList.append(listCode);
	}

	if(startHint == -1 || !codeList.removeOne(code))
		return false;
	codeList.insert(codeList.indexOf(before), code);

	if(!this->mainDB.transaction())
		return false;
	for(int i = 0, max = codeList.size(); i < max; ++i) {
		QSqlQuery insertQuery(this->mainDB);
		insertQuery.prepare(QStringLiteral("UPDATE EmojiMapping SET SortHint = :hint WHERE GroupID = :group AND EmojiID = :emoji"));
		insertQuery.bindValue(QStringLiteral(":group"), groupID);
		insertQuery.bindValue(QStringLiteral(":emoji"), codeList[i]);
		insertQuery.bindValue(QStringLiteral(":hint"), i + startHint);
		if(!insertQuery.exec()) {
			qDebug() << insertQuery.lastError().text();
			this->mainDB.rollback();
			return false;
		}
	}

	return this->mainDB.commit();
}

int DatabaseLoader::createEmojiGroup(const QString &name)
{
	QSqlQuery query(this->mainDB);
	query.prepare(QStringLiteral("INSERT INTO EmojiGroups (Name) VALUES(:name)"));
	query.bindValue(QStringLiteral(":name"), name);
	if(query.exec())
		return query.lastInsertId().toInt();
	else
		return -1;
}

bool DatabaseLoader::deleteEmojiGroup(int groupID)
{
	if(!this->mainDB.transaction())
		return false;
	QSqlQuery removeMappingQuery(this->mainDB);
	removeMappingQuery.prepare(QStringLiteral("DELETE FROM EmojiMapping WHERE GroupID = :group"));
	removeMappingQuery.bindValue(QStringLiteral(":group"), groupID);
	if(!removeMappingQuery.exec()) {
		this->mainDB.rollback();
		return false;
	}

	QSqlQuery removeGroupQuery(this->mainDB);
	removeGroupQuery.prepare(QStringLiteral("DELETE FROM EmojiGroups WHERE ID = :group"));
	removeGroupQuery.bindValue(QStringLiteral(":group"), groupID);
	if(!removeGroupQuery.exec()) {
		this->mainDB.rollback();
		return false;
	} else
		return this->mainDB.commit();
}

void DatabaseLoader::updateEmojiGroupOrder(const QList<int> &idOrder)
{
	if(!this->mainDB.transaction())
		return;
	for(int i = 0; i < idOrder.size(); i++) {
		QSqlQuery query(this->mainDB);
		query.prepare(QStringLiteral("UPDATE EmojiGroups SET SortHint = :index WHERE ID = :id"));
		query.bindValue(QStringLiteral(":id"), idOrder[i]);
		query.bindValue(QStringLiteral(":index"), i);
		if(!query.exec()) {
			this->mainDB.rollback();
			return;
		}
	}
	this->mainDB.commit();
}

QString DatabaseLoader::prepareSearch(QString term, SearchFlags flags)
{
	if(!flags.testFlag(StartsWith))
		term.prepend(QLatin1Char('%'));
	if(!flags.testFlag(EndsWith))
		term.append(QLatin1Char('%'));
	return term;
}
