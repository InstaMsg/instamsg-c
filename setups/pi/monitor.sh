PID=`ps -aux | grep instamsg | grep -v grep | grep -v tail`

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
