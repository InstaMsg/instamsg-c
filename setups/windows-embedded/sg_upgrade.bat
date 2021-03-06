@echo off
setlocal enabledelayedexpansion

call upgrade_params.bat

>>auto_upgrade type nul
set enabled=
for /f "tokens=* delims=" %%i in ('type auto_upgrade') do set enabled=!enabled!^%%i

if "%enabled%" == "" (
	set enabled=1
)

if %enabled% EQU 0 (
    	echo Upgrade is disabled ... aborting ..

	timeout 5 /nobreak
	exit /b
)

>>version.txt type nul
del /F version.txt
.\curl.exe -s -O "%VERSION_URL%" || exit /b


set prev_version=
for /f "tokens=* delims=" %%i in ('type current_version') do set prev_version=!prev_version!^%%i

set new_version=
for /f "tokens=* delims=" %%i in ('type version.txt') do set new_version=!new_version!^%%i

if %new_version% GTR %prev_version% (
    	echo Upgrade will proceed now ...

	del /F setup.zip
	rmdir /S /Q setup

	.\curl.exe -s -O "%PLATFORM%" || exit /b
	"C:\Program Files\GnuWin32\bin\unzip" setup.zip || exit /b


	cd setup || exit /b
	call sg_setup.bat

	echo %new_version% > ..\current_version
	echo "Upgrade completed successfully, rebooting in 5 seconds ..."

	timeout 5 /nobreak
	shutdown /r

	exit /b
)

echo "Upgrade did not proceed because the new version is not greater !!"
timeout 5 /nobreak

exit /b
