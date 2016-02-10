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
	//DEBUG
	Q_ASSERT(QFile::remove(file));
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

QMap<uint, QString> DatabaseLoader::searchName(const QString &nameTerm, SearchFlags mode) const
{
	QSqlQuery query(this->mainDB);
	query.prepare(QStringLiteral("SELECT Code, Name FROM Symbols WHERE Name Like :term"));
	query.bindValue(QStringLiteral(":term"),
					DatabaseLoader::prepareSearch(nameTerm, mode));
	if(query.exec()) {
		QMap<uint, QString> result;
		while(query.next())
			result.insert(query.value(0).toUInt(), query.value(1).toString());
		return result;
	} else
		return QMap<uint, QString>();
}

QStringList DatabaseLoader::createBlock(int blockID) const
{
	if(blockID == 0) {
		QSqlQuery query(this->mainDB);
		query.prepare(QStringLiteral("SELECT Code FROM Recent ORDER BY Count DESC LIMIT 0, :limit"));
		query.bindValue(QStringLiteral(":limit"), SETTINGS_VALUE(SettingsDialog::maxRecent).toInt());
		if(query.exec()) {
			QStringList list;
			while(query.next())
				list += Unicoder::code32ToSymbol(query.value(0).toUInt());
			return list;
		} else
			return QStringList();
	} else {
		//get the blocks borders
		Range range = this->blockRange(blockID);
		if(DatabaseLoader::isRangeValid(range)) {
			QStringList list;
			for(uint code = range.first; code <= range.second; ++code)
				list += Unicoder::code32ToSymbol(code);
			return list;
		} else
			return QStringList();
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
	if(blockID == 0)
		return tr("Recently Used");
	else {
		QSqlQuery query(this->mainDB);
		query.prepare(QStringLiteral("SELECT Name FROM Blocks WHERE ID = :id"));
		query.bindValue(QStringLiteral(":id"), blockID);
		if(query.exec() && query.first())
			return query.value(0).toString();
		else
			return QString();
	}
}

DatabaseLoader::Range DatabaseLoader::blockRange(int blockID) const
{
	QSqlQuery query(this->mainDB);
	query.prepare(QStringLiteral("SELECT Start, End FROM Blocks WHERE ID = :id"));
	query.bindValue(QStringLiteral(":id"), blockID);
	if(query.exec() && query.first())
		return {query.value(0).toUInt(), query.value(1).toUInt()};
	else
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
	QSqlQuery query(this->mainDB);
	query.prepare(QStringLiteral("SELECT ID, Name, Start, End FROM Blocks"));
	if(!query.exec())
		return nullptr;

	QStandardItemModel *model = new QStandardItemModel(0, 3, modelParent);
	model->setHorizontalHeaderLabels({
										 tr("Block Name"),
										 tr("First Symbol"),
										 tr("Last Symbol")
									 });

	//recently used
	QStandardItem *recentItem = new QStandardItem(tr("Recently Used"));
	recentItem->setData(0, BlockModelDataRole);
	model->appendRow({
						 recentItem,
						 new QStandardItem(tr("-/-")),
						 new QStandardItem(tr("-/-"))
					 });

	while(query.next()) {
		QStandardItem *displayItem = new QStandardItem(query.value(1).toString());
		displayItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		displayItem->setData(query.value(0).toInt(), BlockModelDataRole);

		QStandardItem *beginItem = new QStandardItem(QStringLiteral("U+") + query.value(2).toString());
		beginItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		beginItem->setData(query.value(2).toUInt(), BlockModelDataRole);

		QStandardItem *endItem = new QStandardItem(QStringLiteral("U+") + query.value(3).toString());
		endItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		endItem->setData(query.value(3).toUInt(), BlockModelDataRole);

		model->appendRow({
							 displayItem,
							 beginItem,
							 endItem
						 });
	}

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

QStringList DatabaseLoader::createEmojiGroup(int groupID) const
{
	QSqlQuery query(this->mainDB);
	query.prepare(QStringLiteral("SELECT EmojiMapping.EmojiID FROM EmojiGroups INNER JOIN EmojiMapping ON EmojiGroups.ID = EmojiMapping.GroupID WHERE EmojiGroups.ID = :groupID"));
	query.bindValue(QStringLiteral(":groupID"), groupID);
	if(query.exec()) {
		QStringList emojiList;
		while(query.next())
			emojiList += Unicoder::code32ToSymbol(query.value(0).toUInt());
		return emojiList;
	} else
		return QStringList();
}

QString DatabaseLoader::prepareSearch(QString term, SearchFlags flags)
{
	if(!flags.testFlag(StartsWith))
		term.prepend(QLatin1Char('%'));
	if(!flags.testFlag(EndsWith))
		term.append(QLatin1Char('%'));
	return term;
}
