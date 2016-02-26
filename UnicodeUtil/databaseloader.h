#ifndef DATABASELOADER_H
#define DATABASELOADER_H

#include <QObject>
#include <QSqlDatabase>
#include <QMap>
#include <QSqlQueryModel>
#include <QVersionNumber>
#include "unicoder.h"
class SymbolListModel;

class DatabaseLoader : public QObject
{
	Q_OBJECT
public:
	typedef QPair<uint, uint> Range;
	typedef QPair<int, QString> EmojiGroupInfo;

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

	QString dbPath() const;

	static void reset();

	//per symbol
	inline QString nameForSymbol(const QString &symbol) const;
	inline QString nameForSymbol(Unicoder::SurrogatePair code) const;
	QString nameForSymbol(uint code) const;
	QStringList symbolAliases(uint code) const;
	QSqlQueryModel *createSearchModel(QObject *modelParent, bool aliases) const;
	bool searchName(const QString &nameTerm, SearchFlags mode, bool aliases, QSqlQueryModel *model) const;
	void clearSearchModel(QSqlQueryModel *model, bool aliases) const;

	//per block
	SymbolListModel *createBlock(int blockID, QObject *modelParent) const;
	bool createBlock(int blockID, SymbolListModel *updateModel) const;
	inline int findBlock(const QString &symbol) const;
	inline int findBlock(Unicoder::SurrogatePair code) const;
	int findBlock(uint code) const;
	QString blockName(int blockID) const;
	Range blockRange(int blockID) const;
	inline QString findBlockName(const QString &symbol) const;
	inline QString findBlockName(Unicoder::SurrogatePair code) const;
	QString findBlockName(uint code) const;
	QSqlQueryModel *createBlockModel(QObject *modelParent) const;

	//recent
	inline void updateRecent(const QString &symbol);
	inline void updateRecent(Unicoder::SurrogatePair code);
	void updateRecent(uint code);
	void removeRecent(uint code);
	void resetRecent();

	//emojis
	QList<EmojiGroupInfo> listEmojiGroups() const;
	SymbolListModel *createEmojiGroupModel(int groupID, QObject *modelParent) const;
	bool createEmojiGroupModel(int groupID, SymbolListModel *updateModel) const;
	bool addEmoji(int groupID, uint code);
	bool removeEmoji(int groupID, uint code);
	bool moveEmoji(int groupID, uint code, uint before);
	int createEmojiGroup(const QString &name);
	bool deleteEmojiGroup(int groupID);
	void updateEmojiGroupOrder(const QList<int> &idOrder);

	//system
	QVersionNumber currentVersion() const;

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

void DatabaseLoader::updateRecent(const QString &symbol)
{
	this->updateRecent(Unicoder::symbolToCode32(symbol));
}

void DatabaseLoader::updateRecent(Unicoder::SurrogatePair code)
{
	this->updateRecent(Unicoder::code16ToCode32(code));
}

#endif // DATABASELOADER_H
