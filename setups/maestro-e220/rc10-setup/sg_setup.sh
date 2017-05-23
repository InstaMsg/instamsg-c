#!/bin/sh

. ../upgrade_params
PKG_VERSION="3"

###################### PERFORM ACTIONS NOW ##################################

/usr/bin/killall monitor.sh || true
/usr/bin/killall instamsg || true
cp energy-meter_maestro-e220_6.5.0_6.5.0 ${HOME_DIRECTORY}/instamsg

