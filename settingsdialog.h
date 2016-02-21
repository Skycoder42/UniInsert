#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QVariant>
#include <QSettings>
class QAbstractButton;

#define DECL_SETTINGS_CODE(code) \
	static const QString code;\
	static const QVariant code ## Default;
#define SETTINGS_VALUE(code) QSettings().value(code, code ## Default)

namespace Ui {
	class SettingsDialog;
}

class SettingsDialog : public QDialog
{
	Q_OBJECT

public:
	DECL_SETTINGS_CODE(useClip)
	DECL_SETTINGS_CODE(allClip)
	DECL_SETTINGS_CODE(autoHide)
	DECL_SETTINGS_CODE(transparency)
	DECL_SETTINGS_CODE(maxRecent)
	DECL_SETTINGS_CODE(autoStart)
	DECL_SETTINGS_CODE(reset)

	explicit SettingsDialog(QWidget *parent = 0);
	~SettingsDialog();

	static void storeSize(QWidget *widget);
	static void loadSize(QWidget *widget);

public slots:
	void showSettings();
	void accept() Q_DECL_OVERRIDE;

	static void showAboutDialog();

signals:
	void settingsChanged();

private slots:
	void on_buttonBox_clicked(QAbstractButton *button);
	void on_resetRecentButton_clicked();

	void updateAutostart(bool on);
private:
	Ui::SettingsDialog *ui;
};

#endif // SETTINGSDIALOG_H
