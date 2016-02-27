#ifndef GETCODEDIALOG_H
#define GETCODEDIALOG_H

#include "popupdialog.h"

namespace Ui {
	class GetCodeDialog;
}

class AliasAction;
class GetCodeDialog : public PopupDialog
{
	Q_OBJECT

public:
	explicit GetCodeDialog();
	~GetCodeDialog();

	static void showCodeInfo(uint code, QWidget *parent, bool allowGroups = true);

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
	AliasAction *aliasAction;
	bool infoShow;
};

#endif // GETCODEDIALOG_H
