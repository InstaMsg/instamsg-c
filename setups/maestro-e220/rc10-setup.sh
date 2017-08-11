LOGIN="root@192.168.1.1"
SSH_COMMAND="ssh ${LOGIN}"

HOME_DIRECTORY="/overlay/home/sensegrow"

${SSH_COMMAND} "mkdir -p ${HOME_DIRECTORY}"
${SSH_COMMAND} "chmod -R 777 ${HOME_DIRECTORY}"

${SSH_COMMAND} "sed -i '/eventsms_config/d' /etc/init.d/event_sms"
${SSH_COMMAND} "sed -i 's/^[ :\t]*\/usr\/sbin\/event_sms.sh/#\/usr\/sbin\/event_sms.sh/g' /etc/init.d/event_sms"
${SSH_COMMAND} "sed -i 's/^[ :\t]*\/usr\/bin\/eventsms \&/#\/usr\/bin\/eventsms \&\n\/usr\/bin\/sendat \/dev\/ttyACM4 \"at\*psstki=3\" 10/g' /etc/init.d/event_sms"

scp rc10-rc.local  ${LOGIN}:/etc/rc.local
${SSH_COMMAND} "chmod 777 /etc/rc.local"

${SSH_COMMAND} "rm -f /etc/init.d/gps"


scp rc10-monitor.sh  ${LOGIN}:${HOME_DIRECTORY}/monitor.sh
${SSH_COMMAND} "chmod 777 ${HOME_DIRECTORY}/monitor.sh"

scp rc10-wandete.sh  ${LOGIN}:/sbin/wandete.sh
${SSH_COMMAND} "chmod 777 /sbin/wandete.sh"

scp rc10-cron  ${LOGIN}:${HOME_DIRECTORY}/cron
${SSH_COMMAND} "chmod 777 ${HOME_DIRECTORY}/cron"

scp rc10-systemreboot.sh  ${LOGIN}:/usr/sbin/systemreboot.sh
${SSH_COMMAND} "chmod 777 /usr/sbin/systemreboot.sh"

${SSH_COMMAND} "cat ${HOME_DIRECTORY}/cron | crontab -"
${SSH_COMMAND} rm ${HOME_DIRECTORY}/cron

${SSH_COMMAND} touch ${HOME_DIRECTORY}/data.txt
${SSH_COMMAND} rm ${HOME_DIRECTORY}/data.txt
${SSH_COMMAND} touch ${HOME_DIRECTORY}/data.txt
${SSH_COMMAND} chmod 777 ${HOME_DIRECTORY}/data.txt

${SSH_COMMAND} "echo test > ${HOME_DIRECTORY}/prov.txt"

${SSH_COMMAND} "mkdir -p /root/.ssh"
${SSH_COMMAND} "echo \"23.253.207.208 ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAABAQDL8xzmBwnpGm5Yjl0GSN6JchAEy7VS3lnPesoB45rLZS5uq/fBSS26USaFophmUk5RRsAvibibFEfEAVNmmX8XwwTjihc2QyNMJlvgo5wlAiR5x5xpCtvMD1tvMOEUywkHVMHcLnzhG0UlpQa6wpwxCvdy50gqMqmq44/Ev3HhM2z0UJe/cc5uwCIXW52DdbWqAd87SStSgFeX67e1QnzNLEYZIvWet9tk+CTW/MdJMJc96978hxFDm3AXKbMPChoWjby7jwpeJohP5JxXvHE91eyq/R1YvEzSCAZRxa0ja4q85BV4Gdc1EiIOJJjHUbPAPhVdhvFCUChhQIrYVMbN\" > /root/.ssh/known_hosts"

scp reboot.sh  ${LOGIN}:${HOME_DIRECTORY}
${SSH_COMMAND} "chmod 777 ${HOME_DIRECTORY}/reboot.sh"

scp ../../sg_upgrade_try.sh ${LOGIN}:${HOME_DIRECTORY}
${SSH_COMMAND} "chmod 777 ${HOME_DIRECTORY}/sg_upgrade_try.sh"

scp ../../sg_upgrade.sh ${LOGIN}:${HOME_DIRECTORY}
${SSH_COMMAND} "chmod 777 ${HOME_DIRECTORY}/sg_upgrade.sh"

${SSH_COMMAND} "echo > ${HOME_DIRECTORY}/proxy_command"

scp rc10-upgrade_params ${LOGIN}:${HOME_DIRECTORY}/upgrade_params
${SSH_COMMAND} "chmod 777 ${HOME_DIRECTORY}/upgrade_params"

scp gpio.sh ${LOGIN}:${HOME_DIRECTORY}
${SSH_COMMAND} "chmod 777 ${HOME_DIRECTORY}/gpio.sh"

${SSH_COMMAND} "killall proxy_check.sh"
${SSH_COMMAND} "killall proxy_main.sh"
scp proxy_check.sh ${LOGIN}:${HOME_DIRECTORY}
scp proxy_main.sh ${LOGIN}:${HOME_DIRECTORY}
${SSH_COMMAND} "chmod 777 ${HOME_DIRECTORY}/proxy_check.sh"
${SSH_COMMAND} "chmod 777 ${HOME_DIRECTORY}/proxy_main.sh"

${SSH_COMMAND} "killall sshpass"
scp sshpass ${LOGIN}:/usr/bin
${SSH_COMMAND} "chmod 777 /usr/bin/sshpass"

${SSH_COMMAND} "echo 14 > ${HOME_DIRECTORY}/current_version"

${SSH_COMMAND} "killall monitor.sh"
${SSH_COMMAND} "killall instamsg"
scp $1  ${LOGIN}:${HOME_DIRECTORY}/instamsg
