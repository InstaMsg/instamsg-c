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

    sleep 10
done
