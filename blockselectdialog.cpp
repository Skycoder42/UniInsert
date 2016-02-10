#include "blockselectdialog.h"
#include "ui_blockselectdialog.h"
#include <QCompleter>
#include <QTreeView>
#include <QShowEvent>
#include "unicoder.h"
#include "advancedsearchdialog.h"
#include "databaseloader.h"

BlockSelectDialog::BlockSelectDialog() :
	PopupDialog(false),
	ui(new Ui::BlockSelectDialog),
	blockModel(Unicoder::databaseLoader()->createBlockModel(this)),
	displayModel(new DragStringListModel(this))
{
	ui->setupUi(this);

	this->displayModel->createCopyAction(this->ui->listView);
	this->ui->listView->setDragDropMode(QAbstractItemView::DragOnly);
	connect(this->ui->listView, &QListView::activated,
			this, &BlockSelectDialog::accept);
	connect(this->ui->listView, &QListView::activated,
			this->displayModel, &DragStringListModel::activateItem);

	this->ui->comboBox->setModel(this->blockModel);
	this->ui->listView->setModel(this->displayModel);
}

BlockSelectDialog::~BlockSelectDialog()
{
	delete ui;
}

void BlockSelectDialog::showEvent(QShowEvent *event)
{
	this->ui->comboBox->setCurrentIndex(-1);
	this->ui->comboBox->setCurrentIndex(0);
	event->accept();
}

void BlockSelectDialog::on_comboBox_currentIndexChanged(int index)
{
	QModelIndex modelIndex = this->blockModel->index(index, 0);
	if(modelIndex.isValid()) {
		int blockID = this->blockModel->data(modelIndex, DatabaseLoader::BlockModelDataRole).toInt();
		this->displayModel->setStringList(Unicoder::databaseLoader()->createBlock(blockID));
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
