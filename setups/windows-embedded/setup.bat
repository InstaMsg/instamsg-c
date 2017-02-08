@echo off
setlocal enabledelayedexpansion

if not exist "C:\Program Files\GnuWin32\bin\unzip.exe" (
	echo "unzip.exe is not present, please install it."
	exit /b
)

set HOME_DIRECTORY=c:\sensegrow

mkdir %HOME_DIRECTORY%

copy /Y monitor.bat  %HOME_DIRECTORY%
echo test > %HOME_DIRECTORY%\prov.txt

copy /Y curl.exe  %HOME_DIRECTORY%

copy /Y upgrade_params.bat  %HOME_DIRECTORY%
copy /Y sg_upgrade_try.bat  %HOME_DIRECTORY%
copy /Y sg_upgrade.bat  %HOME_DIRECTORY%
copy /Y upgrade_params.bat  %HOME_DIRECTORY%
echo 0 > %HOME_DIRECTORY%\current_version

>>data.txt type nul

copy /Y baretail.exe %HOME_DIRECTORY%
echo > %HOME_DIRECTORY%\instamsg.log

taskkill /IM monitor.bat /F
taskkill /IM instamsg.exe /F

copy /Y instamsg.lnk "C:\ProgramData\Microsoft\Windows\Start Menu\Programs\Startup"

copy /Y %1.exe %HOME_DIRECTORY%
move %HOME_DIRECTORY%\%1.exe %HOME_DIRECTORY%\instamsg.exe
copy /Y %1.ipdb %HOME_DIRECTORY%
move %HOME_DIRECTORY%\%1.ipdb %HOME_DIRECTORY%\instamsg.ipdb
copy /Y %1.iobj %HOME_DIRECTORY%
move %HOME_DIRECTORY%\%1.iobj %HOME_DIRECTORY%\instamsg.iobj
copy /Y %1.pdb %HOME_DIRECTORY%
move %HOME_DIRECTORY%\%1.pdb %HOME_DIRECTORY%\instamsg.pdb

echo "Setup completed successfully ...."
