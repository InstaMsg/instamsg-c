@echo off
setlocal enabledelayedexpansion

set HOME_DIRECTORY=c:\sensegrow

mkdir %HOME_DIRECTORY%

copy -f monitor.bat  %HOME_DIRECTORY%
echo test > %HOME_DIRECTORY%\prov.txt

copy -f curl.exe  %HOME_DIRECTORY%

copy -f upgrade_params.bat  %HOME_DIRECTORY%
copy -f sg_upgrade_try.bat  %HOME_DIRECTORY%
copy -f sg_upgrade.bat  %HOME_DIRECTORY%
copy -f upgrade_params.bat  %HOME_DIRECTORY%
echo 0 > %HOME_DIRECTORY%\current_version

>>data.txt type nul

copy -f baretail.exe %HOME_DIRECTORY%
echo > %HOME_DIRECTORY%\instamsg.log

taskkill /IM monitor.bat /F
taskkill /IM instamsg.exe /F

copy -f instamsg.lnk "C:\ProgramData\Microsoft\Windows\Start Menu\Programs\Startup"

copy -f %1.exe %HOME_DIRECTORY%
move %HOME_DIRECTORY%\%1.exe %HOME_DIRECTORY%\instamsg.exe
copy -f %1.ipdb %HOME_DIRECTORY%
move %HOME_DIRECTORY%\%1.ipdb %HOME_DIRECTORY%\instamsg.ipdb
copy -f %1.iobj %HOME_DIRECTORY%
move %HOME_DIRECTORY%\%1.iobj %HOME_DIRECTORY%\instamsg.iobj
copy -f %1.pdb %HOME_DIRECTORY%
move %HOME_DIRECTORY%\%1.pdb %HOME_DIRECTORY%\instamsg.pdb
