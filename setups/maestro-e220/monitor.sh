PID=`ps | grep instamsg | grep -v grep | grep -v tail | sed -e 's/^ *//g' | cut -d\  -f 1`

if [ -z "${PID}" ]
then
	echo "Binary not running"
    cd /overlay/home/sensegrow
    chmod 777 instamsg

    sleep 3
	./instamsg &
else
	echo "Binary running fine"
fi
