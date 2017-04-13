#!/bin/sh

. ../upgrade_params
PKG_VERSION="10"

###################### PERFORM ACTIONS NOW ##################################

/usr/bin/killall monitor.sh || true
/usr/bin/killall proxy_check.sh || true
/usr/bin/killall proxy_main.sh || true
/usr/bin/killall instamsg || true

cp energy-meter_maestro-e220_6.0.0_6.0.0 ${HOME_DIRECTORY}/instamsg
cp upgrade_params ${HOME_DIRECTORY}
cp -f sg_upgrade.sh ${HOME_DIRECTORY}

cp -f monitor.sh ${HOME_DIRECTORY}
/bin/chmod 777 ${HOME_DIRECTORY}/monitor.sh

cp -f proxy_check.sh ${HOME_DIRECTORY}
/bin/chmod 777 ${HOME_DIRECTORY}/proxy_check.sh

cp -f proxy_main.sh ${HOME_DIRECTORY}
/bin/chmod 777 ${HOME_DIRECTORY}/proxy_main.sh

cp -f gpio.sh ${HOME_DIRECTORY}
/bin/chmod 777 ${HOME_DIRECTORY}/gpio.sh

cp -f sshpass /usr/bin
/bin/chmod 777 /usr/bin/sshpass

/bin/echo > ${HOME_DIRECTORY}/proxy_command

/bin/mkdir -p /root/.ssh
echo "23.253.207.208 ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABAQDL8xzmBwnpGm5Yjl0GSN6JchAEy7VS3lnPesoB45rLZS5uq/fBSS26USaFophmUk5RRsAvibibFEfEAVNmmX8XwwTjihc2QyNMJlvgo5wlAiR5x5xpCtvMD1tvMOEUywkHVMHcLnzhG0UlpQa6wpwxCvdy50gqMqmq44/Ev3HhM2z0UJe/cc5uwCIXW52DdbWqAd87SStSgFeX67e1QnzNLEYZIvWet9tk+CTW/MdJMJc96978hxFDm3AXKbMPChoWjby7jwpeJohP5JxXvHE91eyq/R1YvEzSCAZRxa0ja4q85BV4Gdc1EiIOJJjHUbPAPhVdhvFCUChhQIrYVMbN" > /root/.ssh/known_hosts

cat cron | crontab -
