@echo off
setlocal

REM Set default command to download archive from
set DOWNLOAD_CMD=powershell -command "(New-Object System.Net.WebClient).DownloadFile"

REM Set default install prefix to C:\Program Files
set PREFIX=C:\Program Files

REM Check if the user is running as an administrator
@REM NET SESSION >NUL 2>&1
@REM if %ERRORLEVEL% NEQ 0 (
@REM   echo Please run as administrator!
@REM   exit /b 1
@REM )

REM Check for PowerShell and Invoke-WebRequest
echo Testing for PowerShell...
where powershell >NUL 2>&1
if %ERRORLEVEL% NEQ 0 (
  echo PowerShell is not installed!
  exit /b 1
)

echo Testing for Invoke-WebRequest...
powershell -command "if (!(Get-Command Invoke-WebRequest -ErrorAction SilentlyContinue)) { exit 1 }"
if "%ERRORLEVEL%" NEQ "0" (
  echo Invoke-WebRequest is not available!
  exit /b 1
)

cd %TEMP%
REM If the directory exists, delete it
if exist arithmetica-tui-install rmdir /s /q arithmetica-tui-install
mkdir arithmetica-tui-install
cd arithmetica-tui-install

REM Download the latest release
%DOWNLOAD_CMD https://github.com/avighnac/arithmetica-tui/releases/latest/download/arithmetica.exe arithmetica.exe

echo Successfully downloaded the latest release.

REM Copy the executable to %PREFIX%\arithmetica.exe
copy arithmetica.exe "%PREFIX%\arithmetica.exe"

echo Copied successfully!

REM Cleanup
cd ..
rmdir /s /q arithmetica-tui-install

exit /b 0
