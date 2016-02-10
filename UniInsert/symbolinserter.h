#ifndef SYMBOLINSERTER_H
#define SYMBOLINSERTER_H

#include <QObject>

class SymbolInserter : public QObject
{
	Q_OBJECT
public:
	explicit SymbolInserter(QObject *parent = 0);

public slots:
	void insertSymbol(const QString &symbol);

private:
	Q_INVOKABLE void sendInput(const QString &symbol);

	Q_INVOKABLE void sendClipInput(const QString &symbol);
	bool sendPaste();
};

#endif // SYMBOLINSERTER_H
