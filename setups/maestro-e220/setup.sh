LOGIN="root@192.168.1.1"
SSH_COMMAND="ssh ${LOGIN}"

HOME_DIRECTORY="/overlay/home/sensegrow"

${SSH_COMMAND} "mkdir -p ${HOME_DIRECTORY}"
${SSH_COMMAND} "chmod -R 777 ${HOME_DIRECTORY}"

${SSH_COMMAND} "sed -i 's/^[ :\t]*\/etc\/init.d\/event_sms reload/#\/etc\/init.d\/event_sms reload/g' /usr/sbin/eventtrack.sh"
${SSH_COMMAND} "sed -i 's/^[ :\t]*\/etc\/init.d\/event_sms stop/#\/etc\/init.d\/event_sms stop/g' /usr/sbin/eventtrack.sh"
${SSH_COMMAND} "sed -i 's/^[ :\t]*\/etc\/init.d\/event_sms start/#\/etc\/init.d\/event_sms start/g' /usr/sbin/eventtrack.sh"

${SSH_COMMAND} "sed -i 's/^[ :\t]*\/usr\/sbin\/event_sms.sh/#\/usr\/sbin\/event_sms.sh/g' /etc/init.d/event_sms"

scp rc.local  ${LOGIN}:/etc
${SSH_COMMAND} "chmod 777 /etc/rc.local"

${SSH_COMMAND} "sed -i 's/HL8548/HL85/g' /lib/netifd/proto/3g.sh"
${SSH_COMMAND} "sed -i 's/IPV4V6/IP/g' /etc/chatscripts/3g.chat"
${SSH_COMMAND} "rm -f /etc/init.d/gps"



scp monitor.sh  ${LOGIN}:${HOME_DIRECTORY}
${SSH_COMMAND} "chmod 777 ${HOME_DIRECTORY}/monitor.sh"

scp cron  ${LOGIN}:${HOME_DIRECTORY}
${SSH_COMMAND} "chmod 777 ${HOME_DIRECTORY}/cron"

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

scp upgrade_params ${LOGIN}:${HOME_DIRECTORY}
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

${SSH_COMMAND} "echo 24 > ${HOME_DIRECTORY}/current_version"

${SSH_COMMAND} "killall monitor.sh"
${SSH_COMMAND} "killall instamsg"
scp $1  ${LOGIN}:${HOME_DIRECTORY}/instamsg
