@echo off
setlocal enabledelayedexpansion

call ..\upgrade_params.bat

REM ###################### PERFORM ACTIONS NOW #########################

timeout 5 /nobreak

copy /Y monitor.bat  %HOME_DIRECTORY%
exit /b

