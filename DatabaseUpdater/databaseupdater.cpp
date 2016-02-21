#include "databaseupdater.h"
#include <QThread>
#include <QSqlQuery>
#include <QSqlError>
#include "global.h"

DatabaseUpdater::DatabaseUpdater(QObject *parent) :
	QObject(Q_NULLPTR),
	downloadFiles(),
	aborted(false)
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
	if(this->newDB.isOpen())
		this->newDB.close();
	this->newDB = QSqlDatabase();
	QSqlDatabase::removeDatabase(QStringLiteral("newDB"));
}

int DatabaseUpdater::getInstallCount() const
{
	return 1;
}

void DatabaseUpdater::startInstalling()
{
	emit updateInstallProgress(0, 0);

	this->newDB = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"),
											QStringLiteral("newDB"));
	this->newDB.setDatabaseName(ARG_LOCAL_DB_PATH + QStringLiteral(".update"));
	if(!this->newDB.open() || !this->newDB.isValid()) {
		emit error(this->newDB.lastError().text(), true);
		return;
	}

	QFile queryFile(QStringLiteral(":/data/dbSetup.sql"));
	queryFile.open(QIODevice::ReadOnly);
	Q_ASSERT(queryFile.isOpen());
	QSqlQuery setupQuery(this->newDB);
	if(!setupQuery.exec(QString::fromUtf8(queryFile.readAll()))) {
		emit error(this->newDB.lastError().text(), true);
		return;
	}

	emit installReady();
}

void DatabaseUpdater::abortInstalling()
{
	this->aborted = true;
}

void DatabaseUpdater::addDownloadFile(QTemporaryFile *downloadFile)
{
	QMetaObject::invokeMethod(downloadFile, "deleteLater", Qt::QueuedConnection);
}
