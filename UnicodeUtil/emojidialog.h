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
	friend class EmojiController;

public:
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
	explicit EmojiDialog(PopupController *controller);
	~EmojiDialog();

	Ui::EmojiDialog *ui;
	QSignalMapper *addMapper;
	QSignalMapper *deleteMapper;
	QSignalMapper *pasteMapper;

	QWidget* tabContextWidget;
	QHash<QWidget*, SymbolListModel*> tabModels;
};

class EmojiController : public PopupController
{
protected:
	// PopupController interface
	QString actionName() const Q_DECL_OVERRIDE;
	QKeySequence defaultKeySequence() const Q_DECL_OVERRIDE;
	PopupDialog *createDialog() Q_DECL_OVERRIDE;
};

#endif // EMOJIDIALOG_H
