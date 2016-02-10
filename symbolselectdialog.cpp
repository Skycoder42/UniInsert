#include "symbolselectdialog.h"
#include "ui_symbolselectdialog.h"
#include <QRegularExpressionMatch>
#include <QShowEvent>
#include "unicoder.h"

const QRegularExpression SymbolSelectDialog::unicodeRegex(QStringLiteral(R"__(^(?:U\+?|\\u)?((?:10|[\dA-F])?[\dA-F]{4})$)__"),
														  QRegularExpression::CaseInsensitiveOption);

SymbolSelectDialog::SymbolSelectDialog() :
	PopupDialog(true),
	ui(new Ui::SymbolSelectDialog),
	validator(new QRegularExpressionValidator(SymbolSelectDialog::unicodeRegex, this)),
	previewAction(new PreviewAction(this))
{
	this->ui->setupUi(this);
	this->ui->unicodeLineEdit->setValidator(this->validator);
	this->ui->previewButton->addAction(this->previewAction);

	this->ui->actionCopy_Symbol->setShortcut(QKeySequence::Copy);
	this->ui->copyButton->setDefaultAction(this->ui->actionCopy_Symbol);
	this->ui->previewLineEdit->addAction(this->ui->actionCopy_Symbol);
}

SymbolSelectDialog::~SymbolSelectDialog()
{
	delete this->ui;
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
	QString codeText;
	if(isInputOk) {
		codeText = this->calcUnicode(text);
		isInputOk = !codeText.isNull();
	}

	this->ui->insertButton->setEnabled(isInputOk);
	this->ui->copyButton->setEnabled(isInputOk);
	if(isInputOk) {
		this->ui->previewLineEdit->setText(codeText);
		this->previewAction->setText(codeText);
	} else {
		this->ui->previewLineEdit->clear();
		this->previewAction->setText(QString());
	}
}

void SymbolSelectDialog::on_insertButton_clicked()
{
	Unicoder::sendSymbolInput(this->ui->previewLineEdit->text());
	this->accept();
}

void SymbolSelectDialog::on_actionCopy_Symbol_triggered()
{
	Unicoder::copySymbol(this->ui->previewLineEdit->text());
}

void SymbolSelectDialog::on_searchButton_clicked()
{

}

QString SymbolSelectDialog::calcUnicode(const QString &code)
{
	QRegularExpressionMatch match = SymbolSelectDialog::unicodeRegex.match(code);
	if(match.hasMatch()) {
		bool ok = false;
		uint symbol = match.captured(1).toUInt(&ok, 16);
		if(!ok)
			return QString();

		return Unicoder::code32ToSymbol(symbol);
	} else
		return QString();
}




SymbolSelectDialog::PreviewAction::PreviewAction(QObject *parent) :
	QWidgetAction(parent)
{}

void SymbolSelectDialog::PreviewAction::setText(const QString &text)
{
	this->text = text;
	for(QWidget *widget : this->createdWidgets()) {
		Q_ASSERT(dynamic_cast<QLabel*>(widget));
		QLabel *label = static_cast<QLabel*>(widget);
		label->setText(text);
		label->setFixedWidth(qMax(label->fontMetrics().width(text),
								  label->height()));
	}
}

QWidget *SymbolSelectDialog::PreviewAction::createWidget(QWidget *parent)
{
	QLabel *previewLabel = new QLabel(parent);
	QFont font = previewLabel->font();
	font.setPixelSize(32);
	previewLabel->setFont(font);
	previewLabel->setAlignment(Qt::AlignCenter);

	previewLabel->setText(this->text);
	previewLabel->setFixedHeight(previewLabel->fontMetrics().height());
	previewLabel->setFixedWidth(qMax(previewLabel->fontMetrics().width(this->text),
									 previewLabel->height()));
	return previewLabel;
}
