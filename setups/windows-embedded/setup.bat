@echo off
setlocal enabledelayedexpansion

set HOME_DIRECTORY=c:\sensegrow

mkdir %HOME_DIRECTORY%

cp -f monitor.bat  %HOME_DIRECTORY%
echo test > %HOME_DIRECTORY%\prov.txt

cp -f curl.exe  %HOME_DIRECTORY%

cp -f upgrade_params.bat  %HOME_DIRECTORY%
cp -f sg_upgrade_try.bat  %HOME_DIRECTORY%
cp -f sg_upgrade.bat  %HOME_DIRECTORY%
cp -f upgrade_params.bat  %HOME_DIRECTORY%
echo 0 > %HOME_DIRECTORY%\current_version

cp -f baretail.exe %HOME_DIRECTORY%
echo > %HOME_DIRECTORY%\instamsg.log

taskkill /IM monitor.bat /F
taskkill /IM instamsg.exe /F

cp -f %1.exe %HOME_DIRECTORY%
mv %HOME_DIRECTORY%\%1.exe %HOME_DIRECTORY%\instamsg.exe
cp -f %1.ipdb %HOME_DIRECTORY%
mv %HOME_DIRECTORY%\%1.ipdb %HOME_DIRECTORY%\instamsg.ipdb
cp -f %1.iobj %HOME_DIRECTORY%
mv %HOME_DIRECTORY%\%1.iobj %HOME_DIRECTORY%\instamsg.iobj
cp -f %1.pdb %HOME_DIRECTORY%
mv %HOME_DIRECTORY%\%1.pdb %HOME_DIRECTORY%\instamsg.pdb
