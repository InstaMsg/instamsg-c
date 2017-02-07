call upgrade_params.bat

cd %HOME_DIRECTORY

>>auto_upgrade type nul
set enabled=
for /f "tokens=* delims=" %%i in ('type auto_upgrade') do set enabled=!enabled!^%%i

if %enabled% EQU 0 (
    	echo Upgrade is disabled ... aborting ..
	exit
)

rm -f version.txt
wget "${VERSION_URL}"


set prev_version=
for /f "tokens=* delims=" %%i in ('type current_version') do set prev_version=!prev_version!^%%i
if %prev_version% EQU 0 (
    	echo Current-Version not available ... aborting ..
    	exit
}

set new_version=
for /f "tokens=* delims=" %%i in ('type version.txt') do set new_version=!new_version!^%%i
if %new_version% EQU 0 (
    	echo New Package-Version not available ... aborting ..
	exit
}

if %new_version% GTR %prev_version% (
    	echo Upgrade will proceed now ...

	rm -f setup.zip
	rmdir /S setup

	wget %PLATFORM%
	%EXTRACT_COMMAND%

	call sg_setup.bat

	echo %new_version% > ..\current_version
	%REBOOT_COMMAND%

	exit
)

echo Upgrade did not proceed because the new version is not greater !!

