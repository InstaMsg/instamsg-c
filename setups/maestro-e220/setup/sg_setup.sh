#!/bin/sh

. ../upgrade_params
PKG_VERSION="3"

###################### PERFORM ACTIONS NOW ##################################

/usr/bin/killall monitor.sh || true
/usr/bin/killall instamsg || true

cp energy-meter_maestro-e220_3.0.8_3.0.8 ${HOME_DIRECTORY}/instamsg
cp -f sg_upgrade.sh ${HOME_DIRECTORY}

${HOME_DIRECTORY}/reboot.sh
