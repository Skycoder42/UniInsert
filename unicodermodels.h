#ifndef UNICODERMODELS_H
#define UNICODERMODELS_H

#include <QSqlQueryModel>
#include <QAction>
#include <QAbstractItemView>
#include <QStyledItemDelegate>
#include "unicoder.h"
#include "databaseloader.h"

class SymbolListModel : public QSqlQueryModel
{
public:
	SymbolListModel(QObject *parent = nullptr);

	QAction *createCopyAction(QAbstractItemView *view) const;

	// QAbstractItemModel interface
	QVariant data(const QModelIndex &item, int role) const;
	QStringList mimeTypes() const Q_DECL_OVERRIDE;
	QMimeData *mimeData(const QModelIndexList &indexes) const Q_DECL_OVERRIDE;
	Qt::ItemFlags flags(const QModelIndex &index) const;

public slots:
	void activateItem(const QModelIndex &index) const;
	void copyItem(const QModelIndex &index) const;

private:
	QString getSymbol(const QModelIndex &index) const;
};

class UnicodeDelegate : public QStyledItemDelegate
{
public:
	static QString displayCode(uint code);

	UnicodeDelegate(QObject *parent = nullptr);
	QString displayText(const QVariant &value, const QLocale &locale) const Q_DECL_OVERRIDE;
};

#endif // UNICODERMODELS_H
