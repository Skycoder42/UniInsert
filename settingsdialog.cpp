#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include <QSettings>
#include <QMessageBox>
#include "databaseloader.h"

#define SETTINGS_CODE(code, defaultValue) \
	const QString SettingsDialog::code = QStringLiteral(#code);\
	const QVariant SettingsDialog::code ## Default = defaultValue;\


SETTINGS_CODE(useClip, true)
SETTINGS_CODE(allClip, false)
SETTINGS_CODE(autoHide, true)
SETTINGS_CODE(maxRecent, 42)
SETTINGS_CODE(reset, false)

SettingsDialog::SettingsDialog(QWidget *parent) :
	QDialog(parent, Qt::WindowCloseButtonHint),
	ui(new Ui::SettingsDialog)
{
	ui->setupUi(this);
	SettingsDialog::loadSize(this);
}

SettingsDialog::~SettingsDialog()
{
	SettingsDialog::storeSize(this);
	delete ui;
}

void SettingsDialog::storeSize(QWidget *widget)
{
	QSettings settings;
	settings.beginGroup(QStringLiteral("gui"));
	settings.setValue(widget->objectName(), widget->size());
	settings.endGroup();
}

void SettingsDialog::loadSize(QWidget *widget)
{
	QSettings settings;
	settings.beginGroup(QStringLiteral("gui"));
	QSize size = settings.value(widget->objectName(), widget->size()).toSize();
	widget->resize(size);
	settings.endGroup();
}

void SettingsDialog::showSettings()
{
	this->raise();
	this->activateWindow();
	if(!this->isVisible()) {
		QSettings settings;
		this->ui->useClipboardSendCheckBox->setChecked(settings.value(SettingsDialog::useClip, SettingsDialog::useClipDefault).toBool());
		this->ui->alwaysUseClipboardCheckBox->setChecked(settings.value(SettingsDialog::allClip, SettingsDialog::allClipDefault).toBool());
		this->ui->autoHideWindowsCheckBox->setChecked(settings.value(SettingsDialog::autoHide, SettingsDialog::autoHideDefault).toBool());
		this->ui->maximumRecentlyUsedItemsSpinBox->setValue(settings.value(SettingsDialog::maxRecent, SettingsDialog::maxRecentDefault).toInt());
		this->exec();
	}
}

void SettingsDialog::accept()
{
	QSettings settings;
	settings.setValue(SettingsDialog::useClip, this->ui->useClipboardSendCheckBox->isChecked());
	settings.setValue(SettingsDialog::allClip, this->ui->alwaysUseClipboardCheckBox->isChecked());
	settings.setValue(SettingsDialog::autoHide, this->ui->autoHideWindowsCheckBox->isChecked());
	settings.setValue(SettingsDialog::maxRecent, this->ui->maximumRecentlyUsedItemsSpinBox->value());
	emit settingsChanged();
	this->QDialog::accept();
}

void SettingsDialog::showAboutDialog()
{
	QMessageBox box(NULL);
	box.setModal(true);
	box.setWindowTitle(tr("About"));
	QImage img = QApplication::windowIcon().pixmap(64, 64).toImage();
	img.invertPixels();
	box.setIconPixmap(QPixmap::fromImage(img));
	box.setText(tr("<b>About %1 — Version %2</b>")
				.arg(QApplication::applicationDisplayName())
				.arg(QApplication::applicationVersion()));
	box.setInformativeText(tr("<p>An application to easily insert unicode characters EVERYWHERE.</p>"
							  "<p>Author: Sykcoder Soft (<a href=\"https://github.com/Skycoder42\">Skycoder42</a>)</p>"
							  "<p>For Updates and further Information, check <a href=\"https://github.com/Skycoder42/UniInsert\">https://github.com/Skycoder42/UniInsert</a></p>"));
	box.setStandardButtons(QMessageBox::Ok);
	box.setDefaultButton(QMessageBox::Ok);
	box.exec();
}

void SettingsDialog::on_buttonBox_clicked(QAbstractButton *button)
{
	if(this->ui->buttonBox->standardButton(button) == QDialogButtonBox::RestoreDefaults) {
		if(QMessageBox::warning(this,
								tr("Warning"),
								tr("Do you really want to reset all settings and the database?\n"
								   "This will include custom emoji settings and recently used!\n\n"
								   "The application will be shut down!"),
								QMessageBox::Ok,
								QMessageBox::Abort)
		   == QMessageBox::Ok) {
			QSettings().setValue(SettingsDialog::reset, true);
			qApp->quit();
		}
	}
}

void SettingsDialog::on_resetRecentButton_clicked()
{
	if(QMessageBox::question(this,
							 tr("Reset Recently Used"),
							 tr("Do you really want to reset the list of recently used symbols?"))
	   == QMessageBox::Yes) {
		Unicoder::databaseLoader()->resetRecent();
	}
}
