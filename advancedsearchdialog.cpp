#include "advancedsearchdialog.h"
#include "ui_advancedsearchdialog.h"
#include "unicoder.h"
#include "databaseloader.h"

QModelIndex AdvancedSearchDialog::searchBlock(QWidget *parent, QAbstractItemModel *model)
{
	AdvancedSearchDialog dialog(model, parent);
	if(dialog.exec() == QDialog::Accepted)
		return dialog.selectedIndex;
	else
		return QModelIndex();
}

uint AdvancedSearchDialog::searchSymbol(QWidget *parent)
{

}

AdvancedSearchDialog::AdvancedSearchDialog(QAbstractItemModel *model, QWidget *parent) :
	QDialog(parent, Qt::WindowCloseButtonHint),
	ui(new Ui::AdvancedSearchDialog),
	proxyModel(new QSortFilterProxyModel(this)),
	mode(Contains)
{
	ui->setupUi(this);

	this->proxyModel->setSourceModel(model);
	this->proxyModel->setSortLocaleAware(true);
	this->proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
	this->proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
	this->proxyModel->setFilterKeyColumn(0);
	this->proxyModel->setFilterRole(Qt::DisplayRole);
	this->ui->treeView->setModel(this->proxyModel);

	this->ui->treeView->resizeColumnToContents(0);
	this->ui->treeView->resizeColumnToContents(1);
	this->ui->treeView->sortByColumn(-1, Qt::AscendingOrder);
}

AdvancedSearchDialog::~AdvancedSearchDialog()
{
	delete ui;
}

void AdvancedSearchDialog::on_nameFilterLineEdit_textChanged(const QString &text)
{
	QString pattern = QRegularExpression::escape(text);
	if(this->mode.testFlag(DatabaseLoader::StartsWith))
		pattern.prepend(QLatin1Char('^'));
	if(this->mode.testFlag(DatabaseLoader::EndsWith))
		pattern.append(QLatin1Char('$'));
	this->proxyModel->setFilterRegExp(pattern);
}

void AdvancedSearchDialog::on_filterModeComboBox_currentIndexChanged(int index)
{
	this->mode = (DatabaseLoader::SearchFlag)index;
	this->on_nameFilterLineEdit_textChanged(this->ui->nameFilterLineEdit->text());
}

void AdvancedSearchDialog::on_treeView_activated(const QModelIndex &index)
{
	this->selectedIndex = this->proxyModel->mapToSource(index);
	this->selectedIndex = this->selectedIndex.sibling(this->selectedIndex.row(), 0);
	this->accept();
}
