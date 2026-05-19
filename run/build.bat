@echo off
:: Build script for XFileUnpacker (console)
:: Requires: Qt 5.15.2 msvc2019_64, Visual Studio 2022, CMake, Ninja

set BUILD_DIR=%TEMP%\qt5_xfileunpacker_build
set QT_DIR=C:\Qt\5.15.2\msvc2019_64
set VCVARS="C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
set SOURCE_DIR=%~dp0..

echo === Initializing MSVC environment ===
call %VCVARS% || (echo ERROR: Could not initialize MSVC environment & exit /b 1)

echo === Configuring CMake ===
cmake -S "%SOURCE_DIR%" -B "%BUILD_DIR%" -G Ninja -DCMAKE_PREFIX_PATH="%QT_DIR%" -DBUILD_GUI=OFF
if errorlevel 1 (echo ERROR: CMake configuration failed & exit /b 1)

echo === Building console target ===
cmake --build "%BUILD_DIR%" --target xfileunpackerc
if errorlevel 1 (echo ERROR: Build failed & exit /b 1)

echo === Build successful ===
echo Executable: %BUILD_DIR%\src\console\xfileunpackerc.exe
