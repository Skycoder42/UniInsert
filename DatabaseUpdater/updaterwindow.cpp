#include "updaterwindow.h"
#include "ui_updaterwindow.h"
#include <QFile>
#include <progressbaradapter.h>
#include <wintaskbarprogressadapter.h>
#include <dialogmaster.h>
#include "global.h"

UpdaterWindow::UpdaterWindow(QWidget *parent) :
	QWidget(parent, Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint | Qt::MSWindowsFixedSizeDialogHint),
	ui(new Ui::UpdaterWindow),
	mainProgress(new QProgressGroup(this)),
	taskButton(new QWinTaskbarButton(this)),
	downloader(new BaseDownloader(this)),
	updater(new DatabaseUpdater(this)),
	installMax(0),
	downloaderAborted(false),
	installerAborted(false),
	softErrorList()
{
	ui->setupUi(this);
	this->adjustSize();

	this->mainProgress->addAdapter(new ProgressBarAdapter(this->ui->allInstallProgressBar));
	this->mainProgress->setBarState(QProgressGroup::Indeterminated);
	this->ui->allInstallProgressBar->setTextVisible(true);

	connect(this->downloader, &BaseDownloader::beginDownload,
			this, &UpdaterWindow::beginDownload);
	connect(this->downloader, &BaseDownloader::downloadReady,
			this, &UpdaterWindow::downloadReady);
	connect(this->downloader, &BaseDownloader::updateDownloadProgress,
			this, &UpdaterWindow::updateDownloadProgress);
	connect(this->downloader, &BaseDownloader::error,
			this, &UpdaterWindow::error);
	connect(this->downloader, &BaseDownloader::emojiCountLoaded,
			this, &UpdaterWindow::emojiLoaded);
	connect(this->downloader, &BaseDownloader::abortDone,
			this, &UpdaterWindow::abortDownloaderDone);

	connect(this->updater, &DatabaseUpdater::beginInstall,
			this, &UpdaterWindow::beginInstall);
	connect(this->updater, &DatabaseUpdater::installReady,
			this, &UpdaterWindow::installReady);
	connect(this->updater, &DatabaseUpdater::updateInstallProgress,
			this, &UpdaterWindow::updateInstallProgress);
	connect(this->updater, &DatabaseUpdater::error,
			this, &UpdaterWindow::error);
	connect(this->updater, &DatabaseUpdater::abortDone,
			this, &UpdaterWindow::abortInstallDone);

	connect(this->downloader, &BaseDownloader::downloadReady,
			this->updater, &DatabaseUpdater::handleDownload,
			Qt::QueuedConnection);

	QMetaObject::invokeMethod(this, "initialize", Qt::QueuedConnection);
}

UpdaterWindow::~UpdaterWindow()
{
	delete ui;
}

void UpdaterWindow::showEvent(QShowEvent *ev)
{
	if(!this->taskButton->window()) {
		this->taskButton->setWindow(this->windowHandle());
		this->mainProgress->addAdapter(new WinTaskbarProgressAdapter(this->taskButton->progress()));
	}
	this->QWidget::showEvent(ev);
}

void UpdaterWindow::initialize()
{
	this->ui->titleLabel->setText(this->ui->titleLabel->text().arg(ARG_UPDATE_VERSION));

	this->ui->allDownloadProgressBar->setRange(0, this->downloader->getDownloadCount());
	this->ui->allDownloadProgressBar->setValue(0);

	this->installMax = this->updater->getInstallCount();
	this->mainProgress->setRange(0, this->installMax);
	this->mainProgress->setValue(0);
	this->mainProgress->setBarState(QProgressGroup::Active);

	QMetaObject::invokeMethod(this->updater, "startInstalling", Qt::QueuedConnection);
	this->downloader->startDownloading();
}

void UpdaterWindow::error(const QString &error, bool critical)
{
	if(critical) {
		DialogMaster::msgBox(this,
							 critical ? QMessageBox::Critical : QMessageBox::Warning,
							 error);
		qApp->exit(EXIT_FAILURE);
	} else
		this->softErrorList += QStringLiteral(" • ") + error;
}

void UpdaterWindow::beginDownload(const QUrl &url)
{
	this->ui->currentDownloadProgressBar->setValue(0);
	this->ui->currentDownloadProgressBar->setMaximum(1);
	this->ui->downloadLabel->setText(tr("Downloading: %1…").arg(url.toString()));
}

void UpdaterWindow::downloadReady(const QByteArray &)
{
	this->ui->allDownloadProgressBar->setValue(this->ui->allDownloadProgressBar->value() + 1);
	this->ui->downloadLabel->setText(tr("Download finished!"));
}

void UpdaterWindow::updateDownloadProgress(qint64 value, qint64 max)
{
	this->ui->currentDownloadProgressBar->setMaximum(max);
	this->ui->currentDownloadProgressBar->setValue(value);
}

void UpdaterWindow::emojiLoaded(int delta)
{
	this->ui->allDownloadProgressBar->setMaximum(this->ui->allDownloadProgressBar->maximum() + delta);
	this->installMax += delta;
	this->mainProgress->setMaximum(this->installMax);
}

void UpdaterWindow::abortDownloaderDone()
{
	this->downloaderAborted = true;
	if(this->downloaderAborted && this->installerAborted)
		qApp->quit();
}

void UpdaterWindow::beginInstall(const QString &text, int max)
{
	this->ui->updateLabel->setText(text + "…");
	this->ui->currentInstallProgressBar->setRange(0, max);
	this->ui->currentInstallProgressBar->setValue(0);
}

void UpdaterWindow::installReady()
{
	int val = this->mainProgress->value() + 1;
	this->mainProgress->setValue(val);
	if(val == this->installMax)
		this->completeInstall();
	else
		this->ui->updateLabel->setText(tr("Waiting for the next component to finish downloading…"));
}

void UpdaterWindow::updateInstallProgress(int value)
{
	this->ui->currentInstallProgressBar->setValue(value);
}

void UpdaterWindow::abortInstallDone()
{
	this->installerAborted = true;
	if(this->downloaderAborted && this->installerAborted)
		qApp->quit();
}

void UpdaterWindow::completeInstall()
{
	this->ui->downloadLabel->setText(tr("Database update completed!"));
	bool checked = true;
	DialogMaster::msgBox(this,
						 QMessageBox::Information,
						 tr("The Unicode database update to Version %1 finished successfully.\n\n"
							"Open the details to check for errors while transferring data from the "
							"old database.").arg(ARG_UPDATE_VERSION),
						 tr("Database update completed!"),
						 tr("Finished!"),
						 this->softErrorList.join(QLatin1Char('\n')),
						 &checked,
						 tr("Start Unicode Utility"));
	qDebug() << checked;
	qApp->quit();
}

void UpdaterWindow::on_buttonBox_rejected()
{
	if(DialogMaster::warning(this,
							 tr("Do you really want to abort the database update?\n"
								"The unicode database will be in the state it was before the update was started"),
							 tr("Abort Database Update?"),
							 QString(),
							 QMessageBox::Yes,
							 QMessageBox::No)
	   == QMessageBox::Yes) {
		this->ui->titleLabel->setEnabled(false);
		this->ui->groupBox->setEnabled(false);
		this->ui->groupBox_2->setEnabled(false);
		this->ui->buttonBox->setEnabled(false);
		this->setCursor(Qt::WaitCursor);
		this->downloader->abortDownloading();
		this->updater->abortInstalling();
	}
}
