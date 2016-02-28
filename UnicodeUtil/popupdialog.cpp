#include "popupdialog.h"
#include <QEvent>
#include <QCloseEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QSettings>
#include <QAction>
#include <QHotkey>
#include "settingsdialog.h"
#include "dialogmaster.h"

PopupDialog::PopupDialog(PopupController *controller, bool isFixedSize) :
	QDialog(controller),
	autoHide(true)
{
	DialogMaster::masterDialog(this, isFixedSize, Qt::WindowStaysOnTopHint);
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



PopupController::PopupController() :
	QWidget(Q_NULLPTR),
	hotkey(new QHotkey(this)),
	dialog(Q_NULLPTR),
	action(Q_NULLPTR)
{}

QAction *PopupController::createAction(QObject *parent)
{
	Q_ASSERT(!this->dialog);
	this->dialog = this->createDialog();
	Q_ASSERT(this->dialog);
	QKeySequence shortcut = QKeySequence(SettingsDialog::loadValue(this->dialog,
																   QStringLiteral("hotkey"),
																   this->defaultKeySequence()
																   .toString(QKeySequence::PortableText))
										 .toString(),
										 QKeySequence::PortableText);
	this->hotkey->setShortcut(shortcut);
	connect(this->hotkey, &QHotkey::activated,
			this->dialog, &PopupDialog::popup);

	this->action = new QAction(parent);
	this->action->setText(this->actionName());
	connect(this->action, &QAction::triggered,
			this->dialog, &PopupDialog::popup);

	this->setHotkeyActive(true);
	return this->action;
}

QAction *PopupController::getAction()
{
	return this->action;
}

PopupDialog *PopupController::getDialog()
{
	return this->dialog;
}

void PopupController::setHotkeyActive(bool active)
{
	if(!this->hotkey->setRegistered(active)) {
		if(active) {
			DialogMaster::warning(Q_NULLPTR,
								  PopupDialog::tr("The hotkey <i>%1</i> for \"%2\" could not be registered. "
												  "It is propably already in use or a system-reserved key.")
								  .arg(this->hotkey->shortcut().toString(QKeySequence::NativeText))
								  .arg(this->actionName()),
								  PopupDialog::tr("Failed to register hotkey!"));
		} else {
			DialogMaster::warning(Q_NULLPTR,
								  PopupDialog::tr("The hotkey <i>%1</i> for \"%2\" could not be unregistered. "
												  "It propably failed to register in the first place.")
								  .arg(this->hotkey->shortcut().toString(QKeySequence::NativeText))
								  .arg(this->actionName()),
								  PopupDialog::tr("Failed to unregister hotkey!"));
		}
	} else
		this->action->setShortcut(active ? this->hotkey->shortcut() : QKeySequence());
}

void PopupController::updateHotkey(const QKeySequence &keySequence)
{
	Q_ASSERT(!this->hotkey->isRegistered());
	this->action->setShortcut(keySequence);
	SettingsDialog::storeValue(this->dialog,
							   QStringLiteral("hotkey"),
							   keySequence.toString(QKeySequence::PortableText));
}
