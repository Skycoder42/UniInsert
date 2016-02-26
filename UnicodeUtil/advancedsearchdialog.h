#ifndef ADVANCEDSEARCHDIALOG_H
#define ADVANCEDSEARCHDIALOG_H

#include <QDialog>
#include <QSortFilterProxyModel>
#include <QSqlQueryModel>
#include "databaseloader.h"

namespace Ui {
	class AdvancedSearchDialog;
}

class AdvancedSearchDialog : public QDialog
{
	Q_OBJECT

public:
	static QModelIndex searchBlock(QWidget *parent, QAbstractItemModel *model);
	static uint searchSymbol(QWidget *parent);

private slots:
	void on_nameFilterLineEdit_textChanged(const QString &text);
	void on_nameFilterLineEdit_returnPressed();
	void on_filterModeComboBox_currentIndexChanged(int index);
	void on_treeView_activated(const QModelIndex &index);
	void on_findAliasCheckBox_toggled(bool checked);

private:
	Ui::AdvancedSearchDialog *ui;
	QSortFilterProxyModel *proxyModel;
	QSqlQueryModel *symbolModel;
	DatabaseLoader::SearchFlags mode;

	QModelIndex selectedIndex;

	explicit AdvancedSearchDialog(QAbstractItemModel *model, QWidget *parent = Q_NULLPTR);
	explicit AdvancedSearchDialog(QWidget *parent = Q_NULLPTR);
	~AdvancedSearchDialog();

	void updateSearch(const QString &text, bool force, bool aliases);
};

#endif // ADVANCEDSEARCHDIALOG_H
