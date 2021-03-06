@echo off
setlocal enabledelayedexpansion

set HOME_DIRECTORY=c:\sensegrow
set upgrade_timer=0
set log_file_timer=0

:loop

set PID=
for /f "tokens=* delims=" %%i in ('tasklist ^| findstr /b /l instamsg.exe ^| find /c "instamsg.exe"') do set PID=!PID!^%%i

if %PID% EQU 0 (
	echo Binary not running
	cd %HOME_DIRECTORY%
    del /F instamsg.log
	start /B instamsg
)

if %PID% EQU 1 (
	echo Binary running fine
)

timeout 60 /nobreak

set /a upgrade_timer=%upgrade_timer%+60
if %upgrade_timer% GTR 3600 (
	cd %HOME_DIRECTORY%
	call sg_upgrade_try.bat

	set upgrade_timer=0
)

set /a log_file_timer=%log_file_timer%+60
if %log_file_timer% GTR 36000 (
	cd %HOME_DIRECTORY%
    taskkill /IM instamsg.exe /F
	set log_file_timer=0
)

goto loop

