#include "emojidialog.h"
#include "ui_emojidialog.h"
#include <QListView>
#include <QMouseEvent>
#include <QApplication>
#include <QClipboard>
#include "unicoder.h"
#include "databaseloader.h"
#include "unicodermodels.h"
#include "symbolselectdialog.h"
#include "settingsdialog.h"
#include "dialogmaster.h"

EmojiDialog::EmojiDialog(PopupController *controller) :
	PopupDialog(controller, false),
	ui(new Ui::EmojiDialog),
	addMapper(new QSignalMapper(this)),
	deleteMapper(new QSignalMapper(this)),
	pasteMapper(new QSignalMapper(this)),
	tabContextWidget(Q_NULLPTR),
	tabModels()
{
	ui->setupUi(this);
	SettingsDialog::loadSize(this);

	connect(QApplication::clipboard(), &QClipboard::dataChanged,
			this, &EmojiDialog::clipChange, Qt::QueuedConnection);

	this->ui->tabWidget->installEventFilter(this);
	this->ui->tabWidget->addActions({
										this->ui->actionAdd_Emoji_Group,
										this->ui->actionRemove_Emoji_Group
									});

	connect(this->addMapper, SIGNAL(mapped(QObject*)),
			this, SLOT(addTriggered(QObject*)));
	connect(this->deleteMapper, SIGNAL(mapped(QObject*)),
			this, SLOT(deleteTriggered(QObject*)));
	connect(this->pasteMapper, SIGNAL(mapped(QObject*)),
			this, SLOT(pasteTriggered(QObject*)));

	typedef QList<DatabaseLoader::EmojiGroupInfo>::const_iterator itr;
	QList<DatabaseLoader::EmojiGroupInfo> groups = Unicoder::databaseLoader()->listEmojiGroups();
	for(itr it = groups.begin(), end = groups.end(); it != end; ++it)
		this->createTab(it->first, it->second);
}

EmojiDialog::~EmojiDialog()
{
	SettingsDialog::storeSize(this);
	delete ui;
}

bool EmojiDialog::eventFilter(QObject *, QEvent *event)
{
	if(event->type() == QEvent::MouseButtonPress) {
		QMouseEvent *ev = static_cast<QMouseEvent*>(event);
		if(ev->button() == Qt::RightButton) {
			int index = this->ui->tabWidget->tabBar()->tabAt(ev->pos());
			this->tabContextWidget = this->ui->tabWidget->widget(index);
			this->ui->actionRemove_Emoji_Group->setEnabled(index > -1);
		}
	}
	return false;
}

void EmojiDialog::closeEvent(QCloseEvent *ev)
{
	QList<int> idOrder;
	for(int i = 0, max = this->ui->tabWidget->count(); i < max; ++i) {
		QWidget *wdg = this->ui->tabWidget->widget(i);
		SymbolListModel *model = this->tabModels.value(wdg);
		if(model)
			idOrder += model->property("groupID").toInt();
	}
	Unicoder::databaseLoader()->updateEmojiGroupOrder(idOrder);
	this->QDialog::closeEvent(ev);
}

void EmojiDialog::addTriggered(QObject *model)
{
	Q_ASSERT(dynamic_cast<SymbolListModel*>(model));
	SymbolListModel *symbolModel = static_cast<SymbolListModel*>(model);

	this->setAutoHide(false);
	uint code = SymbolSelectDialog::getSymbol(this);
	if(code != UINT_MAX) {
		int groupID = symbolModel->property("groupID").toInt();
		if(Unicoder::databaseLoader()->addEmoji(groupID, code))
			symbolModel->refresh();
		else
			DialogMaster::critical(this, tr("Failed to add the emoji to the list"));
	}
	this->setAutoHide(true);
}

void EmojiDialog::deleteTriggered(QObject *model)
{
	Q_ASSERT(dynamic_cast<SymbolListModel*>(model));
	SymbolListModel *symbolModel = static_cast<SymbolListModel*>(model);
	QListView *view = symbolModel->property("view").value<QListView*>();
	QModelIndex index = view->currentIndex();
	if(index.isValid()) {
		int groupID = symbolModel->property("groupID").toInt();
		uint code = symbolModel->data(index, Qt::EditRole).toUInt();
		if(Unicoder::databaseLoader()->removeEmoji(groupID, code))
			symbolModel->refresh();
		else {
			this->setAutoHide(false);
			DialogMaster::critical(this, tr("Failed to remove the emoji from the list"));
			this->setAutoHide(true);
		}
	}
}

void EmojiDialog::pasteTriggered(QObject *model)
{
	Q_ASSERT(dynamic_cast<SymbolListModel*>(model));
	SymbolListModel *symbolModel = static_cast<SymbolListModel*>(model);
	uint code = Unicoder::symbolToCode32(QApplication::clipboard()->text());
	if(code != UINT_MAX) {
		if(Unicoder::databaseLoader()->addEmoji(symbolModel->property("groupID").toInt(), code)) {
			symbolModel->refresh();
			return;
		}
	}

	this->setAutoHide(false);
	DialogMaster::critical(this, tr("Failed to add the emoji to the list"));
	this->setAutoHide(true);
}

void EmojiDialog::on_actionAdd_Emoji_Group_triggered()
{
	this->setAutoHide(false);
	QString name = DialogMaster::getText(this,
										 tr("Enter the name for the new group:"),
										 tr("Create Emoji Group"));
	if(!name.isEmpty()) {
		int id = Unicoder::databaseLoader()->createEmojiGroup(name);
		if(id != -1)
			this->createTab(id, name);
		else
			DialogMaster::critical(this, tr("Failed to create the emoji group"));
	}
	this->setAutoHide(true);
}

void EmojiDialog::on_actionRemove_Emoji_Group_triggered()
{
	SymbolListModel *model = this->tabModels.value(this->tabContextWidget, Q_NULLPTR);
	if(!model)
		return;
	this->setAutoHide(false);
	if(DialogMaster::question(this,
							  tr("Do you really want to delete the emoji group?"),
							  tr("Delete Emoji Group"))
	   == QMessageBox::Yes) {
		if(Unicoder::databaseLoader()->deleteEmojiGroup(model->property("groupID").toInt())) {
			this->tabModels.remove(this->tabContextWidget);
			this->tabContextWidget->deleteLater();
			this->tabContextWidget = Q_NULLPTR;
		} else
			DialogMaster::critical(this, tr("Failed to delete the emoji group"));
	}
	this->setAutoHide(true);
}

void EmojiDialog::createTab(int groupID, const QString &groupName)
{
	SymbolListModel *model = Unicoder::databaseLoader()->createEmojiGroupModel(groupID, this);
	model->setProperty("groupID", groupID);

	QListView *view = new QListView(this->ui->tabWidget);
	QFont font = view->font();
	font.setPointSize(25);
	view->setFont(font);
	view->setContextMenuPolicy(Qt::ActionsContextMenu);
	view->setFrameShape(QFrame::NoFrame);
	view->setEditTriggers(QAbstractItemView::NoEditTriggers);
	view->setDropIndicatorShown(true);
	view->setDragEnabled(true);
	view->setMovement(QListView::Static);
	view->setFlow(QListView::LeftToRight);
	view->setResizeMode(QListView::Adjust);
	view->setWrapping(true);
	view->setSelectionRectVisible(false);
	view->setDragDropMode(QAbstractItemView::DragDrop);
	view->setDragDropOverwriteMode(false);
	view->setDefaultDropAction(Qt::MoveAction);

	model->createCopyAction(view);
	connect(view, &QListView::activated, this, &EmojiDialog::accept);
	connect(view, &QListView::activated, model, &SymbolListModel::activateItem);
	view->setModel(model);
	model->setProperty("view", QVariant::fromValue(view));

	QAction *pasteAction = new QAction(QIcon(QStringLiteral(":/icons/paste.ico")), tr("Paste Symbol"), view);
	pasteAction->setShortcut(QKeySequence::Paste);
	pasteAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	this->pasteMapper->setMapping(pasteAction, model);
	connect(pasteAction, SIGNAL(triggered()), this->pasteMapper, SLOT(map()));
	connect(this, &EmojiDialog::updatePasteEnabled, pasteAction, &QAction::setEnabled);

	QAction *seperator = new QAction(view);
	seperator->setSeparator(true);

	QAction *addAction = new QAction(QIcon(QStringLiteral(":/icons/add.ico")), tr("Add Emoji"), view);
	addAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Insert));
	addAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	this->addMapper->setMapping(addAction, model);
	connect(addAction, SIGNAL(triggered()), this->addMapper, SLOT(map()));

	QAction *removeAction = new QAction(QIcon(QStringLiteral(":/icons/delete.ico")), tr("Remove Emoji"), view);
	removeAction->setShortcut(QKeySequence::Delete);
	removeAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	this->deleteMapper->setMapping(removeAction, model);
	connect(removeAction, SIGNAL(triggered()), this->deleteMapper, SLOT(map()));

	view->addActions({
						 pasteAction,
						 seperator,
						 addAction,
						 removeAction
					 });

	QString title = groupName;
	title.replace(QLatin1Char('&'), QStringLiteral("&&"));
	this->ui->tabWidget->addTab(view, title);
	this->tabModels.insert(view, model);

	this->clipChange();
}

void EmojiDialog::clipChange()
{
	uint code = Unicoder::symbolToCode32(QApplication::clipboard()->text());
	emit updatePasteEnabled(code != UINT_MAX, QPrivateSignal());
}



QString EmojiController::actionName() const
{
	return EmojiDialog::tr("Emojis");
}

QKeySequence EmojiController::defaultKeySequence() const
{
	return QKeySequence(Qt::CTRL | Qt::META | Qt::Key_Insert);
}

PopupDialog *EmojiController::createDialog()
{
	return new EmojiDialog(this);
}
