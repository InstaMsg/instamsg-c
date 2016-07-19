sudo kill `pgrep -x instamsg`

HOME_DIRECTORY="/home/sensegrow"

sudo mkdir -p "${HOME_DIRECTORY}"
sudo chmod -R 777 "${HOME_DIRECTORY}"

sudo cp monitor.sh "${HOME_DIRECTORY}"
sudo chmod 777 "${HOME_DIRECTORY}/monitor.sh"

sudo cp $1 "${HOME_DIRECTORY}/instamsg"
sudo chmod 777 "${HOME_DIRECTORY}/instamsg"

sudo touch "${HOME_DIRECTORY}/data.txt"
sudo chmod 777 "${HOME_DIRECTORY}/data.txt"
