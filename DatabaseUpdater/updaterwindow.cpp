#include "updaterwindow.h"
#include "ui_updaterwindow.h"
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
	installerAborted(false)
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

	this->downloader->startDownloading();
	QMetaObject::invokeMethod(this->updater, "startInstalling", Qt::QueuedConnection);
}

void UpdaterWindow::error(const QString &error, bool critical)
{
	DialogMaster::msgBox(this,
						 critical ? QMessageBox::Critical : QMessageBox::Warning,
						 error);
	if(critical)
		qApp->exit(EXIT_FAILURE);
}

void UpdaterWindow::beginDownload(const QUrl &url)
{
	this->ui->currentDownloadProgressBar->setValue(0);
	this->ui->currentDownloadProgressBar->setMaximum(1);
	this->ui->downloadLabel->setText(tr("Downloading: %1…").arg(url.toString()));
}

void UpdaterWindow::downloadReady(QTemporaryFile *)
{
	this->ui->allDownloadProgressBar->setValue(this->ui->allDownloadProgressBar->value() + 1);
	this->ui->downloadLabel->setText(tr("Download finished!"));
}

void UpdaterWindow::updateDownloadProgress(qint64 value, qint64 max)
{
	this->ui->currentDownloadProgressBar->setMaximum(max);
	this->ui->currentDownloadProgressBar->setValue(value);
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
	this->ui->downloadLabel->setText(tr("Installation finished!"));
	if(val == this->installMax) {
		DialogMaster::information(this,
								  tr("Database update completed!"));
		qApp->quit();
	}
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
	}
}
