#!/bin/sh

. ../upgrade_params
PKG_VERSION="25"

###################### PERFORM ACTIONS NOW ##################################

crontab < cron

/usr/bin/killall monitor.sh || true
/usr/bin/killall instamsg || true

sed -i 's/#FSCKFIX=no/FSCKFIX=yes/g' /etc/default/rcS

cp upgrade_params ${HOME_DIRECTORY}
cp -f sg_upgrade.sh ${HOME_DIRECTORY}
cp ioeye_lanner-pc_17.3.0_17.3.0 ${HOME_DIRECTORY}/instamsg
