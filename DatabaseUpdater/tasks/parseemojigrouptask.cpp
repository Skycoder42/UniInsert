#include "parseemojigrouptask.h"
#include "global.h"
#include <QTextDocument>

#define EMOJI_ERROR(var) if(var == -1) {\
		this->engine->failure(emojiPediaError());\
		return false;\
	}

ParseEmojiGroupTask::ParseEmojiGroupTask(const QUrl &downloadUrl) :
	DatabaseTask(),
	url(downloadUrl)
{}

QUrl ParseEmojiGroupTask::downloadUrl() const
{
	return this->url;
}

QString ParseEmojiGroupTask::installText() const
{
	QString name = this->url.path();
	if(name.endsWith(QLatin1Char('/')))
		name.resize(name.size() - 1);
	name = name.mid(name.lastIndexOf(QLatin1Char('/')) + 1);
	return UpdateEngineCore::tr("Creating emoji group \"%1\"")
			.arg(name);
}

bool ParseEmojiGroupTask::execute(QSqlDatabase &newDB)
{
	QList<uint> emojis;

	// Parse the file!!!
	int start = this->downloadData.indexOf("<div class=\"content\">");
	EMOJI_ERROR(start);
	start = this->downloadData.indexOf("</span>", start);
	EMOJI_ERROR(start);
	start += 7;
	int finish = this->downloadData.indexOf("</h1>", start);
	EMOJI_ERROR(finish);

	QTextDocument tDoc;
	tDoc.setHtml(QString::fromUtf8(this->downloadData.mid(start, finish - start)));
	QString groupName = tDoc.toPlainText().simplified();

	start = this->downloadData.indexOf("<ul class=\"emoji-list\">", start);
	EMOJI_ERROR(start);
	finish = this->downloadData.indexOf("</ul>", start);
	EMOJI_ERROR(finish);

	int nextStart = start;
	while (nextStart < finish && nextStart != -1) {
		CHECK_ABORT
		int begin = this->downloadData.indexOf("<span class=\"emoji\">", nextStart);
		EMOJI_ERROR(begin);
		begin += 20;
		int end = this->downloadData.indexOf("</span>", begin);
		EMOJI_ERROR(end);

		QString emojiStr = QString::fromUtf8(this->downloadData.mid(begin, end - begin));
		if(emojiStr.size() == 1)
			emojis += emojiStr.at(0).unicode();
		else if(emojiStr.size() == 2 && emojiStr.at(0).isHighSurrogate())
			emojis +=  QChar::surrogateToUcs4(emojiStr.at(0).unicode(), emojiStr.at(1).unicode());

		nextStart = this->downloadData.indexOf("</li>", end);
		EMOJI_ERROR(nextStart);
		nextStart = this->downloadData.indexOf("<li>", nextStart);
	}

	if(!emojis.isEmpty()) {
		this->engine->updateInstallMax(emojis.size() + 1);

		QSqlQuery createGroupQuery(newDB);
		createGroupQuery.prepare(QStringLiteral("INSERT INTO EmojiGroups (Name) VALUES(:name)"));
		createGroupQuery.bindValue(QStringLiteral(":name"), groupName);
		TRY_EXEC(createGroupQuery)

				int groupID = createGroupQuery.lastInsertId().toInt();
		this->engine->updateInstallValue(1);

		for(int i = 0, max = emojis.size(); i < max; ++i) {
			CHECK_ABORT
					QSqlQuery insertSymbolQuery(newDB);
			insertSymbolQuery.prepare(QStringLiteral("INSERT INTO EmojiMapping (GroupID, EmojiID, SortHint) VALUES(:group, :emoji, :hint)"));
			insertSymbolQuery.bindValue(QStringLiteral(":group"), groupID);
			insertSymbolQuery.bindValue(QStringLiteral(":emoji"), emojis[i]);
			insertSymbolQuery.bindValue(QStringLiteral(":hint"), i + 1);
			TRY_EXEC(insertSymbolQuery)

					this->engine->updateInstallValue(i + 2);
		}
	}

	return true;
}
