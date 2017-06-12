#!/bin/sh

. ../upgrade_params
PKG_VERSION="9"

###################### PERFORM ACTIONS NOW ##################################

crontab < cron

/usr/bin/killall monitor.sh || true
/usr/bin/killall instamsg || true

cp energy-meter_lanner-pc_6.4.0_6.4.2 ${HOME_DIRECTORY}/instamsg
cp upgrade_params ${HOME_DIRECTORY}
cp -f sg_upgrade.sh ${HOME_DIRECTORY}
