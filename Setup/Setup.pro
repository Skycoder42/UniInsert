DISTFILES += \
	config/config.xml \
    config/controllerScript.js \
	packages/com.SkyCoder42.AdvancedSetup/meta/package.xml \
	packages/com.SkyCoder42.AdvancedSetup/meta/install.js \
	packages/com.SkyCoder42.AdvancedSetup/meta/ShortcutPage.ui \
	packages/com.SkyCoder42.AdvancedSetup/meta/UserPage.ui \
	packages/com.SkyCoder42.AdvancedSetup/meta/de.qm \
	packages/com.SkyCoder42.AdvancedSetup/data/regSetUninst.bat \
	packages/com.microsoft.vcredist.x64/meta/package.xml \
	packages/com.microsoft.vcredist.x64/meta/install.js \
	packages/com.microsoft.vcredist.x64/data/vcredist_x64.exe \
    packages/com.SykCoder42.UniInsert/meta/package.xml \
	packages/com.SykCoder42.UniInsert/meta/LICENSE.txt \
    packages/com.SykCoder42.UniInsert.Core/meta/package.xml \
    packages/com.SykCoder42.UniInsert.DatabaseUpdater/meta/package.xml \
	packages/com.SykCoder42.UniInsert.DefaultDatabase/meta/package.xml

setup.target = setup
win32 {#TODO!!!
	setup.commands += $$_PRO_FILE_PWD_/../build_deploy/win/setup.bat $$escape_expand(\n\t)
	setup.commands += repogen.exe --update-new-components -p $$_PRO_FILE_PWD_/packages ./Setup/repositories/win_x64 $$escape_expand(\n\t)
	setup.commands += binarycreator.exe -n -c $$_PRO_FILE_PWD_/config/config.xml -p $$_PRO_FILE_PWD_/packages ./Unicode_Utility_1.1.0_setup_win_x64.exe
}
QMAKE_EXTRA_TARGETS += setup
