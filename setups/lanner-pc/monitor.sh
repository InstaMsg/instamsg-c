#!/bin/sh

/bin/mknod /dev/wd_drv c 241 0
chmod 777 /home/sensegrow/wd_tst

while true
do
    cd /home/sensegrow
    ./wd_tst --swt 120
    ./wd_tst --s-start

    PID=`ps -aux | grep -v grep | grep -v tail | grep instamsg`

    if [ -z "${PID}" ]
    then
	    echo "Binary not running"
        cd /home/sensegrow
        chmod 777 instamsg

        sleep 3
	    ./instamsg &
    else
	    echo "Binary running fine"
    fi

    PID=`ps -aux | grep -v grep | grep -v tail | grep wwan-monitor`

    if [ -z "${PID}" ]
    then
	    echo "WWAN-Monitor Binary not running"
        cd /home/sensegrow
        chmod 777 wwan-monitor
        sleep 3
	    ./wwan-monitor > wwan-monitor.log &
    else
	    echo "WWAN-Monitor Binary running fine"
    fi

    sleep 60
done
