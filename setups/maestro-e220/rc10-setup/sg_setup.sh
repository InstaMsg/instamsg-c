#!/bin/sh

. ../upgrade_params
PKG_VERSION="2"

###################### PERFORM ACTIONS NOW ##################################

/usr/bin/killall instamsg || true
cp -f energy-meter_maestro-e220_6.5.0_6.5.0 ${HOME_DIRECTORY}/instamsg

