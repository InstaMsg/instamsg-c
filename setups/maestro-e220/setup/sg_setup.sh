#!/bin/sh

. ../upgrade_params
PKG_VERSION="24"

###################### PERFORM ACTIONS NOW ##################################

/usr/bin/killall monitor.sh || true
/usr/bin/killall proxy_check.sh || true
/usr/bin/killall proxy_main.sh || true
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

cp -f reboot.sh ${HOME_DIRECTORY}
/bin/chmod 777 ${HOME_DIRECTORY}/reboot.sh

cp -f rc.local /etc
/bin/chmod 777 /etc/rc.local

cp -f sshpass /usr/bin
/bin/chmod 777 /usr/bin/sshpass

sed -i '/^[ :\t]*\/home\/sensegrow\/gpio.sh \&/d' /etc/rc.local
sed -i 's/^[ :\t]*exit 0/\/home\/sensegrow\/gpio.sh \&\nexit 0/g' /etc/rc.local

/bin/echo > ${HOME_DIRECTORY}/proxy_command

/bin/mkdir -p /root/.ssh
echo "23.253.207.208 ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABAQDL8xzmBwnpGm5Yjl0GSN6JchAEy7VS3lnPesoB45rLZS5uq/fBSS26USaFophmUk5RRsAvibibFEfEAVNmmX8XwwTjihc2QyNMJlvgo5wlAiR5x5xpCtvMD1tvMOEUywkHVMHcLnzhG0UlpQa6wpwxCvdy50gqMqmq44/Ev3HhM2z0UJe/cc5uwCIXW52DdbWqAd87SStSgFeX67e1QnzNLEYZIvWet9tk+CTW/MdJMJc96978hxFDm3AXKbMPChoWjby7jwpeJohP5JxXvHE91eyq/R1YvEzSCAZRxa0ja4q85BV4Gdc1EiIOJJjHUbPAPhVdhvFCUChhQIrYVMbN" > /root/.ssh/known_hosts

/usr/bin/killall instamsg || true
cp energy-meter_maestro-e220_11.9.0_11.9.0 ${HOME_DIRECTORY}/instamsg

cat cron | crontab -
