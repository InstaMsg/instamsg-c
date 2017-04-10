#!/bin/sh

PID=`ps | grep sshpass | grep -v grep | grep -v tail | sed -e 's/^ *//g' | cut -d\  -f 1`

if [ -z "${PID}" ]
then
    echo "sshpass not running"

    proxy_command=`cat proxy_command`
    ${proxy_command} &
else
    echo "sshpass running fine"
fi
