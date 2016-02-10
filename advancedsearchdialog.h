#ifndef ADVANCEDSEARCHDIALOG_H
#define ADVANCEDSEARCHDIALOG_H

#include <QDialog>
#include <QSortFilterProxyModel>

namespace Ui {
	class AdvancedSearchDialog;
}

class AdvancedSearchDialog : public QDialog
{
	Q_OBJECT

public:
	static QModelIndex searchBlock(QWidget *parent, QAbstractItemModel *model);

private slots:
	void on_nameFilterLineEdit_textChanged(const QString &text);
	void on_filterModeComboBox_currentIndexChanged(int index);
	void on_treeView_activated(const QModelIndex &index);

private:
	enum SearchMode {
		Contains = 0,
		StartsWith,
		EndsWith,
		Wildcard
	};

	Ui::AdvancedSearchDialog *ui;
	QSortFilterProxyModel *proxyModel;
	SearchMode mode;

	QModelIndex selectedIndex;

	explicit AdvancedSearchDialog(QAbstractItemModel *model, QWidget *parent = nullptr);
	~AdvancedSearchDialog();
};

#endif // ADVANCEDSEARCHDIALOG_H
