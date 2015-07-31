SYSTEM_INCLUDES=`echo                                               \
        -I/usr/lib/gcc/i686-linux-gnu/4.8/include                   \
        -I/usr/local/include                                        \
        -I/usr/lib/gcc/i686-linux-gnu/4.8/include-fixed             \
        -I/usr/include/i386-linux-gnu                               \
        -I/usr/include `

THREAD_IMPL="pthread"
SYSTEM_IMPL="linux_desktop"
TIME_IMPL="sys_timeval"
NETWORK_COMMUNICATION_IMPL="tcp_socket"
FS_COMMUNICATION_IMPL="linux_fs"
COMMAND_COMMUNICATION_IMPL="linux_command_stub"

EXTRA_LINK_LIBS="-lpthread"

gcc -nostdinc                                                       \
        stdoutsub.c                                                 \
                                                                    \
        ${SYSTEM_INCLUDES}                                          \
        -I ../MQTTPacket/src                                        \
                                                                    \
        src/common/instamsg.c                                       \
        src/common/log.c                                            \
        src/common/config.c                                         \
                                                                    \
        ../MQTTPacket/src/MQTTFormat.c                              \
        ../MQTTPacket/src/MQTTPacket.c                              \
        ../MQTTPacket/src/MQTTDeserializePublish.c                  \
        ../MQTTPacket/src/MQTTConnectClient.c                       \
        ../MQTTPacket/src/MQTTSubscribeClient.c                     \
        ../MQTTPacket/src/MQTTSerializePublish.c                    \
        ../MQTTPacket/src/MQTTConnectServer.c                       \
        ../MQTTPacket/src/MQTTSubscribeServer.c                     \
        ../MQTTPacket/src/MQTTUnsubscribeServer.c                   \
        ../MQTTPacket/src/MQTTUnsubscribeClient.c                   \
                                                                    \
        src/threading/${THREAD_IMPL}.c                              \
        src/system/${SYSTEM_IMPL}.c                                 \
        src/time/${TIME_IMPL}.c                                     \
        src/communication/network/${NETWORK_COMMUNICATION_IMPL}.c   \
        src/communication/fs/${FS_COMMUNICATION_IMPL}.c             \
        src/communication/command/${COMMAND_COMMUNICATION_IMPL}.c   \
                                                                    \
        -o stdoutsub                                                \
                                                                    \
        ${EXTRA_LINK_LIBS}
