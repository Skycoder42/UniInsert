#include "popupdialog.h"
#include <QEvent>
#include <QCloseEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QSettings>
#include "settingsdialog.h"
#include "dialogmaster.h"

PopupDialog::PopupDialog(bool isFixedSize) :
	QDialog(new QWidget(nullptr)),
	autoHide(true)
{
	DialogMaster::masterDialog(this, isFixedSize, Qt::WindowStaysOnTopHint);

	connect(this, &PopupDialog::destroyed,
			this->parentWidget(), &QWidget::deleteLater);
}

bool PopupDialog::doesAutoHide() const
{
	return this->autoHide;
}

void PopupDialog::setAutoHide(bool autoHide)
{
	this->autoHide = autoHide;
}

void PopupDialog::popup()
{
	QRect geom = this->geometry();
	geom.moveCenter(QApplication::desktop()->screenGeometry(QCursor::pos()).center());
	this->move(geom.topLeft());
	this->show();
	this->raise();
	this->activateWindow();
}

bool PopupDialog::event(QEvent *event)
{
	if(this->autoHide && event->type() == QEvent::WindowDeactivate) {
		if(SETTINGS_VALUE(SettingsDialog::autoHide).toBool())
			this->close();
	}
	return this->QDialog::event(event);
}

void PopupDialog::closeEvent(QCloseEvent *ev)
{
	ev->accept();
	emit didClose();
}
