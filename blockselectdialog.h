#ifndef BLOCKSELECTDIALOG_H
#define BLOCKSELECTDIALOG_H

#include "popupdialog.h"
#include "unicoder.h"

namespace Ui {
	class BlockSelectDialog;
}

class BlockSelectDialog : public PopupDialog
{
	Q_OBJECT

public:
	explicit BlockSelectDialog();
	~BlockSelectDialog();

protected:
	void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;

private slots:
	void on_comboBox_currentIndexChanged(int index);
	void on_toolButton_clicked();

private:
	Ui::BlockSelectDialog *ui;
	QAbstractItemModel *blockModel;
	DragStringListModel *displayModel;
};

#endif // BLOCKSELECT࣫DIALOG_H
