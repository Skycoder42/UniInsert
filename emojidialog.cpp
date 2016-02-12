#include "emojidialog.h"
#include "ui_emojidialog.h"
#include <QListView>
#include "unicoder.h"
#include "databaseloader.h"
#include "unicodermodels.h"

EmojiDialog::EmojiDialog(QWidget *parent) :
	PopupDialog(parent),
	ui(new Ui::EmojiDialog)
{
	ui->setupUi(this);

	typedef QMap<int, QString>::const_iterator itr;
	QMap<int, QString> groups = Unicoder::databaseLoader()->listEmojiGroups();
	for(itr it = groups.begin(), end = groups.end(); it != end; ++it) {
		SymbolListModel *model = Unicoder::databaseLoader()->createEmojiGroup(it.key(), this);

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
		connect(view, &QListView::activated, model, &SymbolListModel::activateItem);
		view->setModel(model);

		QString title = it.value();
		title.replace(QLatin1Char('&'), QStringLiteral("&&"));
		this->ui->tabWidget->addTab(view, title);
	}
}

EmojiDialog::~EmojiDialog()
{
	delete ui;
}
