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
sed -i '/^[ :\t]*\/home\/sensegrow\/monitor.sh \&/d' /etc/rc.local
sed -i 's/^[ :\t]*exit 0/\/home\/sensegrow\/monitor.sh \&\nexit 0/g' /etc/rc.local

sed -i 's/#FSCKFIX=no/FSCKFIX=yes/g' /etc/default/rcS

sed -i '/^[ :\t]*insmod \/lib\/modules\/3.19.8-031908-generic\/kernel\/drivers\/watchdog\/wd_drv.ko/d' /etc/rc.local
sed -i 's/^[ :\t]*exit 0/insmod \/lib\/modules\/3.19.8-031908-generic\/kernel\/drivers\/watchdog\/wd_drv.ko\nexit 0/g' /etc/rc.local
cp wd_drv.ko /lib/modules/3.19.8-031908-generic/kernel/drivers/watchdog
chmod 644 /lib/modules/3.19.8-031908-generic/kernel/drivers/watchdog/wd_drv.ko
sed -i '/^[ :\t]*wd_drv/d' /etc/modules
echo wd_drv >> /etc/modules

rm -f /etc/udev/rules.d/70-persistent-net.rules

touch "${HOME_DIRECTORY}/data.txt"
chmod 777 "${HOME_DIRECTORY}/data.txt"

cp wd_tst "${HOME_DIRECTORY}"
chmod 777 "${HOME_DIRECTORY}/wd_tst"

cp ../../sg_upgrade_try.sh "${HOME_DIRECTORY}"
chmod 777 "${HOME_DIRECTORY}/sg_upgrade_try.sh"

cp ../../sg_upgrade.sh "${HOME_DIRECTORY}"
chmod 777 "${HOME_DIRECTORY}/sg_upgrade.sh"

cp upgrade_params "${HOME_DIRECTORY}"
chmod 777 "${HOME_DIRECTORY}/upgrade_params"

echo 15 > ${HOME_DIRECTORY}/current_version

kill -9 `pgrep -x wwan-monitor` || true
cp wwan-monitor "${HOME_DIRECTORY}"
chmod 777 "${HOME_DIRECTORY}/wwan-monitor"

echo test > ${HOME_DIRECTORY}/prov.txt

crontab < cron

sed -i 's/GRUB_CMDLINE_LINUX_DEFAULT=.*/GRUB_CMDLINE_LINUX_DEFAULT="quiet splash intel_idle.max_cstate=0 processor.max_cstate=0 idle=poll"/g' /etc/default/grub
update-grub

apt-get -y remove pm-utils

kill -9 `pgrep -x instamsg` || true
cp $1 "${HOME_DIRECTORY}/instamsg"
chmod 777 "${HOME_DIRECTORY}/instamsg"

echo
echo
echo "FIRST STAGE OF SETUP COMPLETE. NOW PLEASE PROCEED TO SETUP UDEV-RULES FOR USB-SERIAL-CABLES AS PER THE DOCUMENTATION."
