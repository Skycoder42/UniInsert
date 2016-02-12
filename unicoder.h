#ifndef UNICODER_H
#define UNICODER_H

#include <QString>
#include <QPair>
#include <QStringListModel>
#include <QAction>
#include <QAbstractItemView>
#include <QStyledItemDelegate>
class DatabaseLoader;

class Unicoder
{
public:
	struct SurrogatePair
	{
		ushort highCode;
		ushort lowCode;

		bool isSurrogated() const;
	};

	static QString code32ToSymbol(uint code);
	static QString code16ToSymbol(SurrogatePair code);
	static inline QString code16ToSymbol(ushort highSurrogate, ushort lowSurrogate) {
		return Unicoder::code16ToSymbol({highSurrogate, lowSurrogate});
	}
	static QString code16ToSymbol(ushort code);

	static uint symbolToCode32(const QString &symbol);
	static SurrogatePair symbolToCode16(const QString &symbol);

	static uint code16ToCode32(SurrogatePair code);
	static SurrogatePair code32ToCode16(uint code);

	static void sendSymbolInput(const QString &symbol);
	static void copySymbol(const QString &symbol);

	static DatabaseLoader *databaseLoader();

private:
	Unicoder() = delete;
	Unicoder(const Unicoder& other) = delete;
};
class DragStringListModel : public QStringListModel
{
public:
	DragStringListModel(QObject *parent = nullptr);

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




#endif // UNICODER_H
