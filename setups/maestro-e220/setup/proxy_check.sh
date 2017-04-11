#!/bin/sh

PID=`ps | grep proxy_main | grep -v grep | grep -v tail | sed -e 's/^ *//g' | cut -d\  -f 1`

if [ -z "${PID}" ]
then
    /overlay/home/sensegrow/proxy_main.sh &
fi
