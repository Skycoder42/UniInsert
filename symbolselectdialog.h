#ifndef SYMBOLSELECTDIALOG_H
#define SYMBOLSELECTDIALOG_H

#include "popupdialog.h"
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QWidgetAction>
#include <QPointer>
#include <QLabel>

namespace Ui {
	class SymbolSelectDialog;
}

class SymbolSelectDialog : public PopupDialog
{
	Q_OBJECT

public:
	static const QRegularExpression unicodeRegex;

	explicit SymbolSelectDialog();
	~SymbolSelectDialog();

protected:
	void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;

private slots:
	void on_unicodeLineEdit_textChanged(const QString &text);
	void on_insertButton_clicked();
	void on_actionCopy_Symbol_triggered();
	void on_actionSearch_symbol_name_triggered();

private:/*functions*/
	uint calcUnicode(const QString &code);

private:
	class PreviewAction : public QWidgetAction
	{
	public:
		PreviewAction(QObject *parent);
		void setText(const QString &text, const QString &toolTip);

	protected:
		QWidget *createWidget(QWidget *parent) Q_DECL_OVERRIDE;

	private:
		QString text;
		QString toolTip;
	};

	Ui::SymbolSelectDialog *ui;
	QRegularExpressionValidator *validator;	
	PreviewAction *previewAction;
};

#endif // SYMBOLSELECTDIALOG_H
