LOGIN="root@192.168.1.1"
SSH_COMMAND="ssh ${LOGIN}"

HOME_DIRECTORY="/overlay/home/sensegrow"

scp maestro-e220-updatedkernel  ${LOGIN}:${HOME_DIRECTORY}
${SSH_COMMAND} "/sbin/mtd write ${HOME_DIRECTORY}/maestro-e220-updatedkernel /dev/mtd3"
${SSH_COMMAND} "/sbin/mtd erase /dev/mtd5"

${SSH_COMMAND} "sed -i 's/^[ :\t]*\/etc\/init.d\/event_sms reload/#\/etc\/init.d\/event_sms reload/g' /usr/sbin/eventtrack.sh"
${SSH_COMMAND} "sed -i 's/^[ :\t]*\/etc\/init.d\/event_sms stop/#\/etc\/init.d\/event_sms stop/g' /usr/sbin/eventtrack.sh"
${SSH_COMMAND} "sed -i 's/^[ :\t]*\/etc\/init.d\/event_sms start/#\/etc\/init.d\/event_sms start/g' /usr/sbin/eventtrack.sh"

${SSH_COMMAND} "sed -i 's/^[ :\t]*\/usr\/sbin\/event_sms.sh/#\/usr\/sbin\/event_sms.sh/g' /etc/init.d/event_sms"

${SSH_COMMAND} "sed -i '/^[ :\t]*\/home\/sensegrow\/monitor.sh \&/d' /etc/rc.local"
${SSH_COMMAND} "sed -i 's/^[ :\t]*exit 0/\/home\/sensegrow\/monitor.sh \&\nexit 0/g' /etc/rc.local"

${SSH_COMMAND} "sed -i 's/HL8548/HL85/g' /lib/netifd/proto/3g.sh"


${SSH_COMMAND} "mkdir -p ${HOME_DIRECTORY}"
${SSH_COMMAND} "chmod -R 777 ${HOME_DIRECTORY}"

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

${SSH_COMMAND} "killall instamsg"
scp $1  ${LOGIN}:${HOME_DIRECTORY}/instamsg
