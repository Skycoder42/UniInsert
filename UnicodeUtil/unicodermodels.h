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
	static const QString IndexMimeType;

	SymbolListModel(QObject *parent = Q_NULLPTR, bool isEmoji = false);

	QAction *createCopyAction(QAbstractItemView *view) const;

	void refresh();
	QString getSymbol(const QModelIndex &index) const;

	// QAbstractItemModel interface
	QVariant data(const QModelIndex &item, int role) const Q_DECL_OVERRIDE;
	QStringList mimeTypes() const Q_DECL_OVERRIDE;
	QMimeData *mimeData(const QModelIndexList &indexes) const Q_DECL_OVERRIDE;
	Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
	Qt::DropActions supportedDropActions() const Q_DECL_OVERRIDE;
	Qt::DropActions supportedDragActions() const Q_DECL_OVERRIDE;
	bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) Q_DECL_OVERRIDE;

public slots:
	void activateItem(const QModelIndex &index) const;
	void copyItem(const QModelIndex &index) const;
	void removeRecentItem(const QModelIndex &index);

private:

	bool isEmoji;
};

class UnicodeDelegate : public QStyledItemDelegate
{
public:
	static QString displayCode(uint code);

	UnicodeDelegate(bool isPreview, QObject *parent = Q_NULLPTR);
	QString displayText(const QVariant &value, const QLocale &locale) const Q_DECL_OVERRIDE;
private:
	bool isPreview;
};

#endif // UNICODERMODELS_H
