:: Creates deployment directory will all necessary files (unordered)
:: %1: %{CurrentBuild:Type}
:: workingdir: %{buildDir}
mkdir .\deploy

xcopy .\UnicodeUtil\%1\UnicodeUtil.exe .\deploy\* /y
xcopy .\DatabaseUpdater\%1\DatabaseUpdater.exe .\deploy\* /y
xcopy .\defaultDatabase.rcc .\deploy\* /y

windeployqt -%1 .\deploy\DatabaseUpdater.exe
windeployqt -%1 .\deploy\UnicodeUtil.exe
