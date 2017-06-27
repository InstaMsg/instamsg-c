#! /bin/sh
count=0
var1=`uci get network.@watchdog[0].enable | cut -d = -f 2`
if [ "$var1" != "0" ]
	then
	var2=`uci get network.@watchdog[0].time | cut -d = -f 2`
	var2=`expr $var2 \* 60`
else
	exit
fi


while [ $var2 -ne 0 ]
do
	p=`get_interface_status.sh get_internet_status`
	echo $p
	if [ "$p" == "offline" ]
		then
		echo "No Internet"
		count=`expr $count + 1`
		echo $count
		if [ $count -eq $var2 ]
			then
			echo "counter exceeded"
			count=0
			echo reboot
			/usr/bin/logger -t NetworkWatchDog -p info "Restarting due to all interface offline for $var2 seconds"
			/overlay/home/sensegrow/reboot.sh
		fi
	else
		echo " Internet Resetting count"
		count=0
	fi
	sleep 1
done

