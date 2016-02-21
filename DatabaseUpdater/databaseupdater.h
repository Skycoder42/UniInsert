#ifndef DATABASEUPDATER_H
#define DATABASEUPDATER_H

#include <QObject>
#include <QTemporaryFile>
#include <QQueue>
#include <QSqlDatabase>

class DatabaseUpdater : public QObject
{
	Q_OBJECT
public:
	explicit DatabaseUpdater(QObject *parent = Q_NULLPTR);
	~DatabaseUpdater();

	int getInstallCount() const;

public slots:
	void startInstalling();
	void abortInstalling();

	void addDownloadFile(QTemporaryFile *downloadFile);

signals:
	void beginInstall(const QString &text, bool indeterminated);
	void installReady();
	void updateInstallProgress(int value, int max);
	void error(const QString &error, bool critical);

	void abortDone();

private:
	QQueue<QTemporaryFile*> downloadFiles;
	bool aborted;

	QSqlDatabase newDB;
};

#endif // DATABASEUPDATER_H
