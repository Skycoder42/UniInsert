#ifndef BLOCKSELECTDIALOG_H
#define BLOCKSELECTDIALOG_H

#include "popupdialog.h"
#include "unicoder.h"
#include "unicodermodels.h"

namespace Ui {
	class BlockSelectDialog;
}

class BlockSelectDialog : public PopupDialog
{
	Q_OBJECT
	friend class BlockSelectController;

public slots:
	void showBlock(int blockID);

protected:
	void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;

private slots:
	void on_comboBox_currentIndexChanged(int index);
	void on_toolButton_clicked();

	void on_actionRemove_from_list_triggered();

private:
	explicit BlockSelectDialog(PopupController *controller);
	~BlockSelectDialog();

	Ui::BlockSelectDialog *ui;
	QAbstractItemModel *blockModel;
	SymbolListModel *displayModel;
	bool indexSet;
};

class BlockSelectController : public PopupController
{
protected:
	// PopupController interface
	QString actionName() const Q_DECL_OVERRIDE;
	QKeySequence defaultKeySequence() const Q_DECL_OVERRIDE;
	PopupDialog *createDialog() Q_DECL_OVERRIDE;
};

#endif // BLOCKSELECT࣫DIALOG_H
