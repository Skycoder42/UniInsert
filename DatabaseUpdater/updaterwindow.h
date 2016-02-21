#ifndef UPDATERWINDOW_H
#define UPDATERWINDOW_H

#include <QWidget>
#include <QWinTaskbarButton>
#include <qprogressgroup.h>
#include "basedownloader.h"

namespace Ui {
class UpdaterWindow;
}

class UpdaterWindow : public QWidget
{
	Q_OBJECT

public:
	enum UpdateFlag {
		Database = 0x00,
		RecentlyUsed = 0x01,
		Emojis = 0x02,

		All = (RecentlyUsed | Emojis | Database)
	};
	Q_DECLARE_FLAGS(UpdateFlags, UpdateFlag)
	Q_FLAG(UpdateFlags)

	explicit UpdaterWindow(QWidget *parent = Q_NULLPTR);
	~UpdaterWindow();

protected:
	void showEvent(QShowEvent *ev) Q_DECL_OVERRIDE;

private slots:
	void initialize();

	void error(const QString &error, bool critical);

	void beginDownload(const QUrl &url);
	void downloadReady(QTemporaryFile *downloadFile);
	void updateDownloadProgress(qint64 value, qint64 max);
	void abortDownloaderDone();

	void on_buttonBox_rejected();

private:
	Ui::UpdaterWindow *ui;
	QProgressGroup *mainProgress;
	QWinTaskbarButton *taskButton;

	BaseDownloader *downloader;
	bool downloaderAborted;
	bool installerAborted;
};

#endif // UPDATERWINDOW_H
