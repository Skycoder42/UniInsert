TEMPLATE = subdirs

SUBDIRS += UnicodeUtil \
	DatabaseUpdater \
	Setup

DISTFILES += \
	LICENSE \
	README.md \
	localDB/unicodeutil_defaultdb.qrc \
	build_deploy/win/deploy.bat \
	build_deploy/win/setup.bat


#custom target to generate the rcc-file
localDB.target = localDB
localDB.commands = rcc -compress 9 -binary $$PWD/localDB/unicodeutil_defaultdb.qrc -o ./defaultDatabase.rcc
QMAKE_EXTRA_TARGETS += localDB

translate.target = translate
translate.commands += @if not exist UnicodeUtil exit 1 $$escape_expand(\n\t) \
	@if not exist DatabaseUpdater exit 1 $$escape_expand(\n\t) \
	@set MAKEFLAGS=$(MAKEFLAGS) $$escape_expand(\n\t) \
	$(MAKE) -f UnicodeUtil/Makefile translate $$escape_expand(\n\t) \
	$(MAKE) -f DatabaseUpdater/Makefile translate
QMAKE_EXTRA_TARGETS += translate
