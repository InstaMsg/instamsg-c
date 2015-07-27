TIME_IMPL="sys_timeval"
NETWORK_IMPL="tcp_socket"
THREAD_IMPL="pthread"

EXTRA_LINK_LIBS="-lpthread"

gcc stdoutsub.c -I ../MQTTPacket/src src/common/instamsg.c src/time/${TIME_IMPL}.c src/network/${NETWORK_IMPL}.c src/threading/${THREAD_IMPL}.c ../MQTTPacket/src/MQTTFormat.c  ../MQTTPacket/src/MQTTPacket.c ../MQTTPacket/src/MQTTDeserializePublish.c ../MQTTPacket/src/MQTTConnectClient.c ../MQTTPacket/src/MQTTSubscribeClient.c ../MQTTPacket/src/MQTTSerializePublish.c -o stdoutsub ../MQTTPacket/src/MQTTConnectServer.c ../MQTTPacket/src/MQTTSubscribeServer.c ../MQTTPacket/src/MQTTUnsubscribeServer.c ../MQTTPacket/src/MQTTUnsubscribeClient.c ${EXTRA_LINK_LIBS}
