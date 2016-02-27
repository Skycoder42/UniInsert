#include "resetdatabasedialog.h"
#include "ui_resetdatabasedialog.h"
#include <QMovie>
#include <QFile>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QRegularExpression>
#include <QRegularExpressionMatchIterator>
#include <QStringListModel>
#include <QPushButton>
#include <QProcess>
#include <dialogmaster.h>
#include <qsingleinstance.h>
#include "settingsdialog.h"
#include "databaseloader.h"

template<typename... Args> struct SELECT {
	template<typename C, typename R>
	static constexpr auto OVERLOAD_OF( R (C::*pmf)(Args...) ) -> decltype(pmf) {
		return pmf;
	}
};

ResetDatabaseDialog::ResetDatabaseDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::ResetDatabaseDialog),
	modeGroup(new QButtonGroup(this)),
	nam(new QNetworkAccessManager(this))
{
	ui->setupUi(this);
	SettingsDialog::loadSize(this);
	DialogMaster::masterDialog(this);

	this->modeGroup->addButton(this->ui->downloadButton, 0);
	this->modeGroup->addButton(this->ui->restoreButton, 1);
	QPushButton *btn = this->ui->buttonBox->button(QDialogButtonBox::Ok);
	btn->setText(tr("Update"));
	btn->setEnabled(false);
	connect(this->modeGroup, SELECT<QAbstractButton*>::OVERLOAD_OF(&QButtonGroup::buttonClicked),
			btn, [btn](){
		btn->setEnabled(true);
	});

	QMovie *loaderMovie = new QMovie(QStringLiteral(":/icons/loading.gif"),
									 "gif",
									 this->ui->loadingLabel);
	this->ui->loadingLabel->setMovie(loaderMovie);
	loaderMovie->start();

	this->ui->installedDatabaseVersionLineEdit->setText(Unicoder::databaseLoader()->currentVersion().toString());

	QFile versionFile(QStringLiteral(":/database/version.txt"));
	if(versionFile.open(QIODevice::ReadOnly)) {
		QVersionNumber version = QVersionNumber::fromString(QString::fromUtf8(versionFile.readAll()));
		if(!version.isNull()) {
			this->ui->restoreVersionLineEdit->setText(version.toString());
			this->ui->restoreWidget->setEnabled(true);
		}
	}

	connect(this->nam, &QNetworkAccessManager::finished,
			this, &ResetDatabaseDialog::ftpListingReady);
	QNetworkRequest request(QStringLiteral("http://ftp.unicode.org/Public/zipped"));//trick to only get "valid" versions
	request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
	this->nam->get(request);
}

ResetDatabaseDialog::~ResetDatabaseDialog()
{
	delete ui;
}

bool ResetDatabaseDialog::tryReset(QWidget *parent, bool installOnly)
{
	ResetDatabaseDialog dialog(parent);
	if(installOnly) {
		dialog.ui->keepRecentlyUsedCheckBox->setChecked(false);
		dialog.ui->keepRecentlyUsedCheckBox->setEnabled(false);
		dialog.ui->keepEmojisButton->setChecked(false);
		dialog.ui->keepEmojisButton->setEnabled(false);
		dialog.ui->loadEmojisButton->setChecked(true);
		dialog.ui->loadEmojisButton->setEnabled(false);
	}
	if(dialog.exec() == QDialog::Accepted)
		return true;
	else
		return false;
}

void ResetDatabaseDialog::accept()
{
	if(DialogMaster::question(this, tr("Do you really want to reset/update the database?")) != QMessageBox::Yes)
	   return;

	if(this->modeGroup->checkedId() == 0) {
		QStringList arguments;
		arguments += DatabaseLoader::dbPath();
		arguments += this->ui->downloadVersionListView->currentIndex().data().toString();
		arguments += Unicoder::singleInstance()->instanceID();
		int flags = 0;
		if(this->ui->keepRecentlyUsedCheckBox->isChecked())
			flags |= 1;
		if(this->ui->keepEmojisButton->isChecked())
			flags |= 2;
		arguments += QString::number(flags);

#ifdef QT_NO_DEBUG
		if(QProcess::startDetached(QApplication::applicationDirPath() +
								   QStringLiteral("/DatabaseUpdater.exe"),
								   arguments))
			this->done(Accepted);
		else
			DialogMaster::critical(this, tr("Failed to start Database Updater!"));
#else
		qDebug() << arguments;
		this->done(Accepted);
#endif
	} else {
		QSettings().setValue(SettingsDialog::resetDatabase, true);
		Unicoder::singleInstance()->closeInstance();
		QProcess::startDetached(QApplication::applicationFilePath());
		this->done(Accepted);
	}
}

void ResetDatabaseDialog::ftpListingReady(QNetworkReply *reply)
{
	if(reply->error() == QNetworkReply::NoError) {
		QStringList versions;

		QRegularExpression regex(QStringLiteral(R"__(<a href="([^"]+)\/">[^<>]*<\/a>)__"));
		QRegularExpressionMatchIterator it = regex.globalMatch(QString::fromUtf8(reply->readAll()));
		while(it.hasNext()) {
			QString str = it.next().captured(1);
			QVersionNumber number = QVersionNumber::fromString(str);
			if(!number.isNull())
				versions += str;
		}

		if(!versions.isEmpty()) {
			versions.removeDuplicates();
			QStringListModel *model = new QStringListModel(versions, this->ui->downloadVersionListView);
			this->ui->downloadVersionListView->setModel(model);

			QModelIndex lastIndex(model->index(versions.size() - 1));
			this->ui->downloadVersionListView->setCurrentIndex(lastIndex);
//			this->ui->downloadVersionListView->selectionModel()->select(lastIndex, QItemSelectionModel::SelectCurrent);
			this->ui->downloadVersionListView->scrollTo(lastIndex);
			this->ui->downloadWidget->setEnabled(true);
		}
	} else {
		DialogMaster::msgBox(this,
							 QMessageBox::Warning,
							 tr("Failed to download version list from unicode.org"),
							 QString(),
							 QString(),
							 tr("Error-Code: %1\n\n%2")
							 .arg(reply->error())
							 .arg(reply->errorString()));
	}

	this->ui->loadingLabel->setVisible(false);
	reply->deleteLater();
}
