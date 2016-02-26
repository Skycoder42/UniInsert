#include "resetdatabasedialog.h"
#include "ui_resetdatabasedialog.h"
#include <dialogmaster.h>
#include "settingsdialog.h"

ResetDatabaseDialog::ResetDatabaseDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::ResetDatabaseDialog),
	modeGroup(new QButtonGroup(this))
{
	ui->setupUi(this);
	SettingsDialog::loadSize(this);
	DialogMaster::masterDialog(this);

	this->modeGroup->addButton(this->ui->downloadButton);
	this->modeGroup->addButton(this->ui->restoreButton);
}

ResetDatabaseDialog::~ResetDatabaseDialog()
{
	delete ui;
}

bool ResetDatabaseDialog::tryReset(QWidget *parent)
{
	ResetDatabaseDialog dialog(parent);
	if(dialog.exec() == QDialog::Accepted)
		return true;
	else
		return false;
}
