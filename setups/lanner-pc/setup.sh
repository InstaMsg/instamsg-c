set -x
set -e

dpkg -i linux-headers-3.19*.deb linux-image-3.19*.deb
update-grub

add-apt-repository -y ppa:aleksander-m/modemmanager-trusty
apt-get update
apt-get install -y modemmanager

touch /etc/udev/rules.d/99-usb-serial.rules
chmod 777 /etc/udev/rules.d/99-usb-serial.rules

HOME_DIRECTORY="/home/sensegrow"

mkdir -p "${HOME_DIRECTORY}"
chmod -R 777 "${HOME_DIRECTORY}"

cp monitor.sh "${HOME_DIRECTORY}"
chmod 777 "${HOME_DIRECTORY}/monitor.sh"

kill -9 `pgrep -x instamsg` || true
cp $1 "${HOME_DIRECTORY}/instamsg"
chmod 777 "${HOME_DIRECTORY}/instamsg"

touch "${HOME_DIRECTORY}/data.txt"
chmod 777 "${HOME_DIRECTORY}/data.txt"

kill -9 `pgrep -x wwan-monitor` || true
cp wwan-monitor "${HOME_DIRECTORY}"
chmod 777 "${HOME_DIRECTORY}/wwan-monitor"

echo test > ${HOME_DIRECTORY}/prov.txt

crontab < cron

echo
echo
echo "EVERYTHING COMPLETED SUCCESSFULLY ... PLEASE SHUTDOWN THE MACHINE, CONNECT ALL SERIAL-CABLES, AND THEN POWER-UP."
