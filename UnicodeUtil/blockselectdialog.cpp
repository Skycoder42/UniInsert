#include "blockselectdialog.h"
#include "ui_blockselectdialog.h"
#include <QCompleter>
#include <QTreeView>
#include <QShowEvent>
#include "unicoder.h"
#include "advancedsearchdialog.h"
#include "databaseloader.h"
#include "unicodermodels.h"
#include "settingsdialog.h"

BlockSelectDialog::BlockSelectDialog(PopupController *controller) :
	PopupDialog(controller, false),
	ui(new Ui::BlockSelectDialog),
	blockModel(Unicoder::databaseLoader()->createBlockModel(this)),
	displayModel(new SymbolListModel(this)),
	indexSet(false)
{
	ui->setupUi(this);
	SettingsDialog::loadSize(this);

	this->displayModel->createCopyAction(this->ui->listView);
	QAction *seperator = new QAction(this);
	seperator->setSeparator(true);
	this->ui->listView->addActions({
									   seperator,
									   this->ui->actionRemove_from_list
								   });
	this->ui->listView->setDragDropMode(QAbstractItemView::DragOnly);
	connect(this->ui->listView, &QListView::activated,
			this, &BlockSelectDialog::accept);
	connect(this->ui->listView, &QListView::activated,
			this->displayModel, &SymbolListModel::activateItem);

	this->ui->comboBox->setModel(this->blockModel);
	this->ui->listView->setModel(this->displayModel);
}

BlockSelectDialog::~BlockSelectDialog()
{
	SettingsDialog::storeSize(this);
	delete ui;
}

void BlockSelectDialog::showBlock(int blockID)
{
	//try the easy way...
	QModelIndex index = this->blockModel->index(blockID, 3);
	if(index.isValid() && index.data().toInt() == blockID)
		this->ui->comboBox->setCurrentIndex(blockID);
	else {//if that fails -> long fallback...
		qDebug("FALLBACK");
		for(int i = 0, max = this->blockModel->rowCount(); i < max; ++i) {
			int id = this->blockModel->data(this->blockModel->index(i, 3)).toInt();
			if(id == blockID) {
				this->ui->comboBox->setCurrentIndex(i);
				break;
			}
		}
	}
	this->indexSet = true;
	this->popup();
}

void BlockSelectDialog::showEvent(QShowEvent *event)
{
	if(this->indexSet)
		this->indexSet = false;
	else {
		this->ui->comboBox->setCurrentIndex(-1);
		this->ui->comboBox->setCurrentIndex(0);
	}
	event->accept();
}

void BlockSelectDialog::on_comboBox_currentIndexChanged(int index)
{
	QModelIndex modelIndex = this->blockModel->index(index, 3);
	if(modelIndex.isValid()) {
		int blockID = this->blockModel->data(modelIndex).toInt();
		Unicoder::databaseLoader()->createBlock(blockID, this->displayModel);
		this->ui->actionRemove_from_list->setEnabled(blockID == 0);
		this->ui->actionRemove_from_list->setVisible(blockID == 0);
	}
}

void BlockSelectDialog::on_toolButton_clicked()
{
	this->setAutoHide(false);
	QModelIndex index = AdvancedSearchDialog::searchBlock(this, this->blockModel);
	if(index.isValid())
		this->ui->comboBox->setCurrentIndex(index.row());
	this->setAutoHide(true);
}

void BlockSelectDialog::on_actionRemove_from_list_triggered()
{
	if(this->ui->comboBox->currentIndex() == 0)
		this->displayModel->removeRecentItem(this->ui->listView->currentIndex());
}



QString BlockSelectController::actionName() const
{
	return BlockSelectDialog::tr("Blocklist/Recently used");
}

QKeySequence BlockSelectController::defaultKeySequence() const
{
	return QKeySequence(Qt::CTRL | Qt::META | Qt::Key_Delete);
}

PopupDialog *BlockSelectController::createDialog()
{
	return new BlockSelectDialog(this);
}
