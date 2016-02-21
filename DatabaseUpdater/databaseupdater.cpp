#include "databaseupdater.h"
#include <QThread>
#include <QSqlQuery>
#include <QSqlError>
#include <QTextStream>
#include "global.h"

#define DELETE_QUEUED(obj) QMetaObject::invokeMethod(obj, "deleteLater", Qt::QueuedConnection)

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
	return 2;
}

void DatabaseUpdater::startInstalling()
{
	QFile queryFile(QStringLiteral(":/data/dbSetup.sql"));
	queryFile.open(QIODevice::ReadOnly);
	Q_ASSERT(queryFile.isOpen());
	QStringList queries = QString::fromUtf8(queryFile.readAll())
						  .split(QLatin1Char(';'), QString::SkipEmptyParts);
	queryFile.close();
	emit beginInstall(tr("Creating database"), queries.size());

	QString path = ARG_LOCAL_DB_PATH + QStringLiteral(".update");
	QFile::remove(path);

	this->newDB = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"),
											QStringLiteral("newDB"));
	this->newDB.setDatabaseName(path);
	if(!this->newDB.open() || !this->newDB.isValid()) {
		emit error(this->newDB.lastError().text(), true);
		return;
	}

	for(int i = 0, max = queries.size(); i < max; ++i) {
		QSqlQuery setupQuery(this->newDB);
		if(!setupQuery.exec(queries[i])) {
			emit error(setupQuery.lastError().text(), true);
			return;
		}
		emit updateInstallProgress(i + 1);
	}

	emit installReady();
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
			emit error(tr("Invalid UnicodeData.txt file1! (Download corrupted?)"), true);
			return;
		}
	} while(file->peek(1) != "\n");
	QByteArrayList lst = file->readAll().split(';');
	uint max = lst.first().toUInt(&ok, 16);
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
		QStringList line = stream.readLine().split(QLatin1Char(';'));
		uint code = line.first().toUInt(&ok, 16);
		if(!ok || line.size() < 2) {
			emit error(tr("Invalid UnicodeData.txt file3! (Download corrupted?)"), true);
			return;
		}

		for(; counter < code; counter++) {
			if(this->abortRequested)
				return;

			QSqlQuery insertUnnamedQuery(this->newDB);
			insertUnnamedQuery.prepare(QStringLiteral("INSERT INTO Symbols (Code) VALUES(:code)"));
			insertUnnamedQuery.bindValue(QStringLiteral(":code"), counter);
			if(!insertUnnamedQuery.exec()) {
				emit error(insertUnnamedQuery.lastError().text(), true);
				return;
			}

			countNext(counter + 1, max, buffer);
		}

		QSqlQuery insertNamedQuery(this->newDB);
		insertNamedQuery.prepare(QStringLiteral("INSERT INTO Symbols (Code, Name) VALUES(:code, :name)"));
		insertNamedQuery.bindValue(QStringLiteral(":code"), counter);
		insertNamedQuery.bindValue(QStringLiteral(":name"), line[1].toUpper());
		if(!insertNamedQuery.exec()) {
			emit error(insertNamedQuery.lastError().text(), true);
			return;
		}
		//TODO alias
		countNext(++counter, max, buffer);
	}
	this->newDB.commit();

	this->newDB.close();
	this->newDB = QSqlDatabase();
	QSqlDatabase::removeDatabase(QStringLiteral("newDB"));

	emit installReady();
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
