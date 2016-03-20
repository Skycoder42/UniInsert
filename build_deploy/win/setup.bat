:: %1: %{Qt:QT_INSTALL_BINS}
:: %2: path to Qt-Installer-Framwork
:: %3: %{sourceDir}
:: working directory: %{buildDir}
@echo off



"%~2/bin/repogen.exe" --update-new-components -p %3/Setup/packages ./Setup/repositories/win_x64
"%~2/bin/binarycreator.exe" -n -c %3/Setup/config/config.xml -p %3/Setup/packages ./Unicode_Utility_1.1.0_setup_win_x64.exe
