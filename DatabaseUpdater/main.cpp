#include "updaterwindow.h"
#include <QApplication>
#include <QIcon>
#include <QThread>
#include <qsingleinstance.h>
#include <dialogmaster.h>
#include <global.h>

int main(int argc, char *argv[])
{
	//Arguments: local DB path, new version, single-instance-key, update-mode
	QApplication a(argc, argv);
	QApplication::setApplicationName(TARGET);
	QApplication::setApplicationVersion(VERSION);
	QApplication::setOrganizationName(COMPANY);
	QApplication::setApplicationDisplayName(DISPLAY_NAME);
	QApplication::setWindowIcon(QIcon(QStringLiteral(":/icons/main.ico")));

	if(QApplication::arguments().size() != 5) {
		DialogMaster::critical(Q_NULLPTR,
							   QApplication::translate("GLOBAL", "The program requires the command line arguments:\n"
																 "   •  Path to the local unicode database\n"
																 "   •  Unicode version to update to\n"
																 "   •  The key of the Unicode Utility single instance\n"
																 "   •  The updater mode flags (0-3)"),
							   QApplication::translate("GLOBAL", "Invalid commandline arguments!"));
		return EXIT_FAILURE;
	} else {
		QSingleInstance instance;
		instance.setInstanceID(ARG_INSTANCE_KEY);

		do {
			QThread::msleep(500);
			instance.process(QStringList());
		} while(!instance.isMaster());

		UpdaterWindow w;
		instance.setNotifyWindow(&w);

		QObject::connect(&instance, &QSingleInstance::instanceMessage, &w, [&](QStringList){
			DialogMaster::warning(&w,
								  QApplication::translate("GLOBAL", "You can't use the Unicode Utility while updating the database!\n"
																	"Please wait until the update has been finished."));
		});

		w.show();
		return a.exec();
	}
}
