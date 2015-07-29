THREAD_IMPL="pthread"
TIME_IMPL="sys_timeval"
NETWORK_COMMUNICATION_IMPL="tcp_socket"
FS_COMMUNICATION_IMPL="linux_fs"


EXTRA_LINK_LIBS="-lpthread"

gcc stdoutsub.c -I ../MQTTPacket/src src/common/instamsg.c src/threading/${THREAD_IMPL}.c src/time/${TIME_IMPL}.c src/communication/network/${NETWORK_COMMUNICATION_IMPL}.c src/communication/fs/${FS_COMMUNICATION_IMPL}.c ../MQTTPacket/src/MQTTFormat.c  ../MQTTPacket/src/MQTTPacket.c ../MQTTPacket/src/MQTTDeserializePublish.c ../MQTTPacket/src/MQTTConnectClient.c ../MQTTPacket/src/MQTTSubscribeClient.c ../MQTTPacket/src/MQTTSerializePublish.c -o stdoutsub ../MQTTPacket/src/MQTTConnectServer.c ../MQTTPacket/src/MQTTSubscribeServer.c ../MQTTPacket/src/MQTTUnsubscribeServer.c ../MQTTPacket/src/MQTTUnsubscribeClient.c ${EXTRA_LINK_LIBS}
