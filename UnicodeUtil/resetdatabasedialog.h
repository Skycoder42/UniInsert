#ifndef RESETDATABASEDIALOG_H
#define RESETDATABASEDIALOG_H

#include <QDialog>
#include <QButtonGroup>

namespace Ui {
	class ResetDatabaseDialog;
}

class ResetDatabaseDialog : public QDialog
{
	Q_OBJECT

public:
	explicit ResetDatabaseDialog(QWidget *parent = Q_NULLPTR);
	~ResetDatabaseDialog();

	static bool tryReset(QWidget *parent);

private:
	Ui::ResetDatabaseDialog *ui;
	QButtonGroup *modeGroup;
};

#endif // RESETDATABASEDIALOG_H
