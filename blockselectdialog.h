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

public:
	explicit BlockSelectDialog();
	~BlockSelectDialog();

public slots:
	void showBlock(int blockID);

protected:
	void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;

private slots:
	void on_comboBox_currentIndexChanged(int index);
	void on_toolButton_clicked();

	void on_actionRemove_from_list_triggered();

private:
	Ui::BlockSelectDialog *ui;
	QAbstractItemModel *blockModel;
	SymbolListModel *displayModel;
	bool indexSet;
};

#endif // BLOCKSELECT࣫DIALOG_H
