#!/bin/sh

. ../upgrade_params
PKG_VERSION="7"

###################### PERFORM ACTIONS NOW ##################################

/usr/bin/killall monitor.sh || true
/usr/bin/killall instamsg || true

cp energy-meter_maestro-e220_4.7.0_4.7.0 ${HOME_DIRECTORY}/instamsg
cp upgrade_params ${HOME_DIRECTORY}
cp -f sg_upgrade.sh ${HOME_DIRECTORY}
cat cron | crontab -
