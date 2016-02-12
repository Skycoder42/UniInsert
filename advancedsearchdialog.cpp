#include "advancedsearchdialog.h"
#include "ui_advancedsearchdialog.h"
#include <QSqlQuery>
#include "unicoder.h"
#include "databaseloader.h"
#include "unicodermodels.h"

QModelIndex AdvancedSearchDialog::searchBlock(QWidget *parent, QAbstractItemModel *model)
{
	AdvancedSearchDialog dialog(model, parent);
	if(dialog.exec() == QDialog::Accepted)
		return dialog.selectedIndex;
	else
		return QModelIndex();
}

QString AdvancedSearchDialog::searchSymbol(QWidget *parent)
{
	AdvancedSearchDialog dialog(parent);
	if(dialog.exec() == QDialog::Accepted)
		return UnicodeDelegate::displayCode(dialog.selectedIndex.data().toUInt());
	else
		return QString();
}

AdvancedSearchDialog::AdvancedSearchDialog(QAbstractItemModel *model, QWidget *parent) :
	QDialog(parent, Qt::WindowCloseButtonHint),
	ui(new Ui::AdvancedSearchDialog),
	proxyModel(new QSortFilterProxyModel(this)),
	symbolModel(nullptr),
	mode(DatabaseLoader::Contains)
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

AdvancedSearchDialog::AdvancedSearchDialog(QWidget *parent) :
	QDialog(parent, Qt::WindowCloseButtonHint),
	ui(new Ui::AdvancedSearchDialog),
	proxyModel(new QSortFilterProxyModel(this)),
	symbolModel(new QSqlQueryModel(this)),
	mode(DatabaseLoader::Contains)
{
	ui->setupUi(this);

	this->symbolModel->setHeaderData(0, Qt::Horizontal, tr("Code"));
	this->symbolModel->setHeaderData(1, Qt::Horizontal, tr("Name"));
	this->symbolModel->setQuery(Unicoder::databaseLoader()->emptySearchQuery());

	this->proxyModel->setSourceModel(this->symbolModel);
	this->proxyModel->setSortLocaleAware(true);
	this->proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
	this->ui->treeView->setModel(this->proxyModel);

	this->ui->treeView->setItemDelegateForColumn(0, new UnicodeDelegate(this->ui->treeView));
	this->ui->treeView->sortByColumn(-1, Qt::AscendingOrder);
}

AdvancedSearchDialog::~AdvancedSearchDialog()
{
	delete ui;
}

void AdvancedSearchDialog::on_nameFilterLineEdit_textChanged(const QString &text)
{
	this->updateSearch(text, false);
}

void AdvancedSearchDialog::on_nameFilterLineEdit_returnPressed()
{
	this->updateSearch(this->ui->nameFilterLineEdit->text(), true);
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

void AdvancedSearchDialog::updateSearch(const QString &text, bool force)
{
	if(this->symbolModel) {
		if(!force && text.size() < 3)
			this->symbolModel->setQuery(Unicoder::databaseLoader()->emptySearchQuery());
		else {
			QString pattern = text;
			pattern.replace(QLatin1Char('*'), QLatin1Char('%'));
			pattern.replace(QLatin1Char('?'), QLatin1Char('_'));
			this->symbolModel->setQuery(Unicoder::databaseLoader()->searchNameQuery(pattern, this->mode));
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
