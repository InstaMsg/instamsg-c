PID=`pgrep -x pi`

if [ -z "${PID}" ]
then
	echo "Binary not running"

	RESTART_CASE=`cat /home/sensegrow/restart | head -1`
	if [ "${RESTART_CASE}" = "1" ]
	then
		rm -f /home/sensegrow/instamsg.log
		rm -f /home/sensegrow/restart
	fi

	/home/sensegrow/pi &
else
	echo "Binary running fine"
fi
