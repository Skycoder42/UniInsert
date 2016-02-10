#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include <QSettings>

#define SETTINGS_CODE(code, defaultValue) \
	const QString SettingsDialog::code = QStringLiteral(#code);\
	const QVariant SettingsDialog::code ## Default = defaultValue;\


SETTINGS_CODE(useClip, true)
SETTINGS_CODE(allClip, false)
SETTINGS_CODE(autoHide, true)
SETTINGS_CODE(maxRecent, 42)

SettingsDialog::SettingsDialog(QWidget *parent) :
	QDialog(parent, Qt::WindowCloseButtonHint),
	ui(new Ui::SettingsDialog)
{
	ui->setupUi(this);
}

SettingsDialog::~SettingsDialog()
{
	delete ui;
}

void SettingsDialog::showSettings()
{
	QSettings settings;
	this->ui->useClipboardSendCheckBox->setChecked(settings.value(SettingsDialog::useClip, SettingsDialog::useClipDefault).toBool());
	this->ui->alwaysUseClipboardCheckBox->setChecked(settings.value(SettingsDialog::allClip, SettingsDialog::allClipDefault).toBool());
	this->ui->autoHideWindowsCheckBox->setChecked(settings.value(SettingsDialog::autoHide, SettingsDialog::autoHideDefault).toBool());
	this->ui->maximumRecentlyUsedItemsSpinBox->setValue(settings.value(SettingsDialog::maxRecent, SettingsDialog::maxRecentDefault).toInt());
	this->exec();
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
