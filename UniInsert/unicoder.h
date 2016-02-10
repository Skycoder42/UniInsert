#ifndef UNICODER_H
#define UNICODER_H

#include <QString>
#include <QPair>
#include <QStringListModel>
#include <QAction>
#include <QAbstractItemView>
class CodeBlockReader;

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
	static inline QString code16ToSymbol(ushort highSurrogate, ushort lowSurrogate);
	static QString code16ToSymbol(ushort code);

	static uint symbolToCode32(const QString &symbol);
	static SurrogatePair symbolToCode16(const QString &symbol);

	static uint code16ToCode32(SurrogatePair code);
	static SurrogatePair code32ToCode16(uint code);

	static void sendSymbolInput(const QString &symbol);
	static void copySymbol(const QString &symbol);

	static CodeBlockReader *getCodeBlockReader();
	static QString findSymbolBlock(const QString &symbol);
	static QString findSymbolBlock(Unicoder::SurrogatePair code);
	static QString findSymbolBlock(uint code);

private:
	Unicoder() = delete;
	Unicoder(const Unicoder& other) = delete;
};

class DragStringListModel : public QStringListModel
{
public:
	DragStringListModel(QObject *parent = NULL);

	QAction *createCopyAction(QAbstractItemView *view) const;

	// QAbstractItemModel interface
	QStringList mimeTypes() const Q_DECL_OVERRIDE;
	QMimeData *mimeData(const QModelIndexList &indexes) const Q_DECL_OVERRIDE;

public slots:
	void activateItem(const QModelIndex &index) const;
	void copyItem(const QModelIndex &index) const;
};

#endif // UNICODER_H
