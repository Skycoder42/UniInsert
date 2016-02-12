#include "unicodermodels.h"
#include <QAbstractItemModel>
#include <QMimeData>

SymbolListModel::SymbolListModel(QObject *parent) :
	QStandardItemModel(0, 1, parent)
{}

void SymbolListModel::resetData(DatabaseLoader::SymbolInfoList symbolList)
{
	this->clear();
	for(DatabaseLoader::SymbolInfo info : symbolList) {
		QStandardItem *item = new QStandardItem(Unicoder::code32ToSymbol(info.first));
		item->setToolTip(info.second);
		this->appendRow(item);
	}
}

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

QStringList SymbolListModel::mimeTypes() const
{
	QStringList types = this->QStandardItemModel::mimeTypes();
	types.append(QStringLiteral("text/plain"));
	return types;
}

QMimeData *SymbolListModel::mimeData(const QModelIndexList &indexes) const
{
	QMimeData *data = this->QStandardItemModel::mimeData(indexes);
	if(data && indexes.size() == 1) {
		QString symbol = this->data(indexes.first(), Qt::DisplayRole).toString();
		Unicoder::databaseLoader()->updateRecent(symbol);
		data->setText(symbol);
	}
	return data;
}

void SymbolListModel::activateItem(const QModelIndex &index) const
{
	if(index.isValid())
		Unicoder::sendSymbolInput(this->data(index, Qt::DisplayRole).toString());
}

void SymbolListModel::copyItem(const QModelIndex &index) const
{
	if(index.isValid())
		Unicoder::copySymbol(this->data(index, Qt::DisplayRole).toString());
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
	return UnicodeDelegate::displayCode(value.toUInt());
}

