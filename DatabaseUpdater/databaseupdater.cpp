#include "databaseupdater.h"
#include <QThread>
#include <QFile>
#include <QSqlQuery>
#include <QSqlError>
#include <QTextStream>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include "global.h"

#define TRY_OPEN(db) \
	if(!db.open() || !db.isValid()) {\
		emit error(db.lastError().text(), true);\
		return;\
	}

#define TRY_EXEC(query)  \
	if(!query.exec()) {\
		emit error(query.lastError().text(), true);\
		return;\
	}

#define COMMIT_FINISH \
	if(this->newDB.commit())\
		emit installReady();\
	else {\
		emit error(this->newDB.lastError().text(), true);\
		return;\
	}

DatabaseUpdater::DatabaseUpdater(QObject *parent) :
	QObject(Q_NULLPTR),
	newDB(),
	abortRequested(false),
	nextFunc(ARG_UPDATE_MODE.testFlag(UpdaterWindow::Emojis) ? 0 : 1)
{
	QThread *runThread = new QThread(parent);

	connect(runThread, &QThread::finished, this, &DatabaseUpdater::deleteLater);
	connect(parent, &QObject::destroyed, runThread, [runThread](){
		runThread->quit();
		if(!runThread->wait(5000))
			runThread->terminate();
	});

	runThread->start(QThread::HighPriority);
	this->moveToThread(runThread);
}

DatabaseUpdater::~DatabaseUpdater()
{
	if(this->newDB.isOpen()) {
		this->newDB.close();
		this->newDB = QSqlDatabase();
		QSqlDatabase::removeDatabase(QStringLiteral("newDB"));
	}
}

int DatabaseUpdater::getInstallCount() const
{
	int count = 7;
	UpdaterWindow::UpdateFlags flags = ARG_UPDATE_MODE;
	if(flags.testFlag(UpdaterWindow::RecentlyUsed))
		count += 1;
	return count;
}

void DatabaseUpdater::startInstalling()
{
	QFile queryFile(QStringLiteral(":/data/dbSetup.sql"));
	queryFile.open(QIODevice::ReadOnly);
	Q_ASSERT(queryFile.isOpen());
	QStringList queries = QString::fromUtf8(queryFile.readAll())
						  .split(QLatin1Char(';'), QString::SkipEmptyParts);
	queryFile.close();
	emit beginInstall(tr("Creating database"), queries.size() + 1);

	QString path = ARG_LOCAL_DB_PATH + QStringLiteral(".update");
	QFile::remove(path);

	this->newDB = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), QStringLiteral("newDB"));
	this->newDB.setDatabaseName(path);
	TRY_OPEN(this->newDB)

	QSqlDatabase oldDB = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), QStringLiteral("oldDB"));
	oldDB.setDatabaseName(ARG_LOCAL_DB_PATH);

	this->newDB.exec(QStringLiteral("PRAGMA foreign_keys = ON"));
	this->newDB.transaction();
	for(int i = 0, max = queries.size() - 1; i < max; ++i) {//skip last -> empty because of ';'
		QSqlQuery setupQuery(this->newDB);
		if(!setupQuery.exec(queries[i])) {
			emit error(setupQuery.lastError().text(), true);
			return;
		}
		emit updateInstallProgress(i + 1);
	}

	QSqlQuery versionQuery(this->newDB);
	versionQuery.prepare(QStringLiteral("INSERT INTO Meta (UnicodeVersion) VALUES(:version)"));
	versionQuery.bindValue(QStringLiteral(":version"), ARG_UPDATE_VERSION);
	if(versionQuery.exec())
		emit updateInstallProgress(queries.size() + 1);
	else {
		emit error(versionQuery.lastError().text(), true);
		return;
	}

	COMMIT_FINISH
}

void DatabaseUpdater::abortInstalling()
{
	this->abortRequested = true;
	QMetaObject::invokeMethod(this, "doAbort", Qt::QueuedConnection);
}

void DatabaseUpdater::handleDownload(const QByteArray &downloadData)
{
	if(!this->abortRequested) {
		switch (this->nextFunc++) {
		case 0:
			break;//emoji -> do nothing
		case 1:
			this->installCodeData(downloadData);
			break;
		case 2:
			this->installBlocks(downloadData);
			break;
		case 3:
			this->installNameIndex(downloadData);
			break;
		case 4:
			this->installAliases(downloadData);
			break;
		default:
			break;
			Q_UNREACHABLE();//TODO
		}
	}
}

void DatabaseUpdater::installCodeData(const QByteArray &downloadData)
{
	bool ok = false;
	emit beginInstall(tr("Creating unicode symbols"), 0);

	UniMatrix codeMatrix = this->readDownload(downloadData, QLatin1Char(';'), 2);
	if(!codeMatrix.isEmpty())
		this->symbolMax = codeMatrix.last().first().toUInt(&ok, 16);
	if(!ok) {
		emit error(tr("Invalid UnicodeData.txt file! (Download corrupted?)"), true);
		return;
	}

	emit beginInstall(tr("Creating unicode symbols"), DatabaseUpdater::PercentMax);
	this->newDB.transaction();

	uint counter = 0;
	uint buffer = 0;
	for(QStringList line : codeMatrix) {
		if(this->abortRequested)
			return;
		uint code = line.first().toUInt(&ok, 16);
		if(!ok) {
			emit error(tr("Invalid UnicodeData.txt file! (Download corrupted?)"), true);
			return;
		}

		for(; counter < code; counter++) {
			if(this->abortRequested)
				return;
			QSqlQuery insertUnnamedSymbolQuery(this->newDB);
			insertUnnamedSymbolQuery.prepare(QStringLiteral("INSERT INTO Symbols (Code) VALUES(:code)"));
			insertUnnamedSymbolQuery.bindValue(QStringLiteral(":code"), counter);
			TRY_EXEC(insertUnnamedSymbolQuery)
			countNext(counter + 1, this->symbolMax, buffer);
		}
		if(counter != code) {
			emit error(tr("Invalid UnicodeData.txt file! (Download corrupted?)"), true);
			return;
		}

		QString name;
		QStringList aliases;
		this->findName(line, name, aliases);
		QSqlQuery insertSymbolQuery(this->newDB);
		if(name.isEmpty())
			insertSymbolQuery.prepare(QStringLiteral("INSERT INTO Symbols (Code) VALUES(:code)"));
		else {
			insertSymbolQuery.prepare(QStringLiteral("INSERT INTO Symbols (Code, Name) VALUES(:code, :name)"));
			insertSymbolQuery.bindValue(QStringLiteral(":name"), name.toUpper());
		}
		insertSymbolQuery.bindValue(QStringLiteral(":code"), counter);
		TRY_EXEC(insertSymbolQuery)

		for(QString alias : aliases) {
			QSqlQuery insertAliasQuery(this->newDB);
			insertAliasQuery.prepare(QStringLiteral("INSERT OR IGNORE INTO Aliases (Code, NameAlias) VALUES(:code, :alias)"));
			insertAliasQuery.bindValue(QStringLiteral(":code"), counter);
			insertAliasQuery.bindValue(QStringLiteral(":alias"), alias.toUpper());
			TRY_EXEC(insertAliasQuery)
		}

		countNext(++counter, this->symbolMax, buffer);
	}

	COMMIT_FINISH
}

void DatabaseUpdater::findName(const QStringList &entry, QString &name, QStringList &aliases)
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

void DatabaseUpdater::installBlocks(const QByteArray &downloadData)
{
	emit beginInstall(tr("Creating symbol blocks"), 0);

	QRegularExpression regex(QStringLiteral(R"__(^([0-9A-F]{4,})\.\.([0-9A-F]{4,});\s*(.+)$)__"));
	regex.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
	regex.optimize();
	UniMatrix blockMatrix = this->readDownload(downloadData, regex);
	uint newMax = 0;

	emit beginInstall(tr("Creating symbol blocks"), DatabaseUpdater::PercentMax);
	this->newDB.transaction();

	uint buffer = 0;
	for(uint i = 0, max = blockMatrix.size(); i < max; ++i) {
		const QStringList &list = blockMatrix[i];
		uint start = list[1].toUInt(Q_NULLPTR, 16);
		uint end = list[2].toUInt(Q_NULLPTR, 16);
		if(start > end)
			continue;
		newMax = qMax(newMax, end);

		QSqlQuery createBlockQuery(this->newDB);
		createBlockQuery.prepare(QStringLiteral("INSERT INTO Blocks (Name, BlockStart, BlockEnd) VALUES(:name, :start, :end)"));
		createBlockQuery.bindValue(":name", list[3]);
		createBlockQuery.bindValue(":start", start);
		createBlockQuery.bindValue(":end", end);
		TRY_EXEC(createBlockQuery);

		countNext(i + 1, max, buffer);
	}

	COMMIT_FINISH

	this->adjustMax(newMax);
}

void DatabaseUpdater::adjustMax(uint newMax)
{
	if(newMax > this->symbolMax) {
		uint delta = newMax - this->symbolMax;
		qDebug() << "newMax" << delta;
		uint buffer = 0;
		emit beginInstall(tr("Adding remaining missing symbols"), DatabaseUpdater::PercentMax);
		this->newDB.transaction();

		for(uint i = 1; i <= delta; ++i) {
			QSqlQuery insertUnnamedSymbolQuery(this->newDB);
			insertUnnamedSymbolQuery.prepare(QStringLiteral("INSERT INTO Symbols (Code) VALUES(:code)"));
			insertUnnamedSymbolQuery.bindValue(QStringLiteral(":code"), this->symbolMax + i);
			TRY_EXEC(insertUnnamedSymbolQuery)
			countNext(i, delta, buffer);
		}
		this->symbolMax = newMax;

		COMMIT_FINISH
	} else
		emit installReady();
}

void DatabaseUpdater::installNameIndex(const QByteArray &downloadData)
{
	emit beginInstall(tr("Adding new aliases from name index"), 0);

	UniMatrix nameMatrix = this->readDownload(downloadData, QLatin1Char('\t'), 2);

	emit beginInstall(tr("Adding new aliases from name index"), DatabaseUpdater::PercentMax);
	this->newDB.transaction();

	uint buffer = 0;
	for(uint i = 0, max = nameMatrix.size(); i < max; ++i) {
		const QStringList &list = nameMatrix[i];
		bool ok = false;
		uint code = list[1].toUInt(&ok, 16);
		const QString &name = list[0];

		if(ok &&
		   (name == name.toUpper() ||
			name == name.toLower())) {
			QSqlQuery insertAliasQuery(this->newDB);
			insertAliasQuery.prepare(QStringLiteral("INSERT OR IGNORE INTO Aliases (Code, NameAlias) VALUES(:code, :alias)"));
			insertAliasQuery.bindValue(QStringLiteral(":code"), code);
			insertAliasQuery.bindValue(QStringLiteral(":alias"), name.toUpper());
			TRY_EXEC(insertAliasQuery)
		}

		countNext(i + 1, max, buffer);
	}

	COMMIT_FINISH
}

void DatabaseUpdater::installAliases(const QByteArray &downloadData)
{
	emit beginInstall(tr("Adding new aliases from alias list"), 0);

	UniMatrix aliasMatrix = this->readDownload(downloadData, QLatin1Char(';'), 2);

	emit beginInstall(tr("Adding new aliases from alias list"), DatabaseUpdater::PercentMax);
	this->newDB.transaction();

	uint buffer = 0;
	for(uint i = 0, max = aliasMatrix.size(); i < max; ++i) {
		const QStringList &list = aliasMatrix[i];
		bool ok = false;
		uint code = list[0].toUInt(&ok, 16);
		if(ok) {
			QSqlQuery insertAliasQuery(this->newDB);
			insertAliasQuery.prepare(QStringLiteral("INSERT OR IGNORE INTO Aliases (Code, NameAlias) VALUES(:code, :alias)"));
			insertAliasQuery.bindValue(QStringLiteral(":code"), code);
			insertAliasQuery.bindValue(QStringLiteral(":alias"), list[1].toUpper());
			TRY_EXEC(insertAliasQuery)
		}

		countNext(i + 1, max, buffer);
	}

	COMMIT_FINISH

	if(ARG_UPDATE_MODE.testFlag(UpdaterWindow::RecentlyUsed))
		this->transferRecent();
	else
		this->completeUpdate();
}

void DatabaseUpdater::transferRecent()
{
	emit beginInstall(tr("Transfering recently used elements from the old database"), 0);
	this->newDB.transaction();

	QSqlDatabase oldDB = QSqlDatabase::database(QStringLiteral("oldDB"));
	TRY_OPEN(oldDB)

	QSqlQuery recentQuery(oldDB);
	recentQuery.prepare(QStringLiteral("SELECT Code, SymCount FROM Recent"));
	TRY_EXEC(recentQuery)

	while(recentQuery.next()) {
		QSqlQuery transferQuery(this->newDB);
		uint code = recentQuery.value(0).toUInt();
		transferQuery.prepare(QStringLiteral("INSERT INTO Recent (Code, SymCount) VALUES(:code, :num)"));
		transferQuery.bindValue(QStringLiteral(":code"), code);
		transferQuery.bindValue(QStringLiteral(":num"), recentQuery.value(1).toInt());
		if(!transferQuery.exec()) {
			emit error(tr("Recently used Symbol U+%1 could not be transfered!")
					   .arg(code, 4, 16, QChar('0')),
					   false);
		}
	}

	oldDB.close();
	COMMIT_FINISH

	QMetaObject::invokeMethod(this, "completeUpdate", Qt::QueuedConnection);
}

void DatabaseUpdater::completeUpdate()
{
	emit beginInstall(tr("Completing update"), 0);

	if(this->newDB.isOpen())
		this->newDB.close();
	this->newDB = QSqlDatabase();
	QSqlDatabase::removeDatabase(QStringLiteral("newDB"));

	QSqlDatabase oldDB = QSqlDatabase::database(QStringLiteral("oldDB"));
	if(oldDB.isOpen())
		oldDB.close();
	oldDB = QSqlDatabase();
	QSqlDatabase::removeDatabase(QStringLiteral("oldDB"));

	QString path = ARG_LOCAL_DB_PATH;
	if(!QFile::exists(path) || QFile::remove(path)) {
		if(QFile::rename(path + QStringLiteral(".update"), path))
			emit installReady();
		else
			emit error(tr("Failed to rename update to real database!"), true);
	} else
		emit error(tr("Failed to delete old database!"), true);
}

DatabaseUpdater::UniMatrix DatabaseUpdater::readDownload(const QByteArray &data, QChar seperator, int columns)
{
	UniMatrix codeMatrix;
	QTextStream stream(data);
	while(!stream.atEnd()) {
		QString lineString = stream.readLine();
		if(lineString.isEmpty() || lineString.startsWith(QLatin1Char('#')))
		   continue;
		QStringList line = lineString.split(seperator);
		if(line.size() >= columns)
			codeMatrix += line;
	}
	return codeMatrix;
}

DatabaseUpdater::UniMatrix DatabaseUpdater::readDownload(const QByteArray &data, const QRegularExpression &regex)
{
	UniMatrix codeMatrix;
	QTextStream stream(data);
	while(!stream.atEnd()) {
		QRegularExpressionMatch match = regex.match(stream.readLine());
		if(match.hasMatch())
			codeMatrix += match.capturedTexts();
	}
	return codeMatrix;
}

void DatabaseUpdater::doAbort()
{
	if(this->newDB.isOpen())
		this->newDB.close();
	this->newDB = QSqlDatabase();
	QSqlDatabase::removeDatabase(QStringLiteral("newDB"));
	QSqlDatabase::removeDatabase(QStringLiteral("oldDB"));
	QFile::remove(ARG_LOCAL_DB_PATH + QStringLiteral(".update"));
	emit abortDone();
}

void DatabaseUpdater::countNext(uint counter, uint max, uint &buffer)
{
	uint val = DatabaseUpdater::PercentMax * counter / max;
	if(val > buffer) {
		buffer = val;
		emit updateInstallProgress(val);
	}
}
