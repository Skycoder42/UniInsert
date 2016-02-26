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
	dialog.ui->findAliasCheckBox->setVisible(false);
	if(dialog.exec() == QDialog::Accepted)
		return dialog.selectedIndex;
	else
		return QModelIndex();
}

uint AdvancedSearchDialog::searchSymbol(QWidget *parent)
{
	AdvancedSearchDialog dialog(parent);
	if(dialog.exec() == QDialog::Accepted)
		return dialog.selectedIndex.data().toUInt();
	else
		return UINT_MAX;
}

AdvancedSearchDialog::AdvancedSearchDialog(QAbstractItemModel *model, QWidget *parent) :
	QDialog(parent, Qt::WindowCloseButtonHint),
	ui(new Ui::AdvancedSearchDialog),
	proxyModel(new QSortFilterProxyModel(this)),
	symbolModel(Q_NULLPTR),
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
	this->ui->treeView->resizeColumnToContents(0);
	this->ui->treeView->resizeColumnToContents(1);
	this->ui->treeView->sortByColumn(-1, Qt::AscendingOrder);
}

AdvancedSearchDialog::AdvancedSearchDialog(QWidget *parent) :
	QDialog(parent, Qt::WindowCloseButtonHint),
	ui(new Ui::AdvancedSearchDialog),
	proxyModel(new QSortFilterProxyModel(this)),
	symbolModel(Unicoder::databaseLoader()->createSearchModel(this, true)),
	mode(DatabaseLoader::Contains)
{
	ui->setupUi(this);
	SettingsDialog::loadSize(this);
	DialogMaster::masterDialog(this);
	this->setWindowTitle(tr("Advanced Symbol Search"));

	this->proxyModel->setSourceModel(this->symbolModel);
	this->proxyModel->setSortLocaleAware(true);
	this->proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
	this->ui->treeView->setModel(this->proxyModel);

	this->ui->treeView->setItemDelegateForColumn(0, new UnicodeDelegate(true, this->ui->treeView));
	this->ui->treeView->setItemDelegateForColumn(1, new UnicodeDelegate(false, this->ui->treeView));
	this->ui->treeView->resizeColumnToContents(0);
	this->ui->treeView->sortByColumn(-1, Qt::AscendingOrder);
}

AdvancedSearchDialog::~AdvancedSearchDialog()
{
	SettingsDialog::storeSize(this);
	delete ui;
}

void AdvancedSearchDialog::on_nameFilterLineEdit_textChanged(const QString &text)
{
	this->updateSearch(text,
					   false,
					   this->ui->findAliasCheckBox->isChecked());
}

void AdvancedSearchDialog::on_nameFilterLineEdit_returnPressed()
{
	this->updateSearch(this->ui->nameFilterLineEdit->text(),
					   true,
					   this->ui->findAliasCheckBox->isChecked());
}

void AdvancedSearchDialog::on_filterModeComboBox_currentIndexChanged(int index)
{
	this->mode = (DatabaseLoader::SearchFlag)index;
	this->updateSearch(this->ui->nameFilterLineEdit->text(),
					   false,
					   this->ui->findAliasCheckBox->isChecked());
}

void AdvancedSearchDialog::on_treeView_activated(const QModelIndex &index)
{
	this->selectedIndex = this->proxyModel->mapToSource(index);
	if(this->symbolModel)
		this->selectedIndex = this->selectedIndex.sibling(this->selectedIndex.row(), 1);
	else
		this->selectedIndex = this->selectedIndex.sibling(this->selectedIndex.row(), 3);
	this->accept();
}

void AdvancedSearchDialog::on_findAliasCheckBox_clicked(bool checked)
{
	this->updateSearch(this->ui->nameFilterLineEdit->text(),
					   false,
					   checked);
}

void AdvancedSearchDialog::updateSearch(const QString &text, bool force, bool aliases)
{
	if(this->symbolModel) {
		if(!force && text.size() < 3)
			Unicoder::databaseLoader()->clearSearchModel(this->symbolModel, aliases);
		else {
			QString pattern = text;
			pattern.replace(QLatin1Char('*'), QLatin1Char('%'));
			pattern.replace(QLatin1Char('?'), QLatin1Char('_'));
			Unicoder::databaseLoader()->searchName(pattern, this->mode, aliases, this->symbolModel);
		}
	} else {
		QString pattern = QRegExp::escape(text);
		pattern.replace(QStringLiteral("\\*"), QStringLiteral(".*"));
		pattern.replace(QStringLiteral("\\?"), QStringLiteral("."));
		if(this->mode.testFlag(DatabaseLoader::StartsWith))
			pattern.prepend(QLatin1Char('^'));
		if(this->mode.testFlag(DatabaseLoader::EndsWith))
			pattern.append(QLatin1Char('$'));
		this->proxyModel->setFilterRegExp(pattern);
	}
}
