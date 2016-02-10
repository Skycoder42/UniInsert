#include "emojidialog.h"
#include "ui_emojidialog.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include <QListView>
#include "unicoder.h"

EmojiDialog::EmojiDialog(QWidget *parent) :
	PopupDialog(parent),
	ui(new Ui::EmojiDialog)
{
	ui->setupUi(this);

	QFile file(QStringLiteral(":/blocks/emoji.json"));
	file.open(QIODevice::ReadOnly | QIODevice::Text);
	QJsonParseError error;
	QJsonArray groups = QJsonDocument::fromJson(file.readAll(), &error).array();
	file.close();

	if(error.error != QJsonParseError::NoError)
		qDebug() << error.offset << error.errorString();
	else {
		for(QJsonValue group : groups) {
			QJsonObject groupObject = group.toObject();

			QStringList data;
			for(QJsonValue code : groupObject[QLatin1String("codes")].toArray()) {
				bool ok = false;
				uint rCode = code.toString().toUInt(&ok, 16);
				if(ok)
					data += Unicoder::code32ToSymbol(rCode);
			}
			DragStringListModel *model = new DragStringListModel(this);
			model->setStringList(data);

			QListView *view = new QListView(this->ui->tabWidget);
			QFont font = view->font();
			font.setPointSize(25);
			view->setFont(font);
			view->setContextMenuPolicy(Qt::ActionsContextMenu);
			view->setFrameShape(QFrame::NoFrame);
			view->setEditTriggers(QAbstractItemView::NoEditTriggers);
			view->setTabKeyNavigation(true);
			view->setDropIndicatorShown(false);
			view->setDragEnabled(true);
			view->setMovement(QListView::Static);
			view->setFlow(QListView::LeftToRight);
			view->setResizeMode(QListView::Adjust);
			view->setWrapping(true);
			view->setSelectionRectVisible(false);
			view->setDragDropMode(QAbstractItemView::DragOnly);

			model->createCopyAction(view);
			connect(view, &QListView::activated, this, &EmojiDialog::accept);
			connect(view, &QListView::activated, model, &DragStringListModel::activateItem);
			view->setModel(model);

			this->ui->tabWidget->addTab(view, groupObject[QLatin1String("name")].toString().replace(QLatin1Char('&'), QLatin1String("&&")));
		}
	}
}

EmojiDialog::~EmojiDialog()
{
	delete ui;
}
