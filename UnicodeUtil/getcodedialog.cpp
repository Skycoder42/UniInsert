#include "getcodedialog.h"
#include "ui_getcodedialog.h"
#include <QShowEvent>
#include <QWidgetAction>
#include <QStringListModel>
#include <QListView>
#include <QApplication>
#include <QClipboard>
#include <dialogmaster.h>
#include "unicoder.h"
#include "databaseloader.h"
#include "symbolselectdialog.h"

class AliasAction : public QWidgetAction
{
public:
	AliasAction(QObject *parent = Q_NULLPTR);
	void setAliases(const QStringList &aliases);
protected:
	QWidget *createWidget(QWidget *parent) Q_DECL_OVERRIDE;
private:
	QStringList aliases;
};



GetCodeDialog::GetCodeDialog(PopupController *controller) :
	PopupDialog(controller, true),
	ui(new Ui::GetCodeDialog),
	aliasAction(new AliasAction(this)),
	infoShow(false)
{
	ui->setupUi(this);
	this->ui->showAliasesButton->addAction(this->aliasAction);

	this->ui->actionSearch_Symbol->setShortcut(QKeySequence::Find);
	this->ui->searchButton->setDefaultAction(this->ui->actionSearch_Symbol);
	this->ui->symbolLineEdit->addAction(this->ui->actionSearch_Symbol);
}

GetCodeDialog::~GetCodeDialog()
{
	delete ui;
}

void GetCodeDialog::showSymbolInfo(uint code, QWidget *parent)
{
	if(code == UINT_MAX)
		return;
	GetCodeDialog dialog(Q_NULLPTR);
	dialog.setAutoHide(false);
	dialog.setParent(parent);
	DialogMaster::masterDialog(&dialog, true);
	dialog.infoShow = true;
	dialog.ui->symbolLineEdit->setText(Unicoder::code32ToSymbol(code));
	dialog.ui->symbolLineEdit->setEnabled(false);
	dialog.ui->actionSearch_Symbol->setEnabled(false);
	dialog.ui->exploreGroupButton->setEnabled(false);
	dialog.exec();
}

void GetCodeDialog::showEvent(QShowEvent *event)
{
	if(!this->infoShow) {
		this->ui->symbolLineEdit->clear();
		this->ui->symbolLineEdit->setFocus();
	}
	event->accept();
}

void GetCodeDialog::on_symbolLineEdit_textChanged(const QString &text)
{
	QString uniBase = QStringLiteral("U+%1");
	QString htmlBase = QStringLiteral("&#%1;");

	uint longCode = Unicoder::symbolToCode32(text);
	Unicoder::SurrogatePair shortCode = Unicoder::symbolToCode16(text);

	if(longCode != UINT_MAX) {
		this->ui->uTF32CodeLineEdit->setText(uniBase.arg(longCode, 4, 16, QLatin1Char('0')).toUpper());
		this->ui->hTMLCodeLineEdit->setText(htmlBase.arg(longCode).toUpper());
		if(shortCode.isSurrogated()) {
			this->ui->uTF16CodeHighLineEdit->setEnabled(true);
			this->ui->uTF16CodeHighLineEdit->setText(uniBase.arg(shortCode.highCode, 4, 16, QLatin1Char('0')).toUpper());
			this->ui->uTF16CodeLowLineEdit->setText(uniBase.arg(shortCode.lowCode, 4, 16, QLatin1Char('0')).toUpper());
		} else {
			this->ui->uTF16CodeHighLineEdit->setEnabled(false);
			this->ui->uTF16CodeHighLineEdit->clear();
			this->ui->uTF16CodeLowLineEdit->setText(uniBase.arg(shortCode.lowCode, 4, 16, QLatin1Char('0')).toUpper());
		}
		this->ui->symbolNameLineEdit->setText(Unicoder::databaseLoader()->nameForSymbol(text));
		this->ui->symbolGroupLineEdit->setText(Unicoder::databaseLoader()->findBlockName(text));
		this->ui->addRecentButton->setEnabled(true);
		this->aliasAction->setAliases(Unicoder::databaseLoader()->symbolAliases(longCode));
		this->ui->showAliasesButton->setEnabled(true);
		this->ui->exploreGroupButton->setEnabled(true);
	} else {
		this->ui->uTF16CodeHighLineEdit->setEnabled(true);
		this->ui->uTF16CodeHighLineEdit->clear();
		this->ui->uTF16CodeLowLineEdit->clear();
		this->ui->uTF32CodeLineEdit->clear();
		this->ui->hTMLCodeLineEdit->clear();
		this->ui->symbolNameLineEdit->clear();
		this->ui->symbolGroupLineEdit->clear();
		this->ui->addRecentButton->setEnabled(false);
		this->ui->showAliasesButton->setEnabled(false);
		this->aliasAction->setAliases(QStringList());
		this->ui->exploreGroupButton->setEnabled(false);
	}
}

void GetCodeDialog::on_exploreGroupButton_clicked()
{
	int blockID = Unicoder::databaseLoader()->findBlock(this->ui->symbolLineEdit->text());
	if(blockID > -1)
		emit showBlock(blockID);//TODO
}

void GetCodeDialog::on_actionSearch_Symbol_triggered()
{
	this->setAutoHide(false);
	uint code = SymbolSelectDialog::getSymbol(this);
	if(code != UINT_MAX)
		this->ui->symbolLineEdit->setText(Unicoder::code32ToSymbol(code));
	this->setAutoHide(true);
}

void GetCodeDialog::on_addRecentButton_clicked()
{
	Unicoder::databaseLoader()->updateRecent(this->ui->symbolLineEdit->text());
}



QString GetCodeController::actionName() const
{
	return GetCodeDialog::tr("Show symbol information");
}

QKeySequence GetCodeController::defaultKeySequence() const
{
	return QKeySequence(Qt::CTRL | Qt::META | Qt::Key_Asterisk);
}

PopupDialog *GetCodeController::createDialog()
{
	return new GetCodeDialog(this);
}



AliasAction::AliasAction(QObject *parent) :
	QWidgetAction(parent),
	aliases()
{}

void AliasAction::setAliases(const QStringList &aliases)
{
	this->aliases = aliases;
}

QWidget *AliasAction::createWidget(QWidget *parent)
{
	QListView *view = new QListView(parent);
	view->setEditTriggers(QListView::NoEditTriggers);
	view->setSelectionMode(QListView::SingleSelection);
	view->setAlternatingRowColors(true);
	view->setDragDropMode(QListView::NoDragDrop);
	view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	view->setContextMenuPolicy(Qt::ActionsContextMenu);

	QStringListModel *model = new QStringListModel(this->aliases, view);
	view->setModel(model);
	view->setMinimumWidth(view->sizeHintForColumn(0) + view->style()->pixelMetric(QStyle::PM_ScrollBarExtent) + 5);

	QAction *copyAction = new QAction(QIcon(QStringLiteral(":/icons/copy.ico")), GetCodeDialog::tr("Copy Name"), view);
	copyAction->setShortcut(QKeySequence::Copy);
	copyAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	connect(copyAction, &QAction::triggered, view, [=](){
		QModelIndex index = view->currentIndex();
		if(index.isValid()) {
			QString text = model->data(index, Qt::DisplayRole).toString();
			QApplication::clipboard()->setText(text);
		}
	});
	view->addAction(copyAction);

	return view;
}
