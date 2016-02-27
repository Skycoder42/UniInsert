#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include <QSettings>
#include <QDir>
#include "dialogmaster.h"
#include "databaseloader.h"
#include "resetdatabasedialog.h"

#define SETTINGS_CODE(code, defaultValue) \
	const QString SettingsDialog::code = QStringLiteral(#code);\
	const QVariant SettingsDialog::code ## Default = defaultValue;\


SETTINGS_CODE(useClip, true)
SETTINGS_CODE(allClip, false)
SETTINGS_CODE(autoHide, true)
SETTINGS_CODE(transparency, 0.5)
SETTINGS_CODE(maxRecent, 42)
SETTINGS_CODE(autoStart, true)
SETTINGS_CODE(reset, false)
SETTINGS_CODE(resetDatabase, false)

SettingsDialog::SettingsDialog(QWidget *parent) :
	QDialog(parent, Qt::WindowCloseButtonHint),
	ui(new Ui::SettingsDialog)
{
	ui->setupUi(this);
	SettingsDialog::loadSize(this);
	DialogMaster::masterDialog(this, true);

	this->ui->buttonBox->button(QDialogButtonBox::Reset)->setText(tr("Reset Database"));

	//make shure autostart is set as expected
	this->updateAutostart(SETTINGS_VALUE(SettingsDialog::autoStart).toBool());
}

SettingsDialog::~SettingsDialog()
{
	SettingsDialog::storeSize(this);
	delete ui;
}

void SettingsDialog::storeSize(QWidget *widget)
{
	SettingsDialog::storeValue(widget, QStringLiteral("size"), widget->size());
}

void SettingsDialog::loadSize(QWidget *widget)
{
	QSize size = SettingsDialog::loadValue(widget, QStringLiteral("size"), widget->size()).toSize();
	widget->resize(size);
}

void SettingsDialog::storeValue(QWidget *widget, const QString &key, const QVariant &value)
{
	QSettings settings;
	settings.beginGroup(QStringLiteral("gui/%1").arg(widget->objectName()));
	settings.setValue(key, value);
	settings.endGroup();
}

QVariant SettingsDialog::loadValue(QWidget *widget, const QString &key, const QVariant &defaultValue)
{
	QSettings settings;
	settings.beginGroup(QStringLiteral("gui/%1").arg(widget->objectName()));
	QVariant value = settings.value(key, defaultValue);
	settings.endGroup();
	return value;
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
		this->ui->inactiveWindowTransparencySlider->setValue(settings.value(SettingsDialog::transparency, SettingsDialog::transparencyDefault).toDouble() * 100);
		this->ui->maximumRecentlyUsedItemsSpinBox->setValue(settings.value(SettingsDialog::maxRecent, SettingsDialog::maxRecentDefault).toInt());
		this->ui->startWithWindowsCheckBox->setChecked(settings.value(SettingsDialog::autoStart, SettingsDialog::autoStartDefault).toBool());
		this->exec();
	}
}

void SettingsDialog::accept()
{
	QSettings settings;
	settings.setValue(SettingsDialog::useClip, this->ui->useClipboardSendCheckBox->isChecked());
	settings.setValue(SettingsDialog::allClip, this->ui->alwaysUseClipboardCheckBox->isChecked());
	settings.setValue(SettingsDialog::autoHide, this->ui->autoHideWindowsCheckBox->isChecked());
	settings.setValue(SettingsDialog::transparency, this->ui->inactiveWindowTransparencySlider->value() / 100.);
	settings.setValue(SettingsDialog::maxRecent, this->ui->maximumRecentlyUsedItemsSpinBox->value());
	settings.setValue(SettingsDialog::autoStart, this->ui->startWithWindowsCheckBox->isChecked());
	this->updateAutostart(this->ui->startWithWindowsCheckBox->isChecked());
	emit settingsChanged();
	this->QDialog::accept();
}

void SettingsDialog::showAboutDialog()
{
	QImage img = QApplication::windowIcon().pixmap(64, 64).toImage();
	img.invertPixels();
	DialogMaster::msgBox(Q_NULLPTR,
						 QPixmap::fromImage(img),
						 tr("<p>An application to easily insert unicode characters EVERYWHERE.</p>"
							"<p>Author: Sykcoder Soft (<a href=\"https://github.com/Skycoder42\">Skycoder42</a>)</p>"
							"<p>For Updates and further Information, check <a href=\"https://github.com/Skycoder42/UniInsert\">https://github.com/Skycoder42/UniInsert</a></p>"),
						 tr("About %1 — Version %2")
						 .arg(QApplication::applicationDisplayName())
						 .arg(QApplication::applicationVersion()),
						 tr("About"));
}

void SettingsDialog::on_buttonBox_clicked(QAbstractButton *button)
{
	switch(this->ui->buttonBox->standardButton(button)) {
	case QDialogButtonBox::Reset:
		if(ResetDatabaseDialog::tryReset(this))
			qApp->quit();
		break;
	case QDialogButtonBox::RestoreDefaults:
		if(DialogMaster::question(this,
								  tr("Only the settings will be resetted. The database, "
									 "recently used symbols and emojis will stay unchanged.\n\n"
									 "The application will quit if you accept."),
								  tr("Reset Settings?"))
		   == QMessageBox::Yes) {
			QSettings().setValue(SettingsDialog::reset, true);
			qApp->quit();
		}
		break;
	}
}

void SettingsDialog::on_resetRecentButton_clicked()
{
	if(DialogMaster::question(this,
							  tr("Do you really want to reset the list of recently used symbols?"),
							  tr("Reset Recently Used"))
	   == QMessageBox::Yes) {
		Unicoder::databaseLoader()->resetRecent();
	}
}
void SettingsDialog::updateAutostart(bool on)
{
	QSettings regEdit(QStringLiteral("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"), QSettings::NativeFormat);
	if(on)
		regEdit.setValue(QApplication::applicationName(), QDir::toNativeSeparators(QApplication::applicationFilePath()));
	else
		regEdit.remove(QApplication::applicationName());
}
