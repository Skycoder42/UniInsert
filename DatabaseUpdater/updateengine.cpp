#include "updateengine.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QtConcurrent>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "global.h"

const QString UpdateEngineCore::newDB = QStringLiteral("newDB");
const QString UpdateEngineCore::oldDB = QStringLiteral("oldDB");



class SetupTask : public UpdateTask
{
public:
	SetupTask(QTemporaryFile *tempFile, UpdateEngineCore *engine);

	// UpdateTask interface
	QString installText() const Q_DECL_OVERRIDE;
	bool run() Q_DECL_OVERRIDE;

private:
	QTemporaryFile *tempFile;
};



UpdateEngine::UpdateEngine(QObject *parent) :
	QObject(parent),
	abortRequested(false),
	newDBFile(new QTemporaryFile(this)),
	downloadMax(0),
	downloads(),
	currentDownload(Q_NULLPTR),
	isDownloadCompleted(false),
	didAbortDownload(false),
	nam(new QNetworkAccessManager(this)),
	currentReply(Q_NULLPTR),
	installMax(1),
	installs(),
	currentInstall(Q_NULLPTR),
	isInstallCompleted(false),
	didAbortInstall(false),
	watcher(new QFutureWatcher<bool>(this)),
	installProgressMax(0),
	installProgressBuffer(0)
{
	connect(this->watcher, &QFutureWatcherBase::finished,
			this, &UpdateEngine::watcherReady);
	connect(this, &UpdateEngine::error,
			this, &UpdateEngine::abort);

	this->newDBFile->open();
	this->newDBFile->close();
	this->installs.enqueue(new SetupTask(this->newDBFile, this));
}

UpdateEngine::~UpdateEngine()
{
	qDeleteAll(this->downloads);
	delete this->currentDownload;
	qDeleteAll(this->installs);
	delete this->currentInstall;
	delete this->currentReply;
}

void UpdateEngine::failure(const QString &error)
{
	QMetaObject::invokeMethod(this, "error", Qt::QueuedConnection,
							  Q_ARG(QString, error));
}

void UpdateEngine::logError(const QString &error)
{
	QMetaObject::invokeMethod(this, "log", Qt::QueuedConnection,
							  Q_ARG(QString, error));
}

bool UpdateEngine::testAbort() const
{
	return this->abortRequested;
}

void UpdateEngine::updateInstallMax(int max)
{
	this->installProgressMax = max;
	this->installProgressBuffer = 0;
	QMetaObject::invokeMethod(this, "beginInstallProgress", Qt::QueuedConnection,
							  Q_ARG(int, 1000));
}

void UpdateEngine::updateInstallValue(int value)
{
	int val = 1000 * value / this->installProgressMax;
	if(val > this->installProgressBuffer) {
		this->installProgressBuffer = val;
		QMetaObject::invokeMethod(this, "updateInstallProgress", Qt::QueuedConnection,
								  Q_ARG(int, val));
	}
}

UpdateEngineCore::UniMatrix UpdateEngine::readDownload(const QByteArray &data, QChar seperator, int columns)
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

UpdateEngineCore::UniMatrix UpdateEngine::readDownload(const QByteArray &data, const QRegularExpression &regex)
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

void UpdateEngine::addTask(UpdateTask *task)
{
	task->engine = this;
	if(this->isInstallCompleted || this->abortRequested) {
		delete task;
		return;
	}

	if(task->downloadUrl().isValid())
		emit downloadMaxChanged(++this->downloadMax);
	emit installMaxChanged(++this->installMax);

	this->isDownloadCompleted = false;
	this->downloads.enqueue(task);
	QMetaObject::invokeMethod(this, "nextDownload", Qt::QueuedConnection);
	QMetaObject::invokeMethod(this, "nextInstall", Qt::QueuedConnection);
}

void UpdateEngine::abort()
{
	this->abortRequested = true;
	if(this->currentReply)
		this->currentReply->abort();
	else
		this->didAbortDownload = true;
	if(!this->currentInstall)
		this->didAbortInstall = true;
	this->tryAbortReady();
}

void UpdateEngine::nextDownload()
{
	if(this->currentDownload || this->abortRequested)
		return;
	Q_ASSERT(!this->currentReply);

	if(!this->downloads.isEmpty()) {
		QUrl downloadUrl = this->downloads.head()->downloadUrl();
		if(downloadUrl.isValid()) {
			this->currentDownload = this->downloads.dequeue();
			emit beginDownload(this->currentDownload->downloadText());

			QNetworkRequest request(downloadUrl);
			request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
			this->currentReply = this->nam->get(request);
			connect(this->currentReply, &QNetworkReply::downloadProgress,
					this, &UpdateEngine::updateDownloadProgress);
			connect(this->currentReply, &QNetworkReply::finished,
					this, &UpdateEngine::downloadReady);
			connect(this->currentReply, SELECT<QNetworkReply::NetworkError>::OVERLOAD_OF(&QNetworkReply::error),
					this, &UpdateEngine::downloadError);
		} else {
			this->installs.enqueue(this->downloads.dequeue());
			QMetaObject::invokeMethod(this, "nextDownload", Qt::QueuedConnection);
			QMetaObject::invokeMethod(this, "nextInstall", Qt::QueuedConnection);
		}
	} else
		this->isDownloadCompleted = true;
}

void UpdateEngine::downloadReady()
{
	if(this->currentReply && this->currentReply->error() == QNetworkReply::NoError) {
		if(this->abortRequested) {
			this->didAbortDownload = true;
			this->currentReply->deleteLater();
			this->currentReply = Q_NULLPTR;
			this->tryAbortReady();
		} else {
			this->currentDownload->downloadData = this->currentReply->readAll();
			this->currentReply->deleteLater();
			this->currentReply = Q_NULLPTR;

			this->installs.enqueue(this->currentDownload);
			this->currentDownload = Q_NULLPTR;
			emit downloadDone();
			QMetaObject::invokeMethod(this, "nextDownload", Qt::QueuedConnection);
			QMetaObject::invokeMethod(this, "nextInstall", Qt::QueuedConnection);
		}
	}
}

void UpdateEngine::downloadError()
{
	if(this->abortRequested) {
		this->didAbortDownload = true;
		this->currentReply->deleteLater();
		this->currentReply = Q_NULLPTR;
		this->tryAbortReady();
	} else {
		QString errorString = this->currentReply->errorString();
		this->currentReply->deleteLater();
		this->currentReply = Q_NULLPTR;
		emit error(errorString);
	}
}

void UpdateEngine::nextInstall()
{
	if(this->currentInstall || this->abortRequested)
		return;
	Q_ASSERT(this->watcher->isFinished());

	if(this->installs.isEmpty()) {
		if(this->isDownloadCompleted)
			this->completeInstall();
	} else {
		this->currentInstall = this->installs.dequeue();
		emit beginInstall(this->currentInstall->installText());

		this->watcher->setFuture(QtConcurrent::run(this->currentInstall, &UpdateTask::run));
	}
}

void UpdateEngine::watcherReady()
{
	if(this->abortRequested) {
		this->didAbortInstall = true;
		delete this->currentInstall;
		this->currentInstall = Q_NULLPTR;
		this->tryAbortReady();
	} else if(this->watcher->result()) {
		QList<UpdateTask*> newTasks = this->currentInstall->newTasks();
		for(UpdateTask *task : newTasks)
			this->addTask(task);
		delete this->currentInstall;
		this->currentInstall = Q_NULLPTR;

		emit installDone();
		QMetaObject::invokeMethod(this, "nextInstall", Qt::QueuedConnection);
	}
}

void UpdateEngine::completeInstall()
{
	this->closeDBs();

	QString path = ARG_LOCAL_DB_PATH;
	if(!QFile::exists(path) || QFile::remove(path)) {
		QString tempPath = this->newDBFile->fileName();
		this->newDBFile->setAutoRemove(false);
		delete this->newDBFile;
		if(QFile::rename(tempPath, path))
			emit engineDone();
		else
			emit error(tr("Failed to rename update to real database!"));
	} else
		emit error(tr("Failed to delete old database!"));
}

void UpdateEngine::tryAbortReady()
{
	if(this->didAbortDownload && this->didAbortInstall) {
		this->closeDBs();
		emit abortDone();
	}
}

void UpdateEngine::closeDBs()
{
	QSqlDatabase newDB = QSqlDatabase::database(UpdateEngineCore::newDB);
	if(newDB.isOpen())
		newDB.close();
	newDB = QSqlDatabase();
	QSqlDatabase::removeDatabase(UpdateEngineCore::newDB);

	QSqlDatabase oldDB = QSqlDatabase::database(UpdateEngineCore::oldDB);
	if(oldDB.isOpen())
		oldDB.close();
	oldDB = QSqlDatabase();
	QSqlDatabase::removeDatabase(UpdateEngineCore::oldDB);
}


SetupTask::SetupTask(QTemporaryFile *tempFile, UpdateEngineCore *engine) :
	UpdateTask(),
	tempFile(tempFile)
{
	this->engine = engine;
}

QString SetupTask::installText() const
{
	return UpdateEngineCore::tr("Preparing databases");
}

bool SetupTask::run()
{
	QString path = this->tempFile->fileName();
	QFile::remove(path);
	QSqlDatabase newDB = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), UpdateEngineCore::newDB);
	newDB.setDatabaseName(path);
	if(!newDB.open() || !newDB.isValid()) {
		this->engine->failure(newDB.lastError().text());
		return false;
	}
	newDB.exec(QStringLiteral("PRAGMA foreign_keys = ON"));

	if(QFile::exists(ARG_LOCAL_DB_PATH)) {
		QSqlDatabase oldDB = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), UpdateEngineCore::oldDB);
		oldDB.setDatabaseName(ARG_LOCAL_DB_PATH);
		if(!oldDB.isValid()) {
			this->engine->failure(oldDB.lastError().text());
			return false;
		}
	}

	return true;
}
