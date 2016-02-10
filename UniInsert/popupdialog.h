#ifndef POPUPDIALOG_H
#define POPUPDIALOG_H

#include <QDialog>

class PopupDialog : public QDialog
{
	Q_OBJECT

	Q_PROPERTY(bool autoHide READ doesAutoHide WRITE setAutoHide)

public:
	explicit PopupDialog(bool isFixedSize);

	bool doesAutoHide() const;
	void setAutoHide(bool autoHide);

public slots:
	void popup();

signals:
	void didClose();

protected:
	bool event(QEvent *event) Q_DECL_OVERRIDE;
	void closeEvent(QCloseEvent *ev) Q_DECL_OVERRIDE;

private:
	bool autoHide;
};

#endif // POPUPDIALOG_H
