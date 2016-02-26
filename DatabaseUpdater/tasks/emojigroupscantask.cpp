#include "emojigroupscantask.h"
#include "global.h"
#include "parseemojigrouptask.h"

#define EMOJI_ERROR(var) if(var == -1) {\
		this->engine->failure(emojiPediaError());\
		return false;\
	}

EmojiGroupScanTask::EmojiGroupScanTask() :
	UpdateTask(),
	downloadUrls()
{}

QUrl EmojiGroupScanTask::downloadUrl() const
{
	return QUrl(QStringLiteral("http://emojipedia.org"));
}

QString EmojiGroupScanTask::installText() const
{
	return UpdateEngineCore::tr("Scanning for Emoji groups");
}

bool EmojiGroupScanTask::run()
{
	// Parse the file!!!
	int start = this->downloadData.indexOf("Categories");
	EMOJI_ERROR(start);
	start = this->downloadData.indexOf("<ul>", start);
	EMOJI_ERROR(start);
	int finish = this->downloadData.indexOf("</ul>", start);
	EMOJI_ERROR(finish);

	int nextStart = start;
	while (nextStart < finish && nextStart != -1) {
		CHECK_ABORT
		int begin = this->downloadData.indexOf("<li><a href=\"", nextStart);
		EMOJI_ERROR(begin);
		begin += 13;
		int end = this->downloadData.indexOf("\"", begin);
		EMOJI_ERROR(end);

		this->downloadUrls += QString::fromUtf8("http://emojipedia.org" + this->downloadData.mid(begin, end - begin));
		nextStart = this->downloadData.indexOf("</li>", end);
		EMOJI_ERROR(nextStart);
		nextStart = this->downloadData.indexOf("<li>", nextStart);
	}

	return true;
}

QList<UpdateTask *> EmojiGroupScanTask::newTasks() const
{
	QList<UpdateTask *> tasks;
	for(QUrl url : this->downloadUrls)
		tasks += new ParseEmojiGroupTask(url);
	return tasks;
}
