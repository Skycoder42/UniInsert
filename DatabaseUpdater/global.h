#ifndef GLOBAL_H
#define GLOBAL_H

#include <QApplication>
#include "updaterwindow.h"

#define ARG_LOCAL_DB_PATH QApplication::arguments()[1]
#define ARG_UPDATE_VERSION QApplication::arguments()[2]
#define ARG_INSTANCE_KEY QApplication::arguments()[3]
#define ARG_UPDATE_MODE UpdaterWindow::UpdateFlags(QApplication::arguments()[4].toInt())

template<typename... Args> struct SELECT {
	template<typename C, typename R>
	static constexpr auto OVERLOAD_OF( R (C::*pmf)(Args...) ) -> decltype(pmf) {
		return pmf;
	}
};

//TODO
#include <QDebug>

#endif // GLOBAL_H
