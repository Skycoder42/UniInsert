#ifndef GETCODEDIALOG_H
#define GETCODEDIALOG_H

#include "popupdialog.h"
#include "codeblockreader.h"

namespace Ui {
	class GetCodeDialog;
}

class GetCodeDialog : public PopupDialog
{
	Q_OBJECT

public:
	explicit GetCodeDialog();
	~GetCodeDialog();

protected:
	void showEvent(QShowEvent *event);

private slots:
	void on_symbolLineEdit_textChanged(const QString &text);

private:
	Ui::GetCodeDialog *ui;
};

#endif // GETCODEDIALOG_H
