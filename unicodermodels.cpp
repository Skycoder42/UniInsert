#include "unicodermodels.h"
#include <QAbstractItemModel>
#include <QMimeData>

SymbolListModel::SymbolListModel(QObject *parent) :
	QSqlQueryModel(parent)
{}

QAction *SymbolListModel::createCopyAction(QAbstractItemView *view) const
{
	QAction *action = new QAction(QIcon(QStringLiteral(":/icons/copy.ico")),
								  tr("Copy Symbol"),
								  view);
	action->setToolTip("Copies the selected symbol into the clipboard");
	action->setShortcut(QKeySequence::Copy);
	action->setShortcutContext(Qt::WidgetWithChildrenShortcut);

	connect(action, &QAction::triggered, this, [this, view](){
		this->copyItem(view->currentIndex());
	});

	view->addAction(action);
	return action;
}

QVariant SymbolListModel::data(const QModelIndex &item, int role) const
{
	switch(role) {
	case Qt::DisplayRole:
		return Unicoder::code32ToSymbol(this->QSqlQueryModel::data(item, role).toUInt());
	case Qt::ToolTipRole:
		return this->QSqlQueryModel::data(item.sibling(item.row(), 1), Qt::DisplayRole);
	default:
		return this->QSqlQueryModel::data(item, role);
	}
}

QStringList SymbolListModel::mimeTypes() const
{
	QStringList types = this->QSqlQueryModel::mimeTypes();
	types.prepend(QStringLiteral("text/plain"));
	return types;
}

QMimeData *SymbolListModel::mimeData(const QModelIndexList &indexes) const
{
	QMimeData *data = this->QSqlQueryModel::mimeData(indexes);
	if(data && indexes.size() == 1) {
		QString symbol = this->getSymbol(indexes.first());
		Unicoder::databaseLoader()->updateRecent(symbol);
		data->setText(symbol);
	}
	return data;
}

Qt::ItemFlags SymbolListModel::flags(const QModelIndex &index) const
{
	return this->QSqlQueryModel::flags(index) | Qt::ItemIsDragEnabled;
}

void SymbolListModel::activateItem(const QModelIndex &index) const
{
	if(index.isValid())
		Unicoder::sendSymbolInput(this->getSymbol(index));
}

void SymbolListModel::copyItem(const QModelIndex &index) const
{
	if(index.isValid())
		Unicoder::copySymbol(this->getSymbol(index));
}

QString SymbolListModel::getSymbol(const QModelIndex &index) const
{
	return this->data(index.sibling(index.row(), 0), Qt::DisplayRole).toString();
}



QString UnicodeDelegate::displayCode(uint code)
{
	return QStringLiteral("U+%1").arg(code, 4, 16, QLatin1Char('0')).toUpper();
}

UnicodeDelegate::UnicodeDelegate(QObject *parent) :
	QStyledItemDelegate(parent)
{}

QString UnicodeDelegate::displayText(const QVariant &value, const QLocale &locale) const
{
	Q_UNUSED(locale)
	bool ok = false;
	QString text = UnicodeDelegate::displayCode(value.toUInt(&ok));
	if(ok)
		return text;
	else
		return tr("-/-");
}

