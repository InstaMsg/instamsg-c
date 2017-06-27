#!/bin/sh

/usr/bin/killall monitor.sh
sleep 3

/usr/bin/killall sendat
sleep 3

/usr/bin/killall instamsg
sleep 3

/bin/echo `/bin/date +%s` > /overlay/home/sensegrow/last_reboot_timestamp
sleep 1

/sbin/reboot
