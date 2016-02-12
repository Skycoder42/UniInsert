#include <QApplication>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QHotkey>

#include "settingsdialog.h"
#include "symbolselectdialog.h"
#include "getcodedialog.h"
#include "emojidialog.h"
#include "blockselectdialog.h"

class Global {
	Q_DECLARE_TR_FUNCTIONS(Global)
};

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QApplication::setApplicationName(QStringLiteral("UniInsert"));
	QApplication::setOrganizationName(QStringLiteral("Skycoder Soft"));
	QApplication::setApplicationDisplayName(Global::tr("Unicode Utility"));
	QApplication::setWindowIcon(QIcon(QStringLiteral(":/icons/main.ico")));
	QApplication::setQuitOnLastWindowClosed(false);

	QSystemTrayIcon trayIco(QApplication::windowIcon());
	trayIco.setToolTip(QApplication::applicationDisplayName());

	SettingsDialog settingsDiag;
	SymbolSelectDialog symbDiag;
	GetCodeDialog codeDiag;
	EmojiDialog emojiDialog;
	BlockSelectDialog blockDiag;

	QObject::connect(&codeDiag, &GetCodeDialog::showBlock, &blockDiag, &BlockSelectDialog::showBlock);

	QMenu *trayMenu = new QMenu();
	trayIco.setContextMenu(trayMenu);
	QObject::connect(&trayIco, &QSystemTrayIcon::destroyed, trayMenu, [&](){
		delete trayMenu;
	}, Qt::DirectConnection);

	QAction *codeAction = trayMenu->addAction(Global::tr("Enter Code"), &symbDiag, SLOT(popup()), QKeySequence("Ctrl+Meta+#"));
	QHotkey *codeHotkey = new QHotkey(codeAction->shortcut(), true, codeAction);
	QObject::connect(codeHotkey, &QHotkey::activated, codeAction, &QAction::trigger);

	QAction *symbolAction = trayMenu->addAction(Global::tr("Show symbol data"), &codeDiag, SLOT(popup()), QKeySequence("Ctrl+Meta+*"));
	QHotkey *symbolHotkey = new QHotkey(symbolAction->shortcut(), true, symbolAction);
	QObject::connect(symbolHotkey, &QHotkey::activated, symbolAction, &QAction::trigger);

	QAction *emojiAction = trayMenu->addAction(Global::tr("Emojis"), &emojiDialog, SLOT(popup()), QKeySequence("Ctrl+Meta+Ins"));
	QHotkey *emojiHotkey = new QHotkey(emojiAction->shortcut(), true, emojiAction);
	QObject::connect(emojiHotkey, &QHotkey::activated, emojiAction, &QAction::trigger);

	QAction *blockAction = trayMenu->addAction(Global::tr("Blocklist/Recently used"), &blockDiag, SLOT(popup()), QKeySequence("Ctrl+Meta+Del"));
	QHotkey *blockHotkey = new QHotkey(blockAction->shortcut(), true, blockAction);
	QObject::connect(blockHotkey, &QHotkey::activated, blockAction, &QAction::trigger);

	trayMenu->addSeparator();
	trayMenu->addAction(Global::tr("Settings"), &settingsDiag, SLOT(showSettings()));
	trayMenu->addAction(Global::tr("Quit"), qApp, SLOT(quit()));

	trayIco.show();
	return a.exec();
}
