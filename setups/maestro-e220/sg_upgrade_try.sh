#!/bin/sh

PID=`ps | grep sg_upgrade.sh | grep -v grep | grep -v tail | sed -e 's/^ *//g' | cut -d\  -f 1`

if [ -z "${PID}" ]
then
    echo "Safe to proceed (stage 1) ..."

    PID=`ps -aux | grep -v grep | grep -v tail | grep sg_setup.sh`
    if [ -z "${PID}" ]
    then
	echo "Safe to proceed (stage 2) ..."

    	cd /overlay/home/sensegrow
    	chmod 777 ./sg_upgrade.sh
    	./sg_upgrade.sh &
    else
        echo "An instance of sg_setup.sh already running, bye bye"
    fi
else
    echo "An instance of sg_upgrade.sh already running, bye bye"
fi
