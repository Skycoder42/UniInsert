#ifndef CODEBLOCKREADER_H
#define CODEBLOCKREADER_H

#include <QObject>
#include <QString>
#include <QStandardItemModel>
#include "unicoder.h"

class CodeBlockReader : public QObject
{
	Q_OBJECT

public:
	CodeBlockReader(QObject *parent = NULL);
	CodeBlockReader(const QString &file, QObject *parent = NULL);

	QAbstractItemModel *blockModel() const;

	inline QStringList createBlock(int index) const;
	QStringList createBlock(const QModelIndex &index) const;

	inline QModelIndex findBlock(const QString &symbol) const;
	inline QModelIndex findBlock(Unicoder::SurrogatePair code) const;
	QModelIndex findBlock(uint code) const;

	void updateRecent(const QString &symbol);
	inline void updateRecent(Unicoder::SurrogatePair code);
	inline void updateRecent(uint code);

private:
	QStandardItemModel *dataModel;
	QStringList recents;
};

QStringList CodeBlockReader::createBlock(int index) const
{
	return this->createBlock(this->dataModel->index(index, 0));
}

QModelIndex CodeBlockReader::findBlock(const QString &symbol) const
{
	return this->findBlock(Unicoder::symbolToCode32(symbol));
}

QModelIndex CodeBlockReader::findBlock(Unicoder::SurrogatePair code) const
{
	return this->findBlock(Unicoder::code16ToCode32(code));
}

void CodeBlockReader::updateRecent(Unicoder::SurrogatePair code)
{
	this->updateRecent(Unicoder::code16ToSymbol(code));
}

void CodeBlockReader::updateRecent(uint code)
{
	this->updateRecent(Unicoder::code32ToSymbol(code));
}

#endif // CODEBLOCKREADER_H
