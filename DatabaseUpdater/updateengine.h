#ifndef UPDATEENGINE_H
#define UPDATEENGINE_H

#include <QObject>
#include <QQueue>
#include <QNetworkAccessManager>
#include <QFutureWatcher>
#include <QTemporaryFile>
#include "updatetask.h"
#include "updateenginecore.h"

class UpdateEngine : public QObject, public UpdateEngineCore
{
	Q_OBJECT
public:
	explicit UpdateEngine(QObject *parent = Q_NULLPTR);
	~UpdateEngine();

	// UpdateEngineCore interface
	void failure(const QString &error) Q_DECL_OVERRIDE;
	void logError(const QString &error) Q_DECL_OVERRIDE;
	bool testAbort() const Q_DECL_OVERRIDE;
	void updateInstallMax(int max) Q_DECL_OVERRIDE;
	void updateInstallValue(int value) Q_DECL_OVERRIDE;
	UniMatrix readDownload(const QByteArray &data, QChar seperator, int columns) Q_DECL_OVERRIDE;
	UniMatrix readDownload(const QByteArray &data, const QRegularExpression &regex) Q_DECL_OVERRIDE;

public slots:
	void addTask(UpdateTask *task);

	void abort();

signals:
	//general
	void error(const QString &error, bool isNetwork);
	void log(const QString &error);

	void abortDone();
	void engineDone();

	//download
	void downloadMaxChanged(int newMax);
	void beginDownload(const QString &text);
	void updateDownloadProgress(qint64 value, qint64 max);
	void downloadDone();

	//install
	void installMaxChanged(int newMax);
	void beginInstall(const QString &text);
	void beginInstallProgress(int maximum);
	void updateInstallProgress(int value);
	void installDone();

private slots:
	//download
	void nextDownload();

	void downloadReady();
	void downloadError();

	//install
	void nextInstall();

	void watcherReady();

	void completeInstall();

	//general
	void tryAbortReady();
	void closeDBs();

private:
	//general
	bool abortRequested;
	QTemporaryFile *newDBFile;

	//download
	int downloadMax;
	QQueue<UpdateTask*> downloads;
	UpdateTask *currentDownload;
	bool isDownloadCompleted;
	bool didAbortDownload;

	QNetworkAccessManager *nam;
	QNetworkReply *currentReply;

	//install
	int installMax;
	QQueue<UpdateTask*> installs;
	UpdateTask *currentInstall;
	bool isInstallCompleted;
	bool didAbortInstall;

	QFutureWatcher<bool> *watcher;
	int installProgressMax;
	int installProgressBuffer;
};

#endif // UPDATEENGINE_H
