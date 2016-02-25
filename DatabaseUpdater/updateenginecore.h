#ifndef UPDATEENGINECORE_H
#define UPDATEENGINECORE_H

#include <QString>
#include <QApplication>
class UpdateEngine;

class UpdateEngineCore
{
	Q_DECLARE_TR_FUNCTIONS(UpdateTask)

public:
	static const QString newDB;
	static const QString oldDB;

	virtual void failure(const QString &error) = 0;
	virtual void logError(const QString &error) = 0;

	virtual bool testAbort() const = 0;
};

#endif // UPDATEENGINECORE_H
