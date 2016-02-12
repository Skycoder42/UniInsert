#ifndef UNICODERMODELS_H
#define UNICODERMODELS_H

#include <QStandardItemModel>
#include <QAction>
#include <QAbstractItemView>
#include <QStyledItemDelegate>
#include "unicoder.h"
#include "databaseloader.h"

class SymbolListModel : public QStandardItemModel
{
public:
	SymbolListModel(QObject *parent = nullptr);

	void resetData(DatabaseLoader::SymbolInfoList symbolList);

	QAction *createCopyAction(QAbstractItemView *view) const;

	// QAbstractItemModel interface
	QStringList mimeTypes() const Q_DECL_OVERRIDE;
	QMimeData *mimeData(const QModelIndexList &indexes) const Q_DECL_OVERRIDE;

public slots:
	void activateItem(const QModelIndex &index) const;
	void copyItem(const QModelIndex &index) const;
};

class UnicodeDelegate : public QStyledItemDelegate
{
public:
	static QString displayCode(uint code);

	UnicodeDelegate(QObject *parent = nullptr);
	QString displayText(const QVariant &value, const QLocale &locale) const Q_DECL_OVERRIDE;
};

#endif // UNICODERMODELS_H
