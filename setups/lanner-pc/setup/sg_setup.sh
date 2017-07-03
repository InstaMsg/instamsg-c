#!/bin/sh

. ../upgrade_params
PKG_VERSION="12"

###################### PERFORM ACTIONS NOW ##################################

crontab < cron

/usr/bin/killall monitor.sh || true
/usr/bin/killall instamsg || true

cp upgrade_params ${HOME_DIRECTORY}
cp -f sg_upgrade.sh ${HOME_DIRECTORY}
cp energy-meter_lanner-pc_9.1.0_9.1.0 ${HOME_DIRECTORY}/instamsg
