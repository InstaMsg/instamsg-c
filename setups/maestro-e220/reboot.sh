#!/bin/sh

/usr/bin/killall monitor.sh
sleep 3

/usr/bin/killall instamsg
sleep 3

/sbin/reboot
