sudo kill `pgrep -x instamsg`

HOME_DIRECTORY="/home/sensegrow"

sudo mkdir -p "${HOME_DIRECTORY}"
sudo chmod -R 777 "${HOME_DIRECTORY}"

sudo cp monitor.sh "${HOME_DIRECTORY}"
sudo chmod 777 "${HOME_DIRECTORY}/monitor.sh"
sudo sed -i '/^[ :\t]*\/home\/sensegrow\/monitor.sh \&/d' /etc/rc.local
sudo sed -i 's/^[ :\t]*exit 0/\/home\/sensegrow\/monitor.sh \&\nexit 0/g' /etc/rc.local

sudo cp $1 "${HOME_DIRECTORY}/instamsg"
sudo chmod 777 "${HOME_DIRECTORY}/instamsg"

sudo touch "${HOME_DIRECTORY}/data.txt"
sudo chmod 777 "${HOME_DIRECTORY}/data.txt"

sudo echo test > ${HOME_DIRECTORY}/prov.txt

sudo crontab < cron
