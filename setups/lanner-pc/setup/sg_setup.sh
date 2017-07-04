#!/bin/sh

. ../upgrade_params
PKG_VERSION="15"

###################### PERFORM ACTIONS NOW ##################################

crontab < cron

/usr/bin/killall monitor.sh || true
/usr/bin/killall instamsg || true

sed -i 's/#FSCKFIX=no/FSCKFIX=yes/g' /etc/default/rcS

cp upgrade_params ${HOME_DIRECTORY}
cp -f sg_upgrade.sh ${HOME_DIRECTORY}
cp energy-meter_lanner-pc_9.5.0_9.5.0 ${HOME_DIRECTORY}/instamsg
