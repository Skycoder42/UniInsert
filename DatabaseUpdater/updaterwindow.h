#ifndef UPDATERWINDOW_H
#define UPDATERWINDOW_H

#include <QWidget>
#include <QWinTaskbarButton>
#include <qprogressgroup.h>
#include "updateengine.h"

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

	//general
	void error(const QString &error);
	void log(const QString &error);

	void engineDone();

	//download
	void beginDownload(const QString &text);
	void updateDownloadProgress(qint64 value, qint64 max);
	void downloadDone();

	//install
	void installMaxChanged(int newMax);
	void beginInstall(const QString &text);
	void installDone();

	void on_buttonBox_rejected();

private:
	Ui::UpdaterWindow *ui;
	QProgressGroup *mainProgress;
	QWinTaskbarButton *taskButton;

	UpdateEngine *engine;

	int installMax;
	bool downloaderAborted;
	bool installerAborted;

	QStringList softErrorList;
};

#endif // UPDATERWINDOW_H
