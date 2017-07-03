#!/bin/sh

. ../upgrade_params
PKG_VERSION="13"

###################### PERFORM ACTIONS NOW ##################################

crontab < cron

/usr/bin/killall monitor.sh || true
/usr/bin/killall instamsg || true

cp upgrade_params ${HOME_DIRECTORY}
cp -f sg_upgrade.sh ${HOME_DIRECTORY}
cp energy-meter_lanner-pc_9.2.0_9.2.0 ${HOME_DIRECTORY}/instamsg
