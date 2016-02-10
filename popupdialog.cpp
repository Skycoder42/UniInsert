#include "popupdialog.h"
#include <QEvent>
#include <QCloseEvent>
#include "settingsdialog.h"

PopupDialog::PopupDialog(bool isFixedSize) :
	QDialog(new QWidget(nullptr), Qt::Dialog |
			Qt::CustomizeWindowHint |
			Qt::WindowTitleHint |
			Qt::WindowCloseButtonHint |
			Qt::WindowStaysOnTopHint |
			(isFixedSize ? Qt::MSWindowsFixedSizeDialogHint : Qt::Widget)),
	autoHide(true)
{
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
