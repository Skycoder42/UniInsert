#ifndef BASEDOWNLOADER_H
#define BASEDOWNLOADER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QTemporaryFile>
#include <QUrl>
#include <QNetworkReply>
#include <functional>

class BaseDownloader : public QObject
{
	Q_OBJECT
public:
	explicit BaseDownloader(QObject *parent = Q_NULLPTR);

	int getDownloadCount() const;

public slots:
	void startDownloading();
	void abortDownloading();

signals:
	void beginDownload(const QUrl &url);
	void downloadReady(QTemporaryFile *downloadFile);
	void updateDownloadProgress(qint64 value, qint64 max);
	void error(const QString &error, bool critical);

	void abortDone();

private slots:
	void doDownload(const QUrl &url);
	void writePart();
	void replyReady();

	void downloadError(QNetworkReply::NetworkError);

private:
	QNetworkAccessManager *nam;
	QList<QUrl> downloadFiles;
	bool aborted;

	QNetworkReply *currentReply;
	QTemporaryFile *currentFile;
};

#endif // BASEDOWNLOADER_H