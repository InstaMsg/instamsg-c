sudo kill `pgrep -x pi`

HOME_DIRECTORY="/home/sensegrow"

sudo mkdir -p "${HOME_DIRECTORY}"
sudo chmod -R 777 "${HOME_DIRECTORY}"

sudo cp reset.sh "${HOME_DIRECTORY}"
sudo chmod 777 "${HOME_DIRECTORY}/reset.sh"

sudo cp monitor.sh "${HOME_DIRECTORY}"
sudo chmod 777 "${HOME_DIRECTORY}/monitor.sh"

sudo cp $1 "${HOME_DIRECTORY}/pi"
sudo chmod 777 "${HOME_DIRECTORY}/pi"

sudo crontab < cron
