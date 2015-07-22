cp ../../src/MQTTClient.c .
sed -e 's/""/"MQTTLinux.h"/g' ../../src/MQTTClient.h > MQTTClient.h
sh compile.sh
