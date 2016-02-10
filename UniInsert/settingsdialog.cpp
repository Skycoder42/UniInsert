#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include <QSettings>

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
	this->ui->useClipboardSendCheckBox->setChecked(settings.value(QStringLiteral("useClip"), true).toBool());
	this->ui->alwaysUseClipboardCheckBox->setChecked(settings.value(QStringLiteral("allClip"), false).toBool());
	this->ui->autoHideWindowsCheckBox->setChecked(settings.value(QStringLiteral("autoHide"), true).toBool());
	this->exec();
}

void SettingsDialog::accept()
{
	QSettings settings;
	settings.setValue(QStringLiteral("useClip"), this->ui->useClipboardSendCheckBox->isChecked());
	settings.setValue(QStringLiteral("allClip"), this->ui->alwaysUseClipboardCheckBox->isChecked());
	settings.setValue(QStringLiteral("autoHide"), this->ui->autoHideWindowsCheckBox->isChecked());
	emit settingsChanged();
	this->QDialog::accept();
}
