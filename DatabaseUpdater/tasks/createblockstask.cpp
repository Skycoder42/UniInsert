#include "createblockstask.h"
#include "global.h"
#include <QRegularExpression>

QUrl CreateBlocksTask::downloadUrl() const
{
	return QUrl(QStringLiteral("ftp://ftp.unicode.org/Public/%1/ucd/Blocks.txt")
				.arg(ARG_UPDATE_VERSION));
}

QString CreateBlocksTask::installText() const
{
	return UpdateEngineCore::tr("Creating symbol blocks");
}

bool CreateBlocksTask::execute(QSqlDatabase &newDB)
{
	QRegularExpression regex(QStringLiteral(R"__(^([0-9A-F]{4,})\.\.([0-9A-F]{4,});\s*(.+)$)__"));
	regex.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
	regex.optimize();
	UpdateEngineCore::UniMatrix blockMatrix = this->engine->readDownload(this->downloadData, regex);

	uint oldMax = this->symbolMax(newDB);
	if(oldMax == -1)
		return false;
	uint newMax = 0;
	if(!blockMatrix.isEmpty())
		newMax = blockMatrix.last()[2].toUInt(Q_NULLPTR, 16);
	this->engine->updateInstallMax(blockMatrix.size() + qMax<uint>(newMax - oldMax, 0));

	for(int i = 0, max = blockMatrix.size(); i < max; ++i) {
		CHECK_ABORT
		const QStringList &list = blockMatrix[i];
		uint start = list[1].toUInt(Q_NULLPTR, 16);
		uint end = list[2].toUInt(Q_NULLPTR, 16);
		if(start > end)
			continue;

		QSqlQuery createBlockQuery(newDB);
		createBlockQuery.prepare(QStringLiteral("INSERT INTO Blocks (Name, BlockStart, BlockEnd) VALUES(:name, :start, :end)"));
		createBlockQuery.bindValue(":name", list[3]);
		createBlockQuery.bindValue(":start", start);
		createBlockQuery.bindValue(":end", end);
		TRY_EXEC(createBlockQuery);

		this->engine->updateInstallValue(i + 1);
	}

	if(newMax > oldMax) {
		int delta = newMax - oldMax;
		int rMax = blockMatrix.size();

		for(int i = 1; i <= delta; ++i) {
			CHECK_ABORT
			QSqlQuery insertUnnamedSymbolQuery(newDB);
			insertUnnamedSymbolQuery.prepare(QStringLiteral("INSERT INTO Symbols (Code) VALUES(:code)"));
			insertUnnamedSymbolQuery.bindValue(QStringLiteral(":code"), oldMax + i);
			TRY_EXEC(insertUnnamedSymbolQuery)
			this->engine->updateInstallValue(rMax + i);
		}
	}

	return true;
}
