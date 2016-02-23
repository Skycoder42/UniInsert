#ifndef EMOJIDIALOG_H
#define EMOJIDIALOG_H

#include "popupdialog.h"
#include <QSignalMapper>
class SymbolListModel;

namespace Ui {
	class EmojiDialog;
}

class EmojiDialog : public PopupDialog
{
	Q_OBJECT

public:
	explicit EmojiDialog(QWidget *parent = 0);
	~EmojiDialog();

	bool eventFilter(QObject *, QEvent *event) Q_DECL_OVERRIDE;

signals:
	void updatePasteEnabled(bool canPaste, QPrivateSignal);

protected:
	void closeEvent(QCloseEvent *ev);

private slots:
	void addTriggered(QObject *model);
	void deleteTriggered(QObject *model);
	void pasteTriggered(QObject *model);

	void on_actionAdd_Emoji_Group_triggered();
	void on_actionRemove_Emoji_Group_triggered();

	void createTab(int groupID, const QString &groupName);
	void clipChange();

private:
	Ui::EmojiDialog *ui;
	QSignalMapper *addMapper;
	QSignalMapper *deleteMapper;
	QSignalMapper *pasteMapper;

	QWidget* tabContextWidget;
	QHash<QWidget*, SymbolListModel*> tabModels;
};

#endif // EMOJIDIALOG_H