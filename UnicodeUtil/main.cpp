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

	SymbolSelectController *symbController = Q_NULLPTR;
	GetCodeController *codeController = Q_NULLPTR;
	EmojiController *emojiController = Q_NULLPTR;
	BlockSelectController *blockController = Q_NULLPTR;
	SettingsDialog *settingsDiag = Q_NULLPTR;

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

		symbController = new SymbolSelectController();
		codeController = new GetCodeController();
		emojiController = new EmojiController();
		blockController = new BlockSelectController();
		settingsDiag = new SettingsDialog();

		QMenu *trayMenu = new QMenu();
		trayIco->setContextMenu(trayMenu);
		QObject::connect(trayIco, &QSystemTrayIcon::destroyed, trayMenu, [=](){
			delete trayMenu;
		}, Qt::DirectConnection);

		trayMenu->addAction(symbController->createAction());
		trayMenu->addAction(codeController->createAction());
		trayMenu->addAction(emojiController->createAction());
		trayMenu->addAction(blockController->createAction());
		trayMenu->addSeparator();
		trayMenu->addAction(Global::tr("Settings"), settingsDiag, SLOT(showSettings()));
		trayMenu->addAction(Global::tr("About"), settingsDiag, SLOT(showAboutDialog()));
		trayMenu->addAction(Global::tr("About Qt"), qApp, SLOT(aboutQt()));
		trayMenu->addSeparator();
		trayMenu->addAction(Global::tr("Quit"), qApp, SLOT(quit()));

		QObject::connect(codeController->getDialog(), SIGNAL(showBlock(int)),
						 blockController->getDialog(), SLOT(showBlock(int)));
		QObject::connect(symbController->getDialog(), SIGNAL(showInfo(uint,bool)),
						 codeController->getDialog(), SLOT(showSymbolInfo(uint,bool)));
		QObject::connect(emojiController->getDialog(), SIGNAL(showInfo(uint,bool)),
						 codeController->getDialog(), SLOT(showSymbolInfo(uint,bool)));
		QObject::connect(blockController->getDialog(), SIGNAL(showInfo(uint,bool)),
						 codeController->getDialog(), SLOT(showSymbolInfo(uint,bool)));

		QObject::connect(qApp, &QApplication::aboutToQuit, [&](){
			trayIco->hide();
			symbController->deleteLater();
			codeController->deleteLater();
			emojiController->deleteLater();
			blockController->deleteLater();
			settingsDiag->deleteLater();
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
