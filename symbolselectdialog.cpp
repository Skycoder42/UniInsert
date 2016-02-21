#include "symbolselectdialog.h"
#include "ui_symbolselectdialog.h"
#include <QRegularExpressionMatch>
#include <QShowEvent>
#include <QMouseEvent>
#include <QMimeData>
#include <QDrag>
#include <QWindow>
#include <QScreen>
#include "unicoder.h"
#include "advancedsearchdialog.h"
#include "unicodermodels.h"
#include "dialogmaster.h"

const QRegularExpression SymbolSelectDialog::unicodeRegex(QStringLiteral(R"__(^(?:U\+?|\\u)?((?:10|[\dA-F])?[\dA-F]{4})$)__"),
														  QRegularExpression::CaseInsensitiveOption);

SymbolSelectDialog::SymbolSelectDialog() :
	PopupDialog(true),
	ui(new Ui::SymbolSelectDialog),
	validator(new QRegularExpressionValidator(SymbolSelectDialog::unicodeRegex, this)),
	doInsert(true)
{
	this->ui->setupUi(this);
	this->ui->unicodeLineEdit->setValidator(this->validator);
	QFont font = this->ui->previewLabel->font();
	font.setPixelSize(32);
	this->ui->previewLabel->setFont(font);
	this->ui->previewLabel->setFixedHeight(this->ui->previewLayout->sizeHint().height());
	this->ui->previewLabel->setFixedWidth(this->ui->previewLayout->sizeHint().height());

	this->ui->actionCopy_Symbol->setShortcut(QKeySequence::Copy);
	this->ui->copyButton->setDefaultAction(this->ui->actionCopy_Symbol);
	this->ui->previewLineEdit->addAction(this->ui->actionCopy_Symbol);

	this->ui->actionSearch_symbol_name->setShortcut(QKeySequence::Find);
	this->ui->searchButton->setDefaultAction(this->ui->actionSearch_symbol_name);
	this->ui->unicodeLineEdit->addAction(this->ui->actionSearch_symbol_name);
}

SymbolSelectDialog::~SymbolSelectDialog()
{
	delete this->ui;
}

uint SymbolSelectDialog::getSymbol(QWidget *parent)
{
	SymbolSelectDialog selectDialog;
	selectDialog.setAutoHide(false);
	selectDialog.setParent(parent);
	DialogMaster::masterDialog(&selectDialog, true);
	selectDialog.doInsert = false;
	if(selectDialog.exec())
		return Unicoder::symbolToCode32(selectDialog.ui->previewLineEdit->text());
	else
		return UINT_MAX;
}

void SymbolSelectDialog::showEvent(QShowEvent *event)
{
	this->ui->unicodeLineEdit->clear();
	this->ui->unicodeLineEdit->setFocus();
	event->accept();
}

void SymbolSelectDialog::on_unicodeLineEdit_textChanged(const QString &text)
{
	bool isInputOk = this->ui->unicodeLineEdit->hasAcceptableInput();
	uint code;
	if(isInputOk) {
		code = this->calcUnicode(text);
		isInputOk = (code != UINT_MAX);
	}

	this->ui->insertButton->setEnabled(isInputOk);
	this->ui->copyButton->setEnabled(isInputOk);
	if(isInputOk) {
		QString symbol = Unicoder::code32ToSymbol(code);
		QString name = Unicoder::databaseLoader()->nameForSymbol(code);
		this->ui->previewLineEdit->setText(symbol);
		this->ui->previewLineEdit->setToolTip(name);
		this->ui->previewLabel->setText(symbol);
		this->ui->previewLabel->setToolTip(name);
	} else {
		this->ui->previewLineEdit->clear();
		this->ui->previewLineEdit->setToolTip(QString());
		this->ui->previewLabel->clear();
		this->ui->previewLabel->setToolTip(QString());
	}
}

void SymbolSelectDialog::on_insertButton_clicked()
{
	if(this->doInsert)
		Unicoder::sendSymbolInput(this->ui->previewLineEdit->text());
	this->accept();
}

void SymbolSelectDialog::on_actionCopy_Symbol_triggered()
{
	Unicoder::copySymbol(this->ui->previewLineEdit->text());
}

void SymbolSelectDialog::on_actionSearch_symbol_name_triggered()
{
	bool outHideOld = this->doesAutoHide();
	this->setAutoHide(false);
	uint code = AdvancedSearchDialog::searchSymbol(this);
	if(code != UINT_MAX)
		this->ui->unicodeLineEdit->setText(UnicodeDelegate::displayCode(code));
	this->setAutoHide(outHideOld);
}

uint SymbolSelectDialog::calcUnicode(const QString &code)
{
	QRegularExpressionMatch match = SymbolSelectDialog::unicodeRegex.match(code);
	if(match.hasMatch()) {
		bool ok = false;
		uint symbol = match.captured(1).toUInt(&ok, 16);
		if(ok)
			return symbol;
	}

	return UINT_MAX;
}



DragLabel::DragLabel(QWidget *parent) :
	QLabel(parent),
	dragStartPosition()
{}

void DragLabel::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
		this->dragStartPosition = event->pos();
}
#include <QBitmap>
void DragLabel::mouseMoveEvent(QMouseEvent *event)
{
	if (!(event->buttons() & Qt::LeftButton))
		return;
	if ((event->pos() - this->dragStartPosition).manhattanLength()
		 < QApplication::startDragDistance())
		return;
	if(this->text().isNull())
		return;

	QDrag *drag = new QDrag(this);

	QWidget *window = this->window();
	Q_ASSERT(window);
	Q_ASSERT(window->windowHandle());
	Q_ASSERT(window->windowHandle()->screen());
	QRect geom = this->geometry();
	geom.moveTopLeft(this->mapTo(window, QPoint(0,0)));
	QPixmap grab = window->windowHandle()->screen()->grabWindow(window->winId(), geom.x(), geom.y(), geom.width(), geom.height());
	if(!grab.isNull())
		drag->setDragCursor(grab, Qt::CopyAction);

	QMimeData *mimeData = new QMimeData();
	mimeData->setText(this->text());
	drag->setMimeData(mimeData);

	drag->exec(Qt::CopyAction);
}
