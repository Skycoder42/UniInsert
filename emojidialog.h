#ifndef EMOJIDIALOG_H
#define EMOJIDIALOG_H

#include "popupdialog.h"

namespace Ui {
	class EmojiDialog;
}

class EmojiDialog : public PopupDialog
{
	Q_OBJECT

public:
	explicit EmojiDialog(QWidget *parent = 0);
	~EmojiDialog();

private:
	Ui::EmojiDialog *ui;
};

#endif // EMOJIDIALOG_H
