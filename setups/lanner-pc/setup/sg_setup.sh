#!/bin/sh

. ../upgrade_params
PKG_VERSION="4"

###################### PERFORM ACTIONS NOW ##################################

/usr/bin/killall monitor.sh || true
/usr/bin/killall instamsg || true

cp energy-meter_lanner-pc_3.0.9_3.0.9 ${HOME_DIRECTORY}/instamsg
cp upgrade_params ${HOME_DIRECTORY}
cp -f sg_upgrade.sh ${HOME_DIRECTORY}
