#include "codeblockreader.h"
#include <QFile>
#include <QTextStream>
#include <QSettings>
#include "unicoder.h"

CodeBlockReader::CodeBlockReader(QObject *parent) :
	CodeBlockReader(QStringLiteral(":/blocks/blocks.csv"), parent)
{}

CodeBlockReader::CodeBlockReader(const QString &file, QObject *parent) :
	QObject(parent),
	dataModel(new QStandardItemModel(0, 3, this)),
	recents()
{
	this->dataModel->setHorizontalHeaderLabels({
												   tr("Block Name"),
												   tr("First Symbol"),
												   tr("Last Symbol")
											   });

	//load recently used
	for(QVariant code : QSettings().value(QStringLiteral("CodeBlockReader/recentSymbols")).toStringList()) {
		bool ok = false;
		uint rCode = code.toUInt(&ok);
		if(ok)
			this->recents += Unicoder::code32ToSymbol(rCode);
	}
	this->dataModel->appendRow({
								   new QStandardItem(tr("Recently Used")),
								   new QStandardItem(tr("-/-")),
								   new QStandardItem(tr("-/-"))
							   });

	QFile blocksFile(file);
	if(blocksFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QTextStream stream(&blocksFile);
		stream.setCodec("UTF-8");
		while(!stream.atEnd()) {
			QStringList parts = stream.readLine().split(QLatin1Char(';'));
			if(parts.size() < 3)
				continue;

			bool ok;
			int code;

			QStandardItem *beginItem = new QStandardItem(QLatin1String("U+") + parts.first());
			beginItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			code = parts.takeFirst().toUInt(&ok, 16);
			if(ok)
				beginItem->setData((uint)code);
			else {
				delete beginItem;
				continue;
			}

			QStandardItem *endItem = new QStandardItem(QLatin1String("U+") + parts.first());
			endItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			code = parts.takeFirst().toUInt(&ok, 16);
			if(ok)
				endItem->setData((uint)code);
			else {
				delete beginItem;
				delete endItem;
				continue;
			}

			QStandardItem *displayItem = new QStandardItem(parts.join(QLatin1Char(';')));
			displayItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

			this->dataModel->appendRow({
										   displayItem,
										   beginItem,
										   endItem
									   });
		}
		blocksFile.close();
	}
}

QAbstractItemModel *CodeBlockReader::blockModel() const
{
	return this->dataModel;
}

QStringList CodeBlockReader::createBlock(const QModelIndex &index) const
{
	if(index.isValid()) {
		if(index.row() == 0)//recently used
			return this->recents;

		uint begin = this->dataModel->item(index.row(), 1)->data().toUInt();
		uint end = this->dataModel->item(index.row(), 2)->data().toUInt();

		QStringList symbols;
		for(uint i = begin; i <= end; ++i)
			symbols += Unicoder::code32ToSymbol(i);
		return symbols;
	} else
		return QStringList();
}

QModelIndex CodeBlockReader::findBlock(uint code) const
{
	for(int i = 1, max = this->dataModel->rowCount(); i < max; ++i) {
		uint end = this->dataModel->item(i, 2)->data().toUInt();
		if(code <= end) {
			uint begin = this->dataModel->item(i, 1)->data().toUInt();
			if(code >= begin) {
				return this->dataModel->index(i, 0);
			}
		}
	}

	return QModelIndex();
}

void CodeBlockReader::updateRecent(const QString &symbol)
{
	this->recents.removeOne(symbol);
	this->recents.prepend(symbol);
	if(this->recents.size() > 42)
		this->recents.removeLast();

	//update settings
	QVariantList codeList;
	for(QString symbol : this->recents)
		codeList += Unicoder::symbolToCode32(symbol);
	QSettings().setValue(QStringLiteral("CodeBlockReader/recentSymbols"), codeList);
}
