#include "symbolselectdialog.h"
#include "ui_symbolselectdialog.h"
#include <QRegularExpressionMatch>
#include <QShowEvent>
#include <QMouseEvent>
#include <QMimeData>
#include <QDrag>
#include <QWindow>
#include <QScreen>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QMenu>
#include "unicoder.h"
#include "unicodermodels.h"
#include "dialogmaster.h"
#include "settingsdialog.h"

const QRegularExpression SymbolSelectDialog::unicodeRegex(QStringLiteral(R"__(^(?:(?:U\+?|\\u)?((?:10|[\dA-F])?[\dA-F]{4}))|(?:&#(\d+);)$)__"),
														  QRegularExpression::CaseInsensitiveOption |
														  QRegularExpression::OptimizeOnFirstUsageOption);

SymbolSelectDialog::SymbolSelectDialog() :
	PopupDialog(false),
	ui(new Ui::SymbolSelectDialog),
	currentCode(UINT_MAX),
	doInsert(true),
	isSearchMode(true),
	proxyModel(new QSortFilterProxyModel(this)),
	symbolModel(Unicoder::databaseLoader()->createSearchModel(this, true)),
	mode(DatabaseLoader::Contains)
{
	this->ui->setupUi(this);
	SettingsDialog::loadSize(this);

	this->ui->unicodeLineEdit->addActions({
											  this->ui->actionEnter_unicode_codepoint,
											  this->ui->actionEnter_HTML_code
										  });

	QFont font = this->ui->previewLabel->font();
	font.setPixelSize(32);
	this->ui->previewLabel->setFont(font);
	this->ui->previewLabel->setFixedHeight(this->ui->previewLayout->sizeHint().height());
	this->ui->previewLabel->setFixedWidth(this->ui->previewLayout->sizeHint().height());

	QAction *seperator1 = new QAction(this);
	seperator1->setSeparator(true);
	QAction *seperator2 = new QAction(this);
	seperator2->setSeparator(true);

	this->ui->actionCopy_Symbol->setShortcut(QKeySequence::Copy);
	this->ui->copyButton->setDefaultAction(this->ui->actionCopy_Symbol);
	this->ui->copyButton->addActions({
										 seperator1,
										 this->ui->actionCopy_Symbol_Name,
										 this->ui->actionCopy_symbol_unicode_codepoint,
										 this->ui->actionCopy_symbol_HTML_code
									 });

	this->ui->actionShow_Symbol_Info->setShortcut(QKeySequence::HelpContents);
	this->ui->symbolInfoButton->setDefaultAction(this->ui->actionShow_Symbol_Info);

	this->ui->previewLabel->addActions({
										   this->ui->actionCopy_Symbol,
										   seperator1,
										   this->ui->actionCopy_Symbol_Name,
										   this->ui->actionCopy_symbol_unicode_codepoint,
										   this->ui->actionCopy_symbol_HTML_code,
										   seperator2,
										   this->ui->actionShow_Symbol_Info
									   });

	this->proxyModel->setSourceModel(this->symbolModel);
	this->proxyModel->setSortLocaleAware(true);
	this->proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
	this->ui->searchTreeView->setModel(this->proxyModel);
	connect(this->ui->searchTreeView->selectionModel(), &QItemSelectionModel::currentChanged,
			this, &SymbolSelectDialog::searchIndexChanged);

	this->ui->searchTreeView->setItemDelegateForColumn(0, new UnicodeDelegate(true, this->ui->searchTreeView));
	this->ui->searchTreeView->setItemDelegateForColumn(1, new UnicodeDelegate(false, this->ui->searchTreeView));
	this->ui->searchTreeView->addActions({
											 this->ui->actionCopy_Symbol,
											 this->ui->actionShow_Symbol_Info
										 });

	if(!this->ui->searchTreeView->header()->restoreState(SettingsDialog::loadValue(this, QStringLiteral("symbolHeaderState")).toByteArray())) {
		this->ui->searchTreeView->resizeColumnToContents(0);
		this->ui->searchTreeView->sortByColumn(-1, Qt::AscendingOrder);
	}

	this->ui->findAliasCheckBox->setChecked(SettingsDialog::loadValue(this, QStringLiteral("findAlias"), true).toBool());
}

SymbolSelectDialog::~SymbolSelectDialog()
{
	SettingsDialog::storeSize(this);
	SettingsDialog::storeValue(this,
							   QStringLiteral("symbolHeaderState"),
							   this->ui->searchTreeView->header()->saveState());
	SettingsDialog::storeValue(this,
							   QStringLiteral("findAlias"),
							   this->ui->findAliasCheckBox->isChecked());
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
		return Unicoder::symbolToCode32(selectDialog.ui->previewLabel->text());
	else
		return UINT_MAX;
}

void SymbolSelectDialog::showEvent(QShowEvent *event)
{
	this->currentCode = UINT_MAX;
	this->updateSymbol();
	this->ui->unicodeLineEdit->clear();
	this->ui->unicodeLineEdit->setFocus();
	this->on_unicodeLineEdit_textEdited(QString());
	event->accept();
}

void SymbolSelectDialog::on_unicodeLineEdit_textEdited(const QString &text)
{
	this->updateSearch(text, false);
}

void SymbolSelectDialog::on_unicodeLineEdit_returnPressed()
{
	this->updateSearch(this->ui->unicodeLineEdit->text(),  true);
}

void SymbolSelectDialog::on_filterModeComboBox_currentIndexChanged(int index)
{
	this->mode = (DatabaseLoader::SearchFlag)index;
	this->updateSearch(this->ui->unicodeLineEdit->text(), false);
}

void SymbolSelectDialog::on_findAliasCheckBox_toggled(bool)
{
	this->updateSearch(this->ui->unicodeLineEdit->text(), false);
}

void SymbolSelectDialog::on_searchTreeView_activated(const QModelIndex &index)
{
	uint code = index.sibling(index.row(), 1).data().toUInt();
	if(this->doInsert)
		Unicoder::sendSymbolInput(Unicoder::code32ToSymbol(code));
	this->accept();
}

void SymbolSelectDialog::searchIndexChanged(const QModelIndex &current, const QModelIndex &)
{
	if(current.isValid())
		this->currentCode = current.sibling(current.row(), 1).data().toUInt();
	else
		this->currentCode = UINT_MAX;
	this->updateSymbol();
}

void SymbolSelectDialog::on_actionCopy_Symbol_triggered()
{
	Unicoder::copySymbol(this->ui->previewLabel->text());
}

void SymbolSelectDialog::on_actionShow_Symbol_Info_triggered()
{
	Q_UNIMPLEMENTED();
}

void SymbolSelectDialog::on_actionCopy_Symbol_Name_triggered()
{
	QApplication::clipboard()->setText(this->ui->previewLabel->toolTip());
}

void SymbolSelectDialog::on_actionCopy_symbol_unicode_codepoint_triggered()
{
	QApplication::clipboard()->setText(QStringLiteral("U+%1").arg(this->currentCode, 4, 16, QChar('0')).toUpper());
}

void SymbolSelectDialog::on_actionCopy_symbol_HTML_code_triggered()
{
	QApplication::clipboard()->setText(QStringLiteral("&#%1;").arg(this->currentCode));
}

void SymbolSelectDialog::on_actionEnter_unicode_codepoint_triggered()
{
	QString text = QStringLiteral("U+");
	this->ui->unicodeLineEdit->selectAll();
	this->ui->unicodeLineEdit->insert(text);
	this->updateSearch(text, false);
	this->ui->unicodeLineEdit->setCursorPosition(2);
}

void SymbolSelectDialog::on_actionEnter_HTML_code_triggered()
{
	QString text = QStringLiteral("&#;");
	this->ui->unicodeLineEdit->selectAll();
	this->ui->unicodeLineEdit->insert(text);
	this->updateSearch(text, false);
	this->ui->unicodeLineEdit->setCursorPosition(2);
}

uint SymbolSelectDialog::calcUnicode(const QString &code)
{
	QRegularExpressionMatch match = SymbolSelectDialog::unicodeRegex.match(code);
	if(match.hasMatch()) {
		bool ok = false;
		uint symbol = match.captured(1).toUInt(&ok, 16);
		if(ok)
			return symbol;
		else {
			symbol = match.captured(2).toUInt(&ok);
			if(ok)
				return symbol;
		}
	}

	return UINT_MAX;
}

void SymbolSelectDialog::updateSearch(const QString &text, bool force)
{
	bool newMode;
	if(text.startsWith(QStringLiteral("U+"), Qt::CaseInsensitive) ||
	   text.startsWith(QStringLiteral("\\u"), Qt::CaseInsensitive) ||
	   text.startsWith(QStringLiteral("&#")))
		newMode = false;
	else
		newMode = true;

	if(newMode != this->isSearchMode) {
		this->isSearchMode = newMode;
		this->currentCode = UINT_MAX;
		this->updateSymbol();

		if(this->isSearchMode) {
			this->ui->searchTreeView->setEnabled(true);
		} else {
			Unicoder::databaseLoader()->clearSearchModel(this->symbolModel, this->ui->findAliasCheckBox->isChecked());
			this->ui->searchTreeView->setEnabled(false);
		}
	}

	if(this->isSearchMode) {
		this->ui->searchTreeView->selectionModel()->clear();

		if(!force && text.size() < 3)
			Unicoder::databaseLoader()->clearSearchModel(this->symbolModel, this->ui->findAliasCheckBox->isChecked());
		else {
			QString pattern = text;
			pattern.replace(QLatin1Char('*'), QLatin1Char('%'));
			pattern.replace(QLatin1Char('?'), QLatin1Char('_'));
			Unicoder::databaseLoader()->searchName(pattern,
												   this->mode,
												   this->ui->findAliasCheckBox->isChecked(),
												   this->symbolModel);
		}
	} else {
		this->currentCode = this->calcUnicode(text);
		this->updateSymbol();
	}
}

void SymbolSelectDialog::updateSymbol()
{
	if(this->currentCode != UINT_MAX) {
		QString symbol = Unicoder::code32ToSymbol(this->currentCode);
		QString name = Unicoder::databaseLoader()->nameForSymbol(this->currentCode);
		this->ui->previewLabel->setText(symbol);
		this->ui->previewLabel->setToolTip(name);
		this->ui->previewLabel->setCursor(Qt::OpenHandCursor);
		this->ui->actionCopy_Symbol->setEnabled(true);
		this->ui->actionCopy_Symbol_Name->setEnabled(!name.isEmpty());
		this->ui->actionCopy_symbol_unicode_codepoint->setEnabled(true);
		this->ui->actionCopy_symbol_HTML_code->setEnabled(true);
		this->ui->actionShow_Symbol_Info->setEnabled(true);
	} else {
		this->ui->previewLabel->clear();
		this->ui->previewLabel->setToolTip(QString());
		this->ui->previewLabel->setCursor(Qt::ForbiddenCursor);
		this->ui->actionCopy_Symbol->setEnabled(false);
		this->ui->actionCopy_Symbol_Name->setEnabled(false);
		this->ui->actionCopy_symbol_unicode_codepoint->setEnabled(false);
		this->ui->actionCopy_symbol_HTML_code->setEnabled(false);
		this->ui->actionShow_Symbol_Info->setEnabled(false);
	}
}



ExtendedMenuLineEdit::ExtendedMenuLineEdit(QWidget *parent) :
	QLineEdit(parent)
{}

void ExtendedMenuLineEdit::contextMenuEvent(QContextMenuEvent *ev)
{
	QMenu *menu = this->createStandardContextMenu();
	menu->addSeparator();
	menu->addActions(this->actions());
	menu->exec(ev->globalPos());
	menu->deleteLater();
}



DragLabel::DragLabel(QDialog *parent) :
	QLabel(parent),
	dragStartPosition()
{
	Q_ASSERT(dynamic_cast<SymbolSelectDialog*>(parent));
}

void DragLabel::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton) {
		this->dragStartPosition = event->pos();
		if(!this->text().isNull())
			this->setCursor(Qt::ClosedHandCursor);
	}
	event->accept();
}

void DragLabel::mouseReleaseEvent(QMouseEvent *event)
{
	if(!this->text().isNull())
		this->setCursor(Qt::OpenHandCursor);
	event->accept();
}

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
	this->setCursor(Qt::OpenHandCursor);
	event->accept();
}

void DragLabel::mouseDoubleClickEvent(QMouseEvent *event)
{
	if(this->text().isNull())
		return;
	Q_ASSERT(dynamic_cast<SymbolSelectDialog*>(this->parentWidget()));
	SymbolSelectDialog *diag = static_cast<SymbolSelectDialog*>(this->parentWidget());
	if(diag->doInsert)
		Unicoder::sendSymbolInput(this->text());
	diag->accept();
	event->accept();
}
