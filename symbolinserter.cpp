#include "symbolinserter.h"
#include <QApplication>
#include <QClipboard>
#include <qt_windows.h>
#include <QVector>
#include <QDebug>
#include <QMimeData>
#include <QTimer>
#include "settingsdialog.h"

SymbolInserter::SymbolInserter(QObject *parent) :
	QObject(parent)
{}

void SymbolInserter::insertSymbol(const QString &symbol)
{
	const char *method = Q_NULLPTR;
	if(SETTINGS_VALUE(SettingsDialog::allClip).toBool())
		method = "sendClipInput";
	else
		method = "sendInput";
	bool ok = QMetaObject::invokeMethod(this, method, Qt::QueuedConnection,
										Q_ARG(QString, symbol));
	Q_ASSERT(ok);
}

void SymbolInserter::sendInput(const QString &symbol)
{
	const uint len = symbol.size() * 2;
	QVector<INPUT> inputs(len);

	for(uint i = 0; i < len; i+=2) {
		INPUT input;
		ZeroMemory(&input, sizeof(INPUT));
		input.type = INPUT_KEYBOARD;
		input.ki.wScan = symbol[i/2].unicode();
		input.ki.dwFlags = KEYEVENTF_UNICODE;

		inputs[i] = input;
		input.ki.dwFlags |= KEYEVENTF_KEYUP;
		inputs[i+1] = input;
	}

	uint res = ::SendInput(len, inputs.data(), sizeof(INPUT));
	if(res != len) {
		qDebug() << "send data failed with error" << ::GetLastError();
		if(SETTINGS_VALUE(SettingsDialog::useClip).toBool())
			this->sendClipInput(symbol);
	}
}

void SymbolInserter::sendClipInput(const QString &symbol)
{
	QClipboard *clipboard = QApplication::clipboard();

	const QMimeData *oldMime = clipboard->mimeData();
	typedef QList<QPair<QString, QByteArray>> DataStore;
	DataStore oldData;
	for(QString format : oldMime->formats())
		oldData.append({format, oldMime->data(format)});

	clipboard->setText(symbol);

	if(this->sendPaste()) {
		QTimer::singleShot(500, this, [oldData](){
			QMimeData *restoreMime = new QMimeData();
			for(DataStore::const_iterator it = oldData.begin(), end = oldData.end(); it != end; ++it)
				restoreMime->setData(it->first, it->second);
			QApplication::clipboard()->setMimeData(restoreMime);
		});
	}
}

bool SymbolInserter::sendPaste()
{
	QVector<INPUT> inputs(4);
	WORD seq[2] = {VK_CONTROL, 'V'};

	for(uint i = 0; i < 2; ++i) {
		INPUT input;
		ZeroMemory(&input, sizeof(INPUT));
		input.type = INPUT_KEYBOARD;
		input.ki.wVk = seq[i];

		inputs[i] = input;
		input.ki.dwFlags = KEYEVENTF_KEYUP;
		inputs[3-i] = input;
	}

	uint res = ::SendInput(4, inputs.data(), sizeof(INPUT));
	if(res != 4) {
		qDebug() << "failed to send paste keys" << ::GetLastError();
		return false;
	} else
		return true;
}
