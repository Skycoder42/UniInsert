#ifndef GETCODEDIALOG_H
#define GETCODEDIALOG_H

#include "popupdialog.h"

namespace Ui {
	class GetCodeDialog;
}

class GetCodeDialog : public PopupDialog//TODO add to recent button
{
	Q_OBJECT

public:
	explicit GetCodeDialog();
	~GetCodeDialog();

signals:
	void showBlock(int blockID);

protected:
	void showEvent(QShowEvent *event);

private slots:
	void on_symbolLineEdit_textChanged(const QString &text);
	void on_exploreGroupButton_clicked();

	void on_addRecentButton_clicked();

private:
	Ui::GetCodeDialog *ui;
};

#endif // GETCODEDIALOG_H
