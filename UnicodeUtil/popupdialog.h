#ifndef POPUPDIALOG_H
#define POPUPDIALOG_H

#include <QDialog>
class PopupController;
class QHotkey;

class PopupDialog : public QDialog
{
	Q_OBJECT

	Q_PROPERTY(bool autoHide READ doesAutoHide WRITE setAutoHide)

public:
	explicit PopupDialog(PopupController *controller, bool isFixedSize);

	bool doesAutoHide() const;
	void setAutoHide(bool autoHide);

public slots:
	void popup();

signals:
	void showInfo(uint code, bool allowGroups);

protected:
	void closeEvent(QCloseEvent *ev) Q_DECL_OVERRIDE;
	bool event(QEvent *ev) Q_DECL_OVERRIDE;

private:
	bool autoHide;
};

class PopupController : public QWidget
{
public:
	PopupController();
	QAction *createAction();
	QAction *getAction();
	PopupDialog *getDialog();

	void setHotkeyActive(bool active);
	void updateHotkey(const QKeySequence &keySequence);

protected:
	virtual QString actionName() const = 0;
	virtual QKeySequence defaultKeySequence() const = 0;
	virtual PopupDialog *createDialog() = 0;

private:
	QHotkey *hotkey;
	PopupDialog *dialog;
	QAction *action;
};

#endif // POPUPDIALOG_H
