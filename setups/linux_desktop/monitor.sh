PID=`pgrep -x instamsg`

if [ -z "${PID}" ]
then
	echo "Binary not running"

    rm -f /home/sensegrow/instamsg.log
    cd /home/sensegrow
    sleep 3
	./instamsg &
else
	echo "Binary running fine"
fi
