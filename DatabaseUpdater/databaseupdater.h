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

	void handleDownloadFile(QTemporaryFile *downloadFile);

signals:
	void beginInstall(const QString &text, int max);
	void installReady();
	void updateInstallProgress(int value);
	void error(const QString &error, bool critical);

	void abortDone();

private slots:
	void installCodeData(QTemporaryFile *file);
	void findName(const QStringList &entry, QString &name, QStringList &aliases);

	void installBlocks(QTemporaryFile *file);
	void adjustMax(uint newMax);

	void doAbort();
	void countNext(uint counter, uint max, uint &buffer);

private:
	static const int PercentMax = 1000;

	QSqlDatabase newDB;
	bool abortRequested;
	int nextFunc;

	uint symbolMax;
};

#endif // DATABASEUPDATER_H
