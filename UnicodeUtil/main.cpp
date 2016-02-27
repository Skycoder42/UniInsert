#include <QApplication>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QHotkey>
#include <QResource>
#include <qsingleinstance.h>

#include "settingsdialog.h"
#include "symbolselectdialog.h"
#include "getcodedialog.h"
#include "emojidialog.h"
#include "blockselectdialog.h"

class Global {
	Q_DECLARE_TR_FUNCTIONS(Global)
};

static QSingleInstance *singleInstance;

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QApplication::setApplicationName(TARGET);
	QApplication::setApplicationVersion(VERSION);
	QApplication::setOrganizationName(COMPANY);
	QApplication::setApplicationDisplayName(DISPLAY_NAME);
	QApplication::setWindowIcon(QIcon(QStringLiteral(":/icons/main.ico")));
	QApplication::setQuitOnLastWindowClosed(false);

	QSingleInstance instance;

	QSystemTrayIcon *trayIco = Q_NULLPTR;

	SettingsDialog *settingsDiag = Q_NULLPTR;
	SymbolSelectDialog *symbDiag = Q_NULLPTR;
	GetCodeDialog *codeDiag = Q_NULLPTR;
	EmojiDialog *emojiDialog = Q_NULLPTR;
	BlockSelectDialog *blockDiag = Q_NULLPTR;

	instance.setStartupFunction([&]() -> int {
		::singleInstance = &instance;
		//check if reset was requested
		if(SETTINGS_VALUE(SettingsDialog::resetDatabase).toBool()){
			DatabaseLoader::reset();
			QSettings().setValue(SettingsDialog::resetDatabase, false);
		}
		if(SETTINGS_VALUE(SettingsDialog::reset).toBool())
			QSettings().clear();

		QResource::registerResource(QApplication::applicationDirPath() + QStringLiteral("/defaultDatabase.rcc"));

		trayIco = new QSystemTrayIcon(QApplication::windowIcon());
		trayIco->setToolTip(QApplication::applicationDisplayName());

		settingsDiag = new SettingsDialog();
		symbDiag = new SymbolSelectDialog();
		codeDiag = new GetCodeDialog();
		emojiDialog = new EmojiDialog();
		blockDiag = new BlockSelectDialog();

		QObject::connect(codeDiag, &GetCodeDialog::showBlock, blockDiag, &BlockSelectDialog::showBlock);
		settingsDiag->setProperty("instanceKey", instance.instanceID());

		QMenu *trayMenu = new QMenu();
		trayIco->setContextMenu(trayMenu);
		QObject::connect(trayIco, &QSystemTrayIcon::destroyed, trayMenu, [=](){
			delete trayMenu;
		}, Qt::DirectConnection);

		QAction *codeAction = trayMenu->addAction(Global::tr("Enter Code"), symbDiag, SLOT(popup()), QKeySequence("Ctrl+Meta+#"));
		QHotkey *codeHotkey = new QHotkey(codeAction->shortcut(), true, codeAction);
		QObject::connect(codeHotkey, &QHotkey::activated, codeAction, &QAction::trigger);

		QAction *symbolAction = trayMenu->addAction(Global::tr("Show symbol data"), codeDiag, SLOT(popup()), QKeySequence("Ctrl+Meta+*"));
		QHotkey *symbolHotkey = new QHotkey(symbolAction->shortcut(), true, symbolAction);
		QObject::connect(symbolHotkey, &QHotkey::activated, symbolAction, &QAction::trigger);

		QAction *emojiAction = trayMenu->addAction(Global::tr("Emojis"), emojiDialog, SLOT(popup()), QKeySequence("Ctrl+Meta+Ins"));
		QHotkey *emojiHotkey = new QHotkey(emojiAction->shortcut(), true, emojiAction);
		QObject::connect(emojiHotkey, &QHotkey::activated, emojiAction, &QAction::trigger);

		QAction *blockAction = trayMenu->addAction(Global::tr("Blocklist/Recently used"), blockDiag, SLOT(popup()), QKeySequence("Ctrl+Meta+Del"));
		QHotkey *blockHotkey = new QHotkey(blockAction->shortcut(), true, blockAction);
		QObject::connect(blockHotkey, &QHotkey::activated, blockAction, &QAction::trigger);

		trayMenu->addSeparator();
		trayMenu->addAction(Global::tr("Settings"), settingsDiag, SLOT(showSettings()));
		trayMenu->addAction(Global::tr("About"), settingsDiag, SLOT(showAboutDialog()));
		trayMenu->addAction(Global::tr("About Qt"), qApp, SLOT(aboutQt()));
		trayMenu->addSeparator();
		trayMenu->addAction(Global::tr("Quit"), qApp, SLOT(quit()));

		QObject::connect(qApp, &QApplication::aboutToQuit, [&](){
			trayIco->hide();
			settingsDiag->deleteLater();
			symbDiag->deleteLater();
			codeDiag->deleteLater();
			emojiDialog->deleteLater();
			blockDiag->deleteLater();
			trayIco->deleteLater();
		});

		trayIco->show();
		return 0;
	});

	return instance.singleExec();
}

QSingleInstance *Unicoder::singleInstance()
{
	return ::singleInstance;
}
