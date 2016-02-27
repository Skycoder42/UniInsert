#ifndef UNICODER_H
#define UNICODER_H

#include <QString>
#include <QPair>
#include <QApplication>
class DatabaseLoader;

class Unicoder
{
	Q_DECLARE_TR_FUNCTIONS(Unicoder)

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

	static QString singleInstanceKey();

private:
	Unicoder() = delete;
	Unicoder(const Unicoder& other) = delete;
};

#endif // UNICODER_H
