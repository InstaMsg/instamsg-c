LOGIN="root@192.168.1.1"
SSH_COMMAND="ssh ${LOGIN}"

HOME_DIRECTORY="/overlay/home/sensegrow"

${SSH_COMMAND} "mkdir -p ${HOME_DIRECTORY}"
${SSH_COMMAND} "chmod -R 777 ${HOME_DIRECTORY}"

${SSH_COMMAND} "sed -i 's/^[ :\t]*\/etc\/init.d\/event_sms reload/#\/etc\/init.d\/event_sms reload/g' /usr/sbin/eventtrack.sh"
${SSH_COMMAND} "sed -i 's/^[ :\t]*\/etc\/init.d\/event_sms stop/#\/etc\/init.d\/event_sms stop/g' /usr/sbin/eventtrack.sh"
${SSH_COMMAND} "sed -i 's/^[ :\t]*\/etc\/init.d\/event_sms start/#\/etc\/init.d\/event_sms start/g' /usr/sbin/eventtrack.sh"

${SSH_COMMAND} "sed -i 's/^[ :\t]*\/usr\/sbin\/event_sms.sh/#\/usr\/sbin\/event_sms.sh/g' /etc/init.d/event_sms"

${SSH_COMMAND} "sed -i '/^[ :\t]*\/home\/sensegrow\/monitor.sh \&/d' /etc/rc.local"
${SSH_COMMAND} "sed -i 's/^[ :\t]*exit 0/\/home\/sensegrow\/monitor.sh \&\nexit 0/g' /etc/rc.local"

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

scp reboot.sh  ${LOGIN}:${HOME_DIRECTORY}
${SSH_COMMAND} "chmod 777 ${HOME_DIRECTORY}/reboot.sh"

scp ../../sg_upgrade_try.sh ${LOGIN}:${HOME_DIRECTORY}
${SSH_COMMAND} "chmod 777 ${HOME_DIRECTORY}/sg_upgrade_try.sh"

scp ../../sg_upgrade.sh ${LOGIN}:${HOME_DIRECTORY}
${SSH_COMMAND} "chmod 777 ${HOME_DIRECTORY}/sg_upgrade.sh"

${SSH_COMMAND} "echo > ${HOME_DIRECTORY}/proxy_command"

scp upgrade_params ${LOGIN}:${HOME_DIRECTORY}
${SSH_COMMAND} "chmod 777 ${HOME_DIRECTORY}/upgrade_params"

${SSH_COMMAND} "killall proxy_check.sh"
${SSH_COMMAND} "killall proxy_main.sh"
scp proxy_check.sh ${LOGIN}:${HOME_DIRECTORY}
scp proxy_main.sh ${LOGIN}:${HOME_DIRECTORY}
${SSH_COMMAND} "chmod 777 ${HOME_DIRECTORY}/proxy_check.sh"
${SSH_COMMAND} "chmod 777 ${HOME_DIRECTORY}/proxy_main.sh"

${SSH_COMMAND} "killall sshpass"
scp sshpass ${LOGIN}:/usr/bin
${SSH_COMMAND} "chmod 777 /usr/bin/sshpass"

${SSH_COMMAND} "echo 8 > ${HOME_DIRECTORY}/current_version"

${SSH_COMMAND} "killall monitor.sh"
${SSH_COMMAND} "killall instamsg"
scp $1  ${LOGIN}:${HOME_DIRECTORY}/instamsg
