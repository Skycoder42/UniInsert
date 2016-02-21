#include "unicodermodels.h"
#include <QAbstractItemModel>
#include <QMimeData>
#include <QSqlQuery>

const QString SymbolListModel::IndexMimeType = QStringLiteral("uniinsert/listindex");

SymbolListModel::SymbolListModel(QObject *parent, bool canDrop) :
	QSqlQueryModel(parent),
	isEmoji(canDrop)
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

void SymbolListModel::refresh()
{
	QSqlQuery query = this->query();
	this->setQuery(QSqlQuery());
	query.exec();
	this->setQuery(query);
}

QString SymbolListModel::getSymbol(const QModelIndex &index) const
{
	return this->data(index.sibling(index.row(), 0), Qt::DisplayRole).toString();
}

QVariant SymbolListModel::data(const QModelIndex &item, int role) const
{
	switch(role) {
	case Qt::DisplayRole:
		return Unicoder::code32ToSymbol(this->QSqlQueryModel::data(item, Qt::DisplayRole).toUInt());
	case Qt::EditRole:
		return this->QSqlQueryModel::data(item, Qt::DisplayRole);
	case Qt::ToolTipRole:
		return this->QSqlQueryModel::data(item.sibling(item.row(), 1), Qt::DisplayRole);
	default:
		return this->QSqlQueryModel::data(item, role);
	}
}

QStringList SymbolListModel::mimeTypes() const
{
	return {QStringLiteral("text/plain"), SymbolListModel::IndexMimeType};
}

QMimeData *SymbolListModel::mimeData(const QModelIndexList &indexes) const
{
	if(indexes.size() == 1) {
		QMimeData *data = new QMimeData();
		QString symbol = this->getSymbol(indexes.first());
		Unicoder::databaseLoader()->updateRecent(symbol);
		data->setText(symbol);
		data->setData(SymbolListModel::IndexMimeType, QByteArray::number(indexes.first().row()));
		return data;
	} else
		return Q_NULLPTR;
}

Qt::ItemFlags SymbolListModel::flags(const QModelIndex &index) const
{
	if(this->isEmoji && !index.isValid())
		return this->QSqlQueryModel::flags(index) | Qt::ItemIsDropEnabled;
	else
		return this->QSqlQueryModel::flags(index) | Qt::ItemIsDragEnabled;
}

Qt::DropActions SymbolListModel::supportedDropActions() const
{
	return this->isEmoji ? Qt::MoveAction : Qt::IgnoreAction;
}

Qt::DropActions SymbolListModel::supportedDragActions() const
{
	return this->isEmoji ? (Qt::CopyAction | Qt::MoveAction) : Qt::CopyAction;
}

bool SymbolListModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
	Q_UNUSED(action)
	if(this->isEmoji) {
		if(column != 0 ||
		   parent.isValid() ||
		   !data->hasFormat(SymbolListModel::IndexMimeType) ||
		   !this->property("groupID").isValid())
			return false;
		int oldRow = data->data(SymbolListModel::IndexMimeType).toInt();
		if(oldRow == row || row == oldRow + 1)
			return false;

		uint dragCode = this->index(oldRow, 0).data(Qt::EditRole).toUInt();
		uint targetCode = this->index(row, 0).data(Qt::EditRole).toUInt();
		if(Unicoder::databaseLoader()->moveEmoji(this->property("groupID").toInt(), dragCode, targetCode)) {
			this->refresh();
			return true;
		} else
			return false;
	} else
		return false;
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

void SymbolListModel::removeRecentItem(const QModelIndex &index)
{
	QString symbol = this->getSymbol(index);
	if(!symbol.isNull()) {
		Unicoder::databaseLoader()->removeRecent(Unicoder::symbolToCode32(symbol));
		this->refresh();
	}
}



QString UnicodeDelegate::displayCode(uint code)
{
	return QStringLiteral("U+%1").arg(code, 4, 16, QLatin1Char('0')).toUpper();
}

UnicodeDelegate::UnicodeDelegate(bool isPreview, QObject *parent) :
	QStyledItemDelegate(parent),
	isPreview(isPreview)
{}

QString UnicodeDelegate::displayText(const QVariant &value, const QLocale &locale) const
{
	Q_UNUSED(locale)
	bool ok = false;
	QString text;
	if(this->isPreview)
		text = Unicoder::code32ToSymbol(value.toUInt(&ok));
	else
		text = UnicodeDelegate::displayCode(value.toUInt(&ok));
	if(ok)
		return text;
	else
		return this->isPreview ? QString() : tr("-/-");
}

