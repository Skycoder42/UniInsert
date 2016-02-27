#ifndef WINDOWCONTROLLER_H
#define WINDOWCONTROLLER_H

#include <QWidget>
#include <QMenu>
#include <QStack>
#include "symbolselectdialog.h"
#include "getcodedialog.h"
#include "emojidialog.h"
#include "blockselectdialog.h"

class WindowController : public QWidget
{
	Q_OBJECT
public:
	explicit WindowController();

	QMenu *createMenu() const;

private slots:
	void showSymb();
	void showCode();
	void showEmoji();
	void showBlock();

	void windowFocusLost();
	void windowFocusGained();

private:
	SymbolSelectDialog *symbDiag;
	GetCodeDialog *codeDiag;
	EmojiDialog *emojiDialog;
	BlockSelectDialog *blockDiag;

	QStack<PopupDialog*> windowStack;
	bool inHandle;
};

#endif // WINDOWCONTROLLER_H
