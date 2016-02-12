#include "unicoder.h"
#include <QGlobalStatic>
#include <QApplication>
#include <QClipboard>
#include "databaseloader.h"
#include "symbolinserter.h"

//all static "helper classes"
Q_GLOBAL_STATIC(SymbolInserter, symbolInserter)
Q_GLOBAL_STATIC(DatabaseLoader, databaseLoader)

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
	::databaseLoader->updateRecent(symbol);
	symbolInserter->insertSymbol(symbol);
}

void Unicoder::copySymbol(const QString &symbol)
{
	::databaseLoader->updateRecent(symbol);
	QApplication::clipboard()->setText(symbol);
}

DatabaseLoader *Unicoder::databaseLoader()
{
	return ::databaseLoader;
}



bool Unicoder::SurrogatePair::isSurrogated() const
{
	return QChar::isSurrogate(this->highCode);
}
