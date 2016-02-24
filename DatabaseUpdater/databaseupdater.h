#ifndef DATABASEUPDATER_H
#define DATABASEUPDATER_H

#include <QObject>
#include <QByteArray>
#include <QQueue>
#include <QSqlDatabase>

class DatabaseUpdater : public QObject
{
	Q_OBJECT

	typedef QList<QStringList> UniMatrix;
public:
	explicit DatabaseUpdater(QObject *parent = Q_NULLPTR);
	~DatabaseUpdater();

	int getInstallCount() const;

public slots:
	void startInstalling();
	void abortInstalling();

	void handleDownload(const QByteArray &downloadData);

signals:
	void beginInstall(const QString &text, int max);
	void installReady();
	void updateInstallProgress(int value);
	void error(const QString &error, bool critical);

	void abortDone();

private slots:
	void installCodeData(const QByteArray &downloadData);
	void findName(const QStringList &entry, QString &name, QStringList &aliases);

	void installBlocks(const QByteArray &downloadData);
	void adjustMax(uint newMax);

	void installNameIndex(const QByteArray &downloadData);

	void installAliases(const QByteArray &downloadData);

	UniMatrix readDownload(const QByteArray &data, QChar seperator, int columns);
	UniMatrix readDownload(const QByteArray &data, const QRegularExpression &regex);

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
