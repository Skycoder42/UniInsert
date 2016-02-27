#ifndef RESETDATABASEDIALOG_H
#define RESETDATABASEDIALOG_H

#include <QDialog>
#include <QButtonGroup>
#include <QNetworkAccessManager>

namespace Ui {
	class ResetDatabaseDialog;
}

class ResetDatabaseDialog : public QDialog
{
	Q_OBJECT

public:
	explicit ResetDatabaseDialog(QWidget *parent = Q_NULLPTR);
	~ResetDatabaseDialog();

	static bool tryReset(QWidget *parent, bool installOnly = false);

	void accept() Q_DECL_OVERRIDE;

private slots:
	void ftpListingReady(QNetworkReply *reply);

private:
	Ui::ResetDatabaseDialog *ui;
	QButtonGroup *modeGroup;

	QNetworkAccessManager *nam;
};

#endif // RESETDATABASEDIALOG_H
