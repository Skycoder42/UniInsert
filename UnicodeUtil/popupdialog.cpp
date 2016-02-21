#include "popupdialog.h"
#include <QEvent>
#include <QCloseEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QSettings>
#include "settingsdialog.h"
#include "dialogmaster.h"

PopupDialog::PopupDialog(bool isFixedSize) :
	QDialog(new QWidget(Q_NULLPTR)),
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

void PopupDialog::closeEvent(QCloseEvent *ev)
{
	ev->accept();
	emit didClose();
}

bool PopupDialog::event(QEvent *ev)
{
	if(this->autoHide && ev->type() == QEvent::WindowDeactivate) {
		if(SETTINGS_VALUE(SettingsDialog::autoHide).toBool())
			this->close();
		else
			this->setWindowOpacity(SETTINGS_VALUE(SettingsDialog::transparency).toDouble());
	} else if(ev->type() == QEvent::WindowActivate) {
		this->setWindowOpacity(1.0);
	}

	return this->QDialog::event(ev);
}
