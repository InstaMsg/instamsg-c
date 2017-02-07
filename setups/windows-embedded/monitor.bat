@echo off
setlocal enabledelayedexpansion

set HOME_DIRECTORY=c:\sensegrow


set PID=
for /f "tokens=* delims=" %%i in ('tasklist ^| findstr /b /l instamsg.exe ^| find /c "instamsg.exe"') do set PID=!PID!^%%i

if %PID% EQU 0 (
	echo Binary not running
	cd %HOME_DIRECTORY%
	start /B instamsg
)

if %PID% EQU 1 (
	echo Binary running fine
)

