#ifndef DATABASELOADER_H
#define DATABASELOADER_H

#include <QObject>
#include <QSqlDatabase>
#include <QMap>
#include <QAbstractItemModel>
#include "unicoder.h"

class DatabaseLoader : public QObject
{
	Q_OBJECT
public:
	typedef QPair<uint, uint> Range;
	typedef QPair<uint, QString> SymbolInfo;
	typedef QList<SymbolInfo> SymbolInfoList;

	enum {BlockModelDataRole = Qt::UserRole + 1};

	enum SearchFlag {
		Contains = 0x00,
		StartsWith = 0x01,
		EndsWith = 0x02,

		ExactMatch = (StartsWith | EndsWith)
	};
	Q_DECLARE_FLAGS(SearchFlags, SearchFlag)
	Q_FLAG(SearchFlags)

	static const QString DBName;
	static const Range InvalidRange;

	static bool isRangeValid(const Range &range);

	explicit DatabaseLoader(QObject *parent = 0);
	~DatabaseLoader();

	//per symbol
	inline QString nameForSymbol(const QString &symbol) const;
	inline QString nameForSymbol(Unicoder::SurrogatePair code) const;
	QString nameForSymbol(uint code) const;
	SymbolInfoList searchName(const QString &nameTerm, SearchFlags mode = Contains) const;
	QSqlQuery searchNameQuery(const QString &nameTerm, SearchFlags mode = Contains) const;
	QSqlQuery emptySearchQuery() const;

	//per block
	SymbolInfoList createBlock(int blockID) const;
	inline int findBlock(const QString &symbol) const;
	inline int findBlock(Unicoder::SurrogatePair code) const;
	int findBlock(uint code) const;
	QString blockName(int blockID) const;
	Range blockRange(int blockID) const;
	inline QString findBlockName(const QString &symbol) const;
	inline QString findBlockName(Unicoder::SurrogatePair code) const;
	QString findBlockName(uint code) const;
	QAbstractItemModel *createBlockModel(QObject *modelParent) const;

	//recent
	inline void updateRecent(const QString &symbol) const;
	inline void updateRecent(Unicoder::SurrogatePair code) const;
	void updateRecent(uint code) const;

	//emojis
	QMap<int, QString> listEmojiGroups() const;
	SymbolInfoList createEmojiGroup(int groupID) const;

private:/*functions*/
	static QString prepareSearch(QString term, SearchFlags flags);

private:
	QSqlDatabase mainDB;
};

QString DatabaseLoader::nameForSymbol(const QString &symbol) const
{
	return this->nameForSymbol(Unicoder::symbolToCode32(symbol));
}

QString DatabaseLoader::nameForSymbol(Unicoder::SurrogatePair code) const
{
	return this->nameForSymbol(Unicoder::code16ToCode32(code));
}

int DatabaseLoader::findBlock(const QString &symbol) const
{
	return this->findBlock(Unicoder::symbolToCode32(symbol));
}

int DatabaseLoader::findBlock(Unicoder::SurrogatePair code) const
{
	return this->findBlock(Unicoder::code16ToCode32(code));
}

QString DatabaseLoader::findBlockName(const QString &symbol) const
{
	return this->findBlockName(Unicoder::symbolToCode32(symbol));
}

QString DatabaseLoader::findBlockName(Unicoder::SurrogatePair code) const
{
	return this->findBlockName(Unicoder::code16ToCode32(code));
}

void DatabaseLoader::updateRecent(const QString &symbol) const
{
	this->updateRecent(Unicoder::symbolToCode32(symbol));
}

void DatabaseLoader::updateRecent(Unicoder::SurrogatePair code) const
{
	this->updateRecent(Unicoder::code16ToCode32(code));
}

#endif // DATABASELOADER_H
