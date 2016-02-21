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

class DragLabel : public QLabel
{
public:
	DragLabel(QWidget *parent = Q_NULLPTR);
protected:
	// QWidget interface
	void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
	void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
	void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
private:
	QPoint dragStartPosition;
};

class SymbolSelectDialog : public PopupDialog
{
	Q_OBJECT

public:
	static const QRegularExpression unicodeRegex;

	explicit SymbolSelectDialog();
	~SymbolSelectDialog();

	static uint getSymbol(QWidget *parent);

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
	Ui::SymbolSelectDialog *ui;
	QRegularExpressionValidator *validator;	
	bool doInsert;
};

#endif // SYMBOLSELECTDIALOG_H
