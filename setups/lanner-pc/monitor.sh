while true
do
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
