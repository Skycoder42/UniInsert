#include "blockselectdialog.h"
#include "ui_blockselectdialog.h"
#include <QCompleter>
#include <QTreeView>
#include <QShowEvent>
#include "unicoder.h"
#include "advancedsearchdialog.h"
#include "databaseloader.h"
#include "unicodermodels.h"

BlockSelectDialog::BlockSelectDialog() :
	PopupDialog(false),
	ui(new Ui::BlockSelectDialog),
	blockModel(Unicoder::databaseLoader()->createBlockModel(this)),
	displayModel(new SymbolListModel(this)),
	indexSet(false)
{
	ui->setupUi(this);

	this->displayModel->createCopyAction(this->ui->listView);
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
