#include "basedownloader.h"
#include <QNetworkRequest>
#include "global.h"

BaseDownloader::BaseDownloader(QObject *parent) :
	QObject(parent),
	nam(new QNetworkAccessManager(this)),
	downloadFiles(),
	currentReply(Q_NULLPTR),
	aborted(false)
{}

int BaseDownloader::getDownloadCount() const
{
	int res = 4;
	if(ARG_UPDATE_MODE.testFlag(UpdaterWindow::Emojis))
		++res;
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

	if(ARG_UPDATE_MODE.testFlag(UpdaterWindow::Emojis))
		this->doEmoDownload(QStringLiteral("http://emojipedia.org"));
	else
		this->doDownload(this->downloadFiles.takeFirst());
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

	emit beginDownload(url);
	this->currentReply = this->nam->get(QNetworkRequest(url));
	connect(this->currentReply, &QNetworkReply::downloadProgress,
			this, &BaseDownloader::updateDownloadProgress);
	connect(this->currentReply, &QNetworkReply::finished,
			this, &BaseDownloader::replyReady, Qt::QueuedConnection);
	connect(this->currentReply, SELECT<QNetworkReply::NetworkError>::OVERLOAD_OF(&QNetworkReply::error),
			this, &BaseDownloader::downloadError);
}

void BaseDownloader::replyReady()
{
	if(this->currentReply->error() == QNetworkReply::NoError) {
		Q_ASSERT(this->currentReply);
		emit downloadReady(this->currentReply->readAll());
		this->currentReply->deleteLater();
		this->currentReply = Q_NULLPTR;
		if(!this->downloadFiles.isEmpty())
			this->doDownload(this->downloadFiles.takeFirst());
	}
}

void BaseDownloader::doEmoDownload(const QUrl &url)
{
	Q_ASSERT(!this->currentReply);

	emit beginDownload(url);
	this->currentReply = this->nam->get(QNetworkRequest(url));
	connect(this->currentReply, &QNetworkReply::downloadProgress,
			this, &BaseDownloader::updateDownloadProgress);
	connect(this->currentReply, &QNetworkReply::finished,
			this, &BaseDownloader::emoReplyReady, Qt::QueuedConnection);
	connect(this->currentReply, SELECT<QNetworkReply::NetworkError>::OVERLOAD_OF(&QNetworkReply::error),
			this, &BaseDownloader::downloadError);
}

void BaseDownloader::emoReplyReady()
{
	if(this->currentReply->error() == QNetworkReply::NoError) {
		Q_ASSERT(this->currentReply);
		QList<QUrl> emojiSubs = this->parseEmojiSide(this->currentReply->readAll());
		this->currentReply->deleteLater();
		this->currentReply = Q_NULLPTR;
		if(!emojiSubs.isEmpty()) {
			this->downloadFiles += emojiSubs;
			emit emojiCountLoaded(emojiSubs.size());
			if(!this->downloadFiles.isEmpty())
				this->doDownload(this->downloadFiles.takeFirst());
		}
	}
}

#define EMOJI_ERROR(var, i) if(var == -1) {\
		emit error(emojiPediaError() + QString::number(i), true);\
		return QList<QUrl>();\
	}

QList<QUrl> BaseDownloader::parseEmojiSide(const QByteArray &data)
{
	QList<QUrl> emojiSubs;

	// Parse the file!!!
	int start = data.indexOf("Categories");
	EMOJI_ERROR(start, 0);
	start = data.indexOf("<ul>", start);
	EMOJI_ERROR(start, 1);
	int finish = data.indexOf("</ul>", start);
	EMOJI_ERROR(finish, 2);

	int nextStart = start;
	while (nextStart < finish && nextStart != -1) {
		int begin = data.indexOf("<li><a href=\"", nextStart);
		EMOJI_ERROR(begin, 3);
		begin += 13;
		int end = data.indexOf("\"", begin);
		EMOJI_ERROR(end, 4);

		qDebug() << begin << end - begin;
		emojiSubs += QString::fromUtf8("http://emojipedia.org/" + data.mid(begin, end - begin));
		nextStart = data.indexOf("</li>", end);
		EMOJI_ERROR(nextStart, 5);
		nextStart = data.indexOf("<li>", nextStart);
	}

	qDebug() << emojiSubs;
	return emojiSubs;
}

void BaseDownloader::downloadError(QNetworkReply::NetworkError)
{
	Q_ASSERT(this->currentReply);

	if(this->aborted)
		emit abortDone();
	else
		emit error(this->currentReply->errorString(), true);

	this->currentReply->deleteLater();
	this->currentReply = Q_NULLPTR;
}
