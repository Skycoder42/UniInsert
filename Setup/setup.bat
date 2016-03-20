:: working directory: %{sourceDir}
@echo off

mkdir ./build
mkdir ./build/repositories

repogen.exe --update-new-components -p ./packages ./build/repositories/win_x64
binarycreator.exe -n -c ./config/config.xml -p ./packages ./build/Unicode_Utility_1.1.0_setup_win_x64.exe
