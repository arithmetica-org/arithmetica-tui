@echo off
setlocal

REM Set default install prefix to C:\Windows\System32
set PREFIX=C:\Windows\System32

:: BatchGotAdmin
:-------------------------------------
REM  --> Check for permissions
    IF "%PROCESSOR_ARCHITECTURE%" EQU "amd64" (
>nul 2>&1 "%SYSTEMROOT%\SysWOW64\cacls.exe" "%SYSTEMROOT%\SysWOW64\config\system"
) ELSE (
>nul 2>&1 "%SYSTEMROOT%\system32\cacls.exe" "%SYSTEMROOT%\system32\config\system"
)

REM --> If error flag set, we do not have admin.
if '%errorlevel%' NEQ '0' (
    echo Requesting administrative privileges...
    goto UACPrompt
) else ( goto gotAdmin )

:UACPrompt
    echo Set UAC = CreateObject^("Shell.Application"^) > "%temp%\getadmin.vbs"
    set params= %*
    echo UAC.ShellExecute "cmd.exe", "/c ""%~s0"" %params:"=""%", "", "runas", 1 >> "%temp%\getadmin.vbs"

    "%temp%\getadmin.vbs"
    del "%temp%\getadmin.vbs"
    exit /B

:gotAdmin
    pushd "%CD%"
    CD /D "%~dp0"
:--------------------------------------    

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

REM Download the latest prerelease
for /f "usebackq delims=" %%A in (`powershell -command "$response = Invoke-WebRequest -Uri 'https://api.github.com/repos/avighnac/arithmetica-tui/releases'; $json = ConvertFrom-Json -InputObject $response.Content; $prerelease = $json | Where-Object { $_.prerelease -eq $true } | Select-Object -First 1; if ($prerelease) { Write-Output $prerelease.tag_name }"`) do (
  set "TAG=%%A"
)
echo Downloading the latest prerelease: %TAG%
powershell -command "Invoke-WebRequest -Uri 'https://github.com/avighnac/arithmetica-tui/releases/download/%TAG%/arithmetica.exe' -OutFile 'arithmetica.exe'"

echo Successfully downloaded the latest release.

REM Copy the executable to %PREFIX%\arithmetica.exe
copy arithmetica.exe "%PREFIX%\arithmetica.exe"

echo Copied successfully!

REM Cleanup
cd ..
rmdir /s /q arithmetica-tui-install

exit /b 0
