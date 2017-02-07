call upgrade_params.bat

set FIRST_PID=
for /f "tokens=* delims=" %%i in ('tasklist /v | findstr /l sg_upgrade.bat | find /c "sg_upgrade.bat"') do set FIRST_PID=!FIRST_PID!^%%i

if %FIRST_PID% EQU 0 (
    	echo "Safe to proceed (stage 1) ..."

	set SECOND_PID=
	for /f "tokens=* delims=" %%i in ('tasklist /v | findstr /l sg_setup.bat.bat | find /c "sg_setup.bat"') do set SECOND_PID=!SECOND_PID!^%%i

	if %SECOND_PID% EQU 0 (
	    	echo "Safe to proceed (stage 2) ..."

	    	cd %HOME_DIRECTORY%
		start /B sg_upgrade.bat
	)
)

