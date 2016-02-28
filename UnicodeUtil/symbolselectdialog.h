#ifndef SYMBOLSELECTDIALOG_H
#define SYMBOLSELECTDIALOG_H

#include "popupdialog.h"
#include <QRegularExpression>
#include <QLabel>
#include <QSortFilterProxyModel>
#include <QSqlQueryModel>
#include <QLineEdit>
#include "databaseloader.h"

namespace Ui {
	class SymbolSelectDialog;
}

class ExtendedMenuLineEdit : public QLineEdit
{
public:
	ExtendedMenuLineEdit(QWidget *parent = Q_NULLPTR);
protected:
	void contextMenuEvent(QContextMenuEvent *ev) Q_DECL_OVERRIDE;
};

class DragLabel : public QLabel
{
public:
	DragLabel(QDialog *parent = Q_NULLPTR);
protected:
	// QWidget interface
	void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
	void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
	void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
	void mouseDoubleClickEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
private:
	QPoint dragStartPosition;
};

class SymbolSelectDialog : public PopupDialog
{
	Q_OBJECT
	friend class DragLabel;

public:
	static const QRegularExpression unicodeRegex;

	explicit SymbolSelectDialog();
	~SymbolSelectDialog();

	static uint getSymbol(QWidget *parent);

protected:
	void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;

private slots:
	void on_unicodeLineEdit_textEdited(const QString &text);
	void on_unicodeLineEdit_returnPressed();
	void on_filterModeComboBox_currentIndexChanged(int index);
	void on_findAliasCheckBox_toggled(bool);
	void on_searchTreeView_activated(const QModelIndex &index);
	void searchIndexChanged(const QModelIndex &current, const QModelIndex &);

	void on_actionCopy_Symbol_triggered();
	void on_actionShow_Symbol_Info_triggered();
	void on_actionCopy_Symbol_Name_triggered();
	void on_actionCopy_symbol_unicode_codepoint_triggered();
	void on_actionCopy_symbol_HTML_code_triggered();
	void on_actionEnter_unicode_codepoint_triggered();
	void on_actionEnter_HTML_code_triggered();
	void on_actionShow_searchterm_help_triggered();

private:/*functions*/
	uint calcUnicode(const QString &code);
	void updateSearch(const QString &text, bool force);

	void updateSymbol();

private:
	Ui::SymbolSelectDialog *ui;
	uint currentCode;
	bool doInsert;

	bool isSearchMode;
	QSortFilterProxyModel *proxyModel;
	QSqlQueryModel *symbolModel;
	DatabaseLoader::SearchFlags mode;
};

#endif // SYMBOLSELECTDIALOG_H
