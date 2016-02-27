#include "windowcontroller.h"
#include <QHotkey>
#include "settingsdialog.h"

WindowController::WindowController() :
	QWidget(parent),
	symbDiag(new SymbolSelectDialog(this)),
	codeDiag(new GetCodeDialog(this)),
	emojiDialog(new EmojiDialog(this)),
	blockDiag(new BlockSelectDialog(this)),
	windowStack(),
	inHandle(false)
{
	QObject::connect(codeDiag, &GetCodeDialog::showBlock, blockDiag, &BlockSelectDialog::showBlock);
}

QMenu *WindowController::createMenu() const
{
	QMenu *trayMenu = new QMenu();

	QAction *codeAction = trayMenu->addAction(tr("Enter Code"),
											  this,
											  SLOT(showSymb()),
											  QKeySequence("Ctrl+Meta+#"));
	QHotkey *codeHotkey = new QHotkey(codeAction->shortcut(), true, this);
	QObject::connect(codeHotkey, &QHotkey::activated, codeAction, &QAction::trigger);

	QAction *symbolAction = trayMenu->addAction(tr("Show symbol data"),
												this,
												SLOT(showCode()),
												QKeySequence("Ctrl+Meta+*"));
	QHotkey *symbolHotkey = new QHotkey(symbolAction->shortcut(), true, this);
	QObject::connect(symbolHotkey, &QHotkey::activated, symbolAction, &QAction::trigger);

	QAction *emojiAction = trayMenu->addAction(tr("Emojis"),
											   this,
											   SLOT(showEmoji()),
											   QKeySequence("Ctrl+Meta+Ins"));
	QHotkey *emojiHotkey = new QHotkey(emojiAction->shortcut(), true, this);
	QObject::connect(emojiHotkey, &QHotkey::activated, emojiAction, &QAction::trigger);

	QAction *blockAction = trayMenu->addAction(tr("Blocklist/Recently used"),
											   this,
											   SLOT(showBlock()),
											   QKeySequence("Ctrl+Meta+Del"));
	QHotkey *blockHotkey = new QHotkey(blockAction->shortcut(), true, this);
	QObject::connect(blockHotkey, &QHotkey::activated, blockAction, &QAction::trigger);

	return trayMenu;
}

void WindowController::showSymb()
{
	while(!this->windowStack.isEmpty())
		this->windowStack.pop()->close();
}

void WindowController::showCode()
{

}

void WindowController::showEmoji()
{

}

void WindowController::showBlock()
{

}

void WindowController::windowFocusLost()
{
	if(this->inHandle)
		return;
	Q_ASSERT(dynamic_cast<PopupDialog*>(QObject::sender()));
	Q_ASSERT(!this->windowStack.isEmpty());

	PopupDialog *popup = static_cast<PopupDialog*>(QObject::sender());
	if(popup == this->windowStack.top()) {
		this->inHandle = true;
		if(SETTINGS_VALUE(SettingsDialog::autoHide).toBool()) {
			while(!this->windowStack.isEmpty())
				this->windowStack.pop()->close();
		} else {
			double opac = SETTINGS_VALUE(SettingsDialog::transparency).toDouble();
			for(int i = 0, max = this->windowStack.size(); i < max; ++i)
				this->windowStack[i]->setWindowOpacity(opac);
		}
		this->inHandle = false;
	}
}

void WindowController::windowFocusGained()
{
	if(this->inHandle)
		return;
	Q_ASSERT(dynamic_cast<PopupDialog*>(QObject::sender()));
	Q_ASSERT(!this->windowStack.isEmpty());

	PopupDialog *popup = static_cast<PopupDialog*>(QObject::sender());
	if(popup == this->windowStack.top()) {
		this->inHandle = true;
		for(int i = 0, max = this->windowStack.size(); i < max; ++i)
			this->windowStack[i]->setWindowOpacity(1.0);
		this->inHandle = false;
	}
}
