@echo off
setlocal
:: Pristine build script for XFileUnpacker (console).
:: A fresh directory is intentional: restored _mylibs trees can carry source
:: mtimes older than existing objects, which no timestamp-based build tool can
:: distinguish from an up-to-date build.
:: Requires: Qt 5/6 MSVC, Visual Studio 2022, CMake, Ninja.

set "BUILD_DIR=%TEMP%\xfileunpacker_pristine_build"
if not defined QT_DIR (
    if exist "C:\Qt\6.11.0\msvc2022_64\lib\cmake\Qt6" (
        set "QT_DIR=C:\Qt\6.11.0\msvc2022_64"
    ) else (
        set "QT_DIR=C:\Qt\5.15.2\msvc2019_64"
    )
)
set "VCVARS=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
set "SOURCE_DIR=%~dp0.."

echo === Initializing MSVC environment ===
call "%VCVARS%" || (echo ERROR: Could not initialize MSVC environment & exit /b 1)

echo === Removing the fixed temporary build directory ===
cmake -E remove_directory "%BUILD_DIR%"
if exist "%BUILD_DIR%" (echo ERROR: Could not remove %BUILD_DIR% & exit /b 1)

echo === Configuring CMake ===
cmake -S "%SOURCE_DIR%" -B "%BUILD_DIR%" -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="%QT_DIR%" -DBUILD_GUI=OFF
if errorlevel 1 (echo ERROR: CMake configuration failed & exit /b 1)

echo === Building console target ===
cmake --build "%BUILD_DIR%" --target xfileunpackerc
if errorlevel 1 (echo ERROR: Build failed & exit /b 1)

echo === Build successful ===
echo Executable: %BUILD_DIR%\src\console\xfileunpackerc.exe
