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
	friend class GetCodeController;

public slots:
	void showSymbolInfo(uint code, bool allowGroups);

signals:
	void showBlock(int blockID);

protected:
	void showEvent(QShowEvent *event);

private slots:
	void on_symbolLineEdit_textChanged(const QString &text);
	void on_exploreGroupButton_clicked();

	void on_addRecentButton_clicked();

private:
	explicit GetCodeDialog(PopupController *controller);
	~GetCodeDialog();

	Ui::GetCodeDialog *ui;
	AliasAction *aliasAction;
	bool infoShow;
};

class GetCodeController : public PopupController
{
protected:
	// PopupController interface
	QString actionName() const Q_DECL_OVERRIDE;
	QKeySequence defaultKeySequence() const Q_DECL_OVERRIDE;
	PopupDialog *createDialog() Q_DECL_OVERRIDE;
};

#endif // GETCODEDIALOG_H
