#include "basedownloader.h"
#include <QNetworkRequest>
#include "global.h"

BaseDownloader::BaseDownloader(QObject *parent) :
	QObject(parent),
	nam(new QNetworkAccessManager(this)),
	downloadFiles(),
	currentReply(Q_NULLPTR),
	currentFile(Q_NULLPTR),
	aborted(false)
{}

int BaseDownloader::getDownloadCount() const
{
	int res = 4;
	if(ARG_UPDATE_MODE.testFlag(UpdaterWindow::Emojis))
		res += 7;
	return res;
}

void BaseDownloader::startDownloading()
{
	QString urlBase = QStringLiteral("ftp://ftp.unicode.org/Public/%1/ucd/").arg(ARG_UPDATE_VERSION);

	this->downloadFiles = {
		urlBase + QStringLiteral("UnicodeData.txt"),
		urlBase + QStringLiteral("Blocks.txt"),
		urlBase + QStringLiteral("Index.txt"),
		urlBase + QStringLiteral("NameAliases.txt"),
	};
	if(ARG_UPDATE_MODE.testFlag(UpdaterWindow::Emojis)) {
		this->downloadFiles += {
			QStringLiteral("http://emojipedia.org/people"),
			QStringLiteral("http://emojipedia.org/nature"),
			QStringLiteral("http://emojipedia.org/food-drink"),
			QStringLiteral("http://emojipedia.org/activity"),
			QStringLiteral("http://emojipedia.org/travel-places"),
			QStringLiteral("http://emojipedia.org/objects"),
			QStringLiteral("http://emojipedia.org/symbols")
		};
	}

	this->doDownload(QUrl(this->downloadFiles.takeFirst()));
}

void BaseDownloader::abortDownloading()
{
	this->aborted = true;
	if(this->currentReply) {
		disconnect(this->currentReply, &QNetworkReply::downloadProgress,
				   this, &BaseDownloader::updateDownloadProgress);
		this->currentReply->abort();
	} else if(this->downloadFiles.isEmpty())
		emit abortDone();
}

void BaseDownloader::doDownload(const QUrl &url)
{
	Q_ASSERT(!this->currentReply);
	Q_ASSERT(!this->currentFile);

	this->currentFile = new QTemporaryFile(this);
	if(!this->currentFile->open())
		emit error(tr("Failed to create temporary file!"), true);
	else {
		emit beginDownload(url);
		this->currentReply = this->nam->get(QNetworkRequest(url));
		connect(this->currentReply, &QNetworkReply::downloadProgress,
				this, &BaseDownloader::updateDownloadProgress);
		connect(this->currentReply, &QNetworkReply::readyRead,
				this, &BaseDownloader::writePart);
		connect(this->currentReply, &QNetworkReply::finished,
				this, &BaseDownloader::replyReady, Qt::QueuedConnection);
		connect(this->currentReply, SELECT<QNetworkReply::NetworkError>::OVERLOAD_OF(&QNetworkReply::error),
				this, &BaseDownloader::downloadError);
	}
}

void BaseDownloader::writePart()
{
	Q_ASSERT(this->currentReply);
	Q_ASSERT(this->currentFile);
	this->currentFile->write(this->currentReply->readAll());
}

void BaseDownloader::replyReady()
{
	Q_ASSERT(this->currentReply);
	Q_ASSERT(this->currentFile);
	if(this->aborted) {
		this->currentFile->close();
		this->currentFile->deleteLater();
		this->currentFile = Q_NULLPTR;
		this->currentReply->deleteLater();
		this->currentReply = Q_NULLPTR;
		emit abortDone();
	} else {
		this->currentFile->write(this->currentReply->readAll());
		this->currentFile->seek(0);
		emit downloadReady(this->currentFile);
		this->currentFile = Q_NULLPTR;
		this->currentReply->deleteLater();
		this->currentReply = Q_NULLPTR;
		if(!this->downloadFiles.isEmpty())
			this->doDownload(this->downloadFiles.takeFirst());
	}
}

void BaseDownloader::downloadError(QNetworkReply::NetworkError)
{
	if(this->aborted)
		emit abortDone();
	else
		emit error(this->currentReply->errorString(), true);
}
