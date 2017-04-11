#!/bin/sh

while true
do

proxy_command=`cat /overlay/home/sensegrow/proxy_command`

if [ -z "${proxy_command}" ]
then
   echo "proxy_command is empty, nothing to do"
   /usr/bin/killall sshpass
else
   server=`echo "${proxy_command}" | cut -d\  -f 5`
   user=`echo "${proxy_command}" | cut -d\  -f 4`
   password=`echo "${proxy_command}" | cut -d\  -f 1`
   port=`echo "${proxy_command}" | cut -d\  -f 2`
   local_ip=`echo "${proxy_command}" | cut -d\  -f 3`

   remote_diag=`/usr/bin/sshpass -p ${password} /usr/bin/ssh ${user}@${server} "netstat -an | grep LISTEN | grep \ 127.0.0.1:${port}\ `
   remote_diag=`echo ${remote_diag} | grep -v unreachable | grep -v SO_PRIORITY`

   if [ -z "${remote_diag}" ]
   then
   	/usr/bin/killall sshpass
   fi

   PID=`ps | grep sshpass | grep -v grep | grep -v tail | sed -e 's/^ *//g' | cut -d\  -f 1`

   if [ -z "${PID}" ]
    then
        echo "sshpass not running"
        /usr/bin/sshpass -p ${password} /usr/bin/ssh -N -R 0.0.0.0:${port}:${local_ip} ${user}@${server} &
    else
        echo "sshpass running fine"
    fi
fi

sleep 15
done
