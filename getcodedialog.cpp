#include "getcodedialog.h"
#include "ui_getcodedialog.h"
#include <QShowEvent>
#include "unicoder.h"
#include "databaseloader.h"

GetCodeDialog::GetCodeDialog() :
	PopupDialog(true),
	ui(new Ui::GetCodeDialog)
{
	ui->setupUi(this);
	connect(this->ui->pasteButton, &QToolButton::clicked,
			this->ui->symbolLineEdit, &QLineEdit::paste);
}

GetCodeDialog::~GetCodeDialog()
{
	delete ui;
}

void GetCodeDialog::showEvent(QShowEvent *event)
{
	this->ui->symbolLineEdit->clear();
	this->ui->symbolLineEdit->setFocus();
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
	} else {
		this->ui->uTF16CodeHighLineEdit->setEnabled(true);
		this->ui->uTF16CodeHighLineEdit->clear();
		this->ui->uTF16CodeLowLineEdit->clear();
		this->ui->uTF32CodeLineEdit->clear();
		this->ui->hTMLCodeLineEdit->clear();
		this->ui->symbolNameLineEdit->clear();
		this->ui->symbolGroupLineEdit->clear();
	}
}
