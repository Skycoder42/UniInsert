#include "advancedsearchdialog.h"
#include "ui_advancedsearchdialog.h"
#include <QSqlQuery>
#include "unicoder.h"
#include "databaseloader.h"
#include "unicodermodels.h"
#include "settingsdialog.h"
#include "dialogmaster.h"

QModelIndex AdvancedSearchDialog::searchBlock(QWidget *parent, QAbstractItemModel *model)
{
	AdvancedSearchDialog dialog(model, parent);
	if(dialog.exec() == QDialog::Accepted)
		return dialog.selectedIndex;
	else
		return QModelIndex();
}

AdvancedSearchDialog::AdvancedSearchDialog(QAbstractItemModel *model, QWidget *parent) :
	QDialog(parent, Qt::WindowCloseButtonHint),
	ui(new Ui::AdvancedSearchDialog),
	proxyModel(new QSortFilterProxyModel(this)),
	mode(DatabaseLoader::Contains)
{
	ui->setupUi(this);
	SettingsDialog::loadSize(this);
	DialogMaster::masterDialog(this);
	this->setWindowTitle(tr("Advanced Block Search"));

	this->proxyModel->setSourceModel(model);
	this->proxyModel->setSortLocaleAware(true);
	this->proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
	this->proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
	this->proxyModel->setFilterKeyColumn(0);
	this->proxyModel->setFilterRole(Qt::DisplayRole);
	this->ui->treeView->setModel(this->proxyModel);

	this->ui->treeView->setItemDelegateForColumn(1, new UnicodeDelegate(false, this->ui->treeView));
	this->ui->treeView->setItemDelegateForColumn(2, new UnicodeDelegate(false, this->ui->treeView));
	this->ui->treeView->setColumnHidden(3, true);

	if(!this->ui->treeView->header()->restoreState(SettingsDialog::loadValue(this, QStringLiteral("blockHeaderState")).toByteArray())) {
		this->ui->treeView->resizeColumnToContents(0);
		this->ui->treeView->resizeColumnToContents(1);
		this->ui->treeView->sortByColumn(-1, Qt::AscendingOrder);
	}
}

AdvancedSearchDialog::~AdvancedSearchDialog()
{
	SettingsDialog::storeSize(this);
	SettingsDialog::storeValue(this,
							   QStringLiteral("blockHeaderState"),
							   this->ui->treeView->header()->saveState());
	delete ui;
}

void AdvancedSearchDialog::on_nameFilterLineEdit_textChanged(const QString &text)
{
	QString pattern = QRegExp::escape(text);
	pattern.replace(QStringLiteral("\\*"), QStringLiteral(".*"));
	pattern.replace(QStringLiteral("\\?"), QStringLiteral("."));
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
	this->selectedIndex = this->selectedIndex.sibling(this->selectedIndex.row(), 3);
	this->accept();
}
