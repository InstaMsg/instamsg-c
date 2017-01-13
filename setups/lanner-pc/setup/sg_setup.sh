#!/bin/sh

. ../upgrade_params
PKG_VERSION="2"

###################### PERFORM ACTIONS NOW ##################################

/usr/bin/killall monitor.sh || true
/usr/bin/killall instamsg || true

cp energy-meter_lanner-pc_3.0.7_3.0.7 /home/sensegrow/instamsg

/sbin/reboot
