#include "unicoder.h"
#include <QGlobalStatic>
#include <QApplication>
#include <QClipboard>
#include <QAbstractItemModel>
#include <QMimeData>
#include "codeblockreader.h"
#include "symbolinserter.h"

//all static "helper classes"
Q_GLOBAL_STATIC(SymbolInserter, symbolInserter)
Q_GLOBAL_STATIC(CodeBlockReader, codeBlockReader)

QString Unicoder::code32ToSymbol(uint code)
{
	QString text;
	if(QChar::requiresSurrogates(code)) {
		text.append(QChar(QChar::highSurrogate(code)));
		text.append(QChar(QChar::lowSurrogate(code)));
	} else
		text.append(QChar(code));
	return text;
}

QString Unicoder::code16ToSymbol(SurrogatePair code)
{
	QString text;
	if(QChar::isHighSurrogate(code.highCode)) {
		text.append(QChar(code.highCode));
		text.append(QChar(code.lowCode));
	} else
		text.append(QChar(code.lowCode));
	return text;
}

QString Unicoder::code16ToSymbol(ushort highSurrogate, ushort lowSurrogate) {
	return code16ToSymbol({highSurrogate, lowSurrogate});
}

QString Unicoder::code16ToSymbol(ushort code)
{
	return QString(QChar(code));
}

uint Unicoder::symbolToCode32(const QString &symbol)
{
	if(symbol.size() == 1)
		return symbol.at(0).unicode();
	else if(symbol.size() == 2 && symbol.at(0).isHighSurrogate())
		return QChar::surrogateToUcs4(symbol.at(0).unicode(), symbol.at(1).unicode());
	else
		return UINT_MAX;
}

Unicoder::SurrogatePair Unicoder::symbolToCode16(const QString &symbol)
{
	if(symbol.size() == 1)
		return {0, symbol.at(0).unicode()};
	else if(symbol.size() == 2 && symbol.at(0).isHighSurrogate())
		return {symbol.at(0).unicode(), symbol.at(1).unicode()};
	else
		return {USHRT_MAX, USHRT_MAX};
}

uint Unicoder::code16ToCode32(Unicoder::SurrogatePair code)
{
	if(code.isSurrogated())
		return QChar::surrogateToUcs4(code.highCode, code.lowCode);
	else
		return code.lowCode;
}

Unicoder::SurrogatePair Unicoder::code32ToCode16(uint code)
{
	if(QChar::requiresSurrogates(code))
		return {QChar::highSurrogate(code), QChar::lowSurrogate(code)};
	else
		return {0, code};
}

void Unicoder::sendSymbolInput(const QString &symbol)
{
	codeBlockReader->updateRecent(symbol);
	symbolInserter->insertSymbol(symbol);
}

void Unicoder::copySymbol(const QString &symbol)
{
	codeBlockReader->updateRecent(symbol);
	QApplication::clipboard()->setText(symbol);
}

CodeBlockReader *Unicoder::getCodeBlockReader()
{
	return codeBlockReader;
}

QString Unicoder::findSymbolBlock(const QString &symbol)
{
	return codeBlockReader->blockModel()->data(codeBlockReader->findBlock(symbol)).toString();
}

QString Unicoder::findSymbolBlock(Unicoder::SurrogatePair code)
{
	return codeBlockReader->blockModel()->data(codeBlockReader->findBlock(code)).toString();
}

QString Unicoder::findSymbolBlock(uint code)
{
	return codeBlockReader->blockModel()->data(codeBlockReader->findBlock(code)).toString();
}



bool Unicoder::SurrogatePair::isSurrogated() const
{
	return QChar::isSurrogate(this->highCode);
}



DragStringListModel::DragStringListModel(QObject *parent) :
	QStringListModel(parent)
{}

QAction *DragStringListModel::createCopyAction(QAbstractItemView *view) const
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

QStringList DragStringListModel::mimeTypes() const
{
	QStringList types = this->QStringListModel::mimeTypes();
	types.append(QStringLiteral("text/plain"));
	return types;
}

QMimeData *DragStringListModel::mimeData(const QModelIndexList &indexes) const
{
	QMimeData *data = this->QStringListModel::mimeData(indexes);
	if(data && indexes.size() == 1) {
		QString symbol = this->data(indexes.first(), Qt::DisplayRole).toString();
		Unicoder::getCodeBlockReader()->updateRecent(symbol);
		data->setText(symbol);
	}
	return data;
}

void DragStringListModel::activateItem(const QModelIndex &index) const
{
	if(index.isValid())
		Unicoder::sendSymbolInput(this->data(index, Qt::DisplayRole).toString());
}

void DragStringListModel::copyItem(const QModelIndex &index) const
{
	if(index.isValid())
		Unicoder::copySymbol(this->data(index, Qt::DisplayRole).toString());
}