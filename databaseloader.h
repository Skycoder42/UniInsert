#ifndef DATABASELOADER_H
#define DATABASELOADER_H

#include <QObject>
#include <QSqlDatabase>
#include "unicoder.h"

class DatabaseLoader : public QObject
{
	Q_OBJECT
public:
	static const QString dbName;

	explicit DatabaseLoader(QObject *parent = 0);
	~DatabaseLoader();

	//functions - per symbol
	inline QString nameForSymbol(const QString &symbol) const;
	inline QString nameForSymbol(Unicoder::SurrogatePair code) const;
	QString nameForSymbol(uint code);

private:
	QSqlDatabase mainDB;
};

QString DatabaseLoader::nameForSymbol(const QString &symbol) const
{
	return this->nameForSymbol(Unicoder::symbolToCode32(symbol));
}

QString DatabaseLoader::nameForSymbol(Unicoder::SurrogatePair code) const
{
	return this->nameForSymbol(Unicoder::code16ToSymbol(code));
}

#endif // DATABASELOADER_H
