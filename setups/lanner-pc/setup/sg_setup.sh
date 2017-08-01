#!/bin/sh

. ../upgrade_params
PKG_VERSION="19"

###################### PERFORM ACTIONS NOW ##################################

crontab < cron

/usr/bin/killall monitor.sh || true
/usr/bin/killall instamsg || true

sed -i 's/#FSCKFIX=no/FSCKFIX=yes/g' /etc/default/rcS

cp upgrade_params ${HOME_DIRECTORY}
cp -f sg_upgrade.sh ${HOME_DIRECTORY}
cp energy-meter_lanner-pc_10.0.0_10.0.1 ${HOME_DIRECTORY}/instamsg
