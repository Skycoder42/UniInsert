#include "databaseupdater.h"
#include <QThread>
#include <QSqlQuery>
#include <QSqlError>
#include <QTextStream>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include "global.h"

static QRegularExpression groupMatchRegex(QStringLiteral(R"__(^\<(.+),\s*(First|Last)\s*\>$)__"));

#define DELETE_QUEUED(obj) QMetaObject::invokeMethod(obj, "deleteLater", Qt::QueuedConnection)
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
	nextFunc(0)
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
	return 3;
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

	this->newDB = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"),
											QStringLiteral("newDB"));
	this->newDB.setDatabaseName(path);
	if(!this->newDB.open() || !this->newDB.isValid()) {
		emit error(this->newDB.lastError().text(), true);
		return;
	}

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

void DatabaseUpdater::handleDownloadFile(QTemporaryFile *downloadFile)
{
	if(this->abortRequested){
		DELETE_QUEUED(downloadFile);
		return;
	}

	switch (this->nextFunc++) {
	case 0:
		this->installCodeData(downloadFile);
		break;
	case 1:
		this->installBlocks(downloadFile);
		break;
	default:
		break;
		Q_UNREACHABLE();//TODO
	}
}

void DatabaseUpdater::installCodeData(QTemporaryFile *file)
{
	bool ok = false;
	emit beginInstall(tr("Creating unicode symbols"), 1000);//"percent"...

	file->seek(file->size() - 5);//saveguard for the last \n
	do {
		if(!file->seek(file->pos() - 1)) {
			emit error(tr("Invalid UnicodeData.txt file! (Download corrupted?)"), true);
			return;
		}
	} while(file->peek(1) != "\n");
	QByteArrayList lst = file->readAll().split(';');
	this->symbolMax = lst.first().toUInt(&ok, 16);
	if(!ok) {
		emit error(tr("Invalid UnicodeData.txt file! (Download corrupted?)"), true);
		return;
	}

	file->seek(0);
	this->newDB.transaction();
	QTextStream stream(file);
	uint counter = 0;
	uint buffer = 0;
	while(!stream.atEnd()) {
		if(this->abortRequested)
			return;
		QStringList line = stream.readLine().split(QLatin1Char(';'));
		uint code = line.first().toUInt(&ok, 16);
		if(!ok || line.size() < 2) {
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
			insertSymbolQuery.bindValue(QStringLiteral(":name"), name);
		}
		insertSymbolQuery.bindValue(QStringLiteral(":code"), counter);
		TRY_EXEC(insertSymbolQuery)

		for(QString alias : aliases) {
			QSqlQuery insertAliasQuery(this->newDB);
			insertAliasQuery.prepare(QStringLiteral("INSERT OR IGNORE INTO Aliases (Code, NameAlias) VALUES(:code, :alias)"));
			insertAliasQuery.bindValue(QStringLiteral(":code"), counter);
			insertAliasQuery.bindValue(QStringLiteral(":alias"), alias);
			TRY_EXEC(insertAliasQuery)
		}

		countNext(++counter, this->symbolMax, buffer);
	}

	COMMIT_FINISH
	DELETE_QUEUED(file);
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

void DatabaseUpdater::installBlocks(QTemporaryFile *file)
{
	emit beginInstall(tr("Creating symbol blocks"), 0);

	typedef QPair<uint, uint> Range;
	typedef QPair<Range, QString> BlockInfo;
	typedef QList<BlockInfo> BlockList;

	BlockList list;
	uint newMax = 0;
	QRegularExpression regex(QStringLiteral(R"__(^([0-9A-F]{4,})\.\.([0-9A-F]{4,});\s*(.+)$)__"));
	regex.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
	regex.optimize();

	QTextStream stream(file);
	while(!stream.atEnd()) {
		QRegularExpressionMatch match = regex.match(stream.readLine());
		if(match.hasMatch()) {
			BlockInfo info;
			info.first.first = match.captured(1).toUInt(Q_NULLPTR, 16);
			info.first.second = match.captured(2).toUInt(Q_NULLPTR, 16);
			if(info.first.first > info.first.second)
				continue;
			info.second = match.captured(3);
			list += info;
			newMax = qMax(newMax, info.first.second);
		}
	}

	emit beginInstall(tr("Creating symbol blocks"), list.size());
	this->newDB.transaction();

	for(int i = 0, max = list.size(); i < max; ++i) {
		QSqlQuery createBlockQuery(this->newDB);
		createBlockQuery.prepare(QStringLiteral("INSERT INTO Blocks (Name, BlockStart, BlockEnd) VALUES(:name, :start, :end)"));
		createBlockQuery.bindValue(":name", list[i].second);
		createBlockQuery.bindValue(":start", list[i].first.first);
		createBlockQuery.bindValue(":end", list[i].first.second);
		TRY_EXEC(createBlockQuery);
		emit updateInstallProgress(i + 1);
	}

	COMMIT_FINISH
	DELETE_QUEUED(file);

	this->newDB.close();
	this->newDB = QSqlDatabase();
	QSqlDatabase::removeDatabase(QStringLiteral("newDB"));
}

void DatabaseUpdater::doAbort()
{
	if(this->newDB.isOpen()) {
		this->newDB.close();
		this->newDB = QSqlDatabase();
		QSqlDatabase::removeDatabase(QStringLiteral("newDB"));
	}
	QFile::remove(ARG_LOCAL_DB_PATH + QStringLiteral(".update"));
	emit abortDone();
}

void DatabaseUpdater::countNext(uint counter, uint max, uint &buffer)
{
	uint val = 1000 * counter / max;
	if(val > buffer) {
		buffer = val;
		emit updateInstallProgress(val);
	}
}
