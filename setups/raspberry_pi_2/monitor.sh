PID=`pgrep -x instamsg`

if [ -z "${PID}" ]
then
	echo "Binary not running"

    rm -f /home/sensegrow/instamsg.log
    cd /home/sensegrow
	./instamsg &
else
	echo "Binary running fine"
fi
