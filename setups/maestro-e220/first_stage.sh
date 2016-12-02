LOGIN="root@192.168.1.1"
SSH_COMMAND="ssh ${LOGIN}"

scp maestro-e220-updatedkernel /tmp
${SSH_COMMAND} "/sbin/mtd write /tmp/maestro-e220-updatedkernel /dev/mtd3"
${SSH_COMMAND} "/sbin/mtd erase /dev/mtd5"
