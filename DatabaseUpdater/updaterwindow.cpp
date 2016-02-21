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

	UpdateFlags flags = ARG_UPDATE_MODE;

	this->ui->allDownloadProgressBar->setRange(0, this->downloader->getDownloadCount());
	this->ui->allDownloadProgressBar->setValue(0);

	int installCount = 42;
	if(flags.testFlag(RecentlyUsed))
		installCount += 1;
	if(flags.testFlag(Emojis))
		installCount += 7;
	this->mainProgress->setRange(0, installCount);
	this->mainProgress->setValue(0);
	this->mainProgress->setBarState(QProgressGroup::Active);

	this->downloader->startDownloading();
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

void UpdaterWindow::downloadReady(QTemporaryFile *downloadFile)
{
	downloadFile->deleteLater();
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
