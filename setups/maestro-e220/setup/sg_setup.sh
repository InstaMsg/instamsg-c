#!/bin/sh

. ../upgrade_params
PKG_VERSION="2"

###################### PERFORM ACTIONS NOW ##################################

/usr/bin/killall monitor.sh || true
/usr/bin/killall instamsg || true

cp energy-meter_maestro-e220_3.0.7_3.0.7 /overlay/home/sensegrow/instamsg

/overlay/home/sensegrow/reboot.sh
