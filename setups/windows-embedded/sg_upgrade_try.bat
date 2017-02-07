@echo off
setlocal enabledelayedexpansion

call upgrade_params.bat


set PID=
for /f "tokens=* delims=" %%i in ('tasklist /v ^| findstr /l sg_upgrade.bat ^| find /c "sg_upgrade.bat"') do set PID=!PID!^%%i

if %PID% EQU 0 (
    	echo Safe to proceed to stage 1

	set PID=
	for /f "tokens=* delims=" %%i in ('tasklist /v ^| findstr /l sg_setup.bat.bat ^| find /c "sg_setup.bat"') do set PID=!PID!^%%i

	if %PID% EQU 0 (
	    	echo Safe to proceed to stage 2

		start sg_upgrade.bat
		exit /b
	)

        echo "An instance of sg_setup.bat already running, bye bye"
	exit
)

echo "An instance of sg_upgrade.bat already running, bye bye"
exit
