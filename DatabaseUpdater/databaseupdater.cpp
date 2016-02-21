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
		if(!queries[i].simplified().isEmpty()) {
			QSqlQuery setupQuery(this->newDB);
			if(!setupQuery.exec(queries[i])) {
				emit error(setupQuery.lastError().text(), true);
				return;
			}
		}
		emit updateInstallProgress(i + 1);
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
