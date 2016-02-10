#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

namespace Ui {
	class SettingsDialog;
}

class SettingsDialog : public QDialog
{
	Q_OBJECT

public:
	explicit SettingsDialog(QWidget *parent = 0);
	~SettingsDialog();

public slots:
	void showSettings();
	void accept() Q_DECL_OVERRIDE;

signals:
	void settingsChanged();

private:
	Ui::SettingsDialog *ui;
};

#endif // SETTINGSDIALOG_H
