#include "updaterwindow.h"
#include "ui_updaterwindow.h"
#include <QFile>
#include <progressbaradapter.h>
#include <wintaskbarprogressadapter.h>
#include <dialogmaster.h>
#include "global.h"

#include "tasks/emojigroupscantask.h"
#include "tasks/createdbstructuretask.h"

UpdaterWindow::UpdaterWindow(QWidget *parent) :
	QWidget(parent, Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint | Qt::MSWindowsFixedSizeDialogHint),
	ui(new Ui::UpdaterWindow),
	mainProgress(new QProgressGroup(this)),
	taskButton(new QWinTaskbarButton(this)),
	engine(new UpdateEngine(this)),
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

	connect(this->engine, &UpdateEngine::error,
			this, &UpdaterWindow::error);
	connect(this->engine, &UpdateEngine::log,
			this, &UpdaterWindow::log);

	connect(this->engine, &UpdateEngine::abortDone,
			qApp, &QApplication::quit);
	connect(this->engine, &UpdateEngine::engineDone,
			this, &UpdaterWindow::engineDone);

	connect(this->engine, &UpdateEngine::downloadMaxChanged,
			this->ui->allDownloadProgressBar, &QProgressBar::setMaximum);
	connect(this->engine, &UpdateEngine::beginDownload,
			this, &UpdaterWindow::beginDownload);
	connect(this->engine, &UpdateEngine::updateDownloadProgress,
			this, &UpdaterWindow::updateDownloadProgress);
	connect(this->engine, &UpdateEngine::downloadDone,
			this, &UpdaterWindow::downloadDone);

	connect(this->engine, &UpdateEngine::installMaxChanged,
			this, &UpdaterWindow::installMaxChanged);
	connect(this->engine, &UpdateEngine::beginInstall,
			this, &UpdaterWindow::beginInstall);
	connect(this->engine, &UpdateEngine::beginInstallProgress,
			this->ui->currentInstallProgressBar, &QProgressBar::setMaximum);
	connect(this->engine, &UpdateEngine::updateInstallProgress,
			this->ui->currentInstallProgressBar, &QProgressBar::setValue);
	connect(this->engine, &UpdateEngine::installDone,
			this, &UpdaterWindow::installDone);

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

	this->engine->addTask(new CreateDBStructureTask());
	this->engine->addTask(new EmojiGroupScanTask());
}

void UpdaterWindow::error(const QString &error)
{
	DialogMaster::critical(this, error);
	qApp->exit(EXIT_FAILURE);
}

void UpdaterWindow::log(const QString &error)
{
	this->softErrorList += tr(" • ") + error;
}

void UpdaterWindow::engineDone()
{
	this->ui->downloadLabel->setText(tr("Database update completed!"));
	this->ui->updateLabel->setText(tr("Database update completed!"));
	this->ui->currentDownloadProgressBar->setRange(0, 1);
	this->ui->currentDownloadProgressBar->setValue(1);
	this->ui->currentInstallProgressBar->setRange(0, 1);
	this->ui->currentInstallProgressBar->setValue(1);

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
	qDebug() << checked;//TODO
	qApp->quit();
}

void UpdaterWindow::beginDownload(const QString &text)
{
	this->ui->currentDownloadProgressBar->setValue(0);
	this->ui->currentDownloadProgressBar->setMaximum(1);
	this->ui->downloadLabel->setText(text + tr("…"));
}

void UpdaterWindow::updateDownloadProgress(qint64 value, qint64 max)
{
	this->ui->currentDownloadProgressBar->setMaximum(max);
	this->ui->currentDownloadProgressBar->setValue(value);
}

void UpdaterWindow::downloadDone()
{
	this->ui->allDownloadProgressBar->setValue(this->ui->allDownloadProgressBar->value() + 1);
	this->ui->downloadLabel->setText(tr("Download finished!"));
}

void UpdaterWindow::installMaxChanged(int newMax)
{
	this->mainProgress->setMaximum(newMax);
	this->mainProgress->setBarState(QProgressGroup::Active);
}

void UpdaterWindow::beginInstall(const QString &text)
{
	this->ui->currentInstallProgressBar->setValue(0);
	this->ui->currentInstallProgressBar->setMaximum(0);
	this->ui->updateLabel->setText(text + tr("…"));
}

void UpdaterWindow::installDone()
{
	this->mainProgress->setValue(this->mainProgress->value() + 1);
	this->ui->updateLabel->setText(tr("Waiting for the next component to finish downloading…"));
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
		this->engine->abort();
	}
}
