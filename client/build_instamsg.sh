. ${1}

TOTAL_INCLUDES=`echo                                                \
        ${SYSTEM_INCLUDES}                                          \
        -Isrc/${VENDOR}/                                            \
        -I ../MQTTPacket/src    `


SOURCES=`echo                                                                           \
        ${EXTRA_SOURCES}                                                                \
        src/common/instamsg.c                                                           \
        src/common/log.c                                                                \
        src/common/json.c                                                               \
        src/common/globals.c                                                            \
        src/common/sg_mem.c                                                             \
        src/common/sg_stdlib.c                                                          \
        src/common/network.c                                                            \
        src/common/serial_logger.c                                                      \
        src/common/time.c                                                               \
                                                                                        \
        ../MQTTPacket/src/MQTTFormat.c                                                  \
        ../MQTTPacket/src/MQTTPacket.c                                                  \
        ../MQTTPacket/src/MQTTDeserializePublish.c                                      \
        ../MQTTPacket/src/MQTTConnectClient.c                                           \
        ../MQTTPacket/src/MQTTSubscribeClient.c                                         \
        ../MQTTPacket/src/MQTTSerializePublish.c                                        \
        ../MQTTPacket/src/MQTTConnectServer.c                                           \
        ../MQTTPacket/src/MQTTSubscribeServer.c                                         \
        ../MQTTPacket/src/MQTTUnsubscribeServer.c                                       \
        ../MQTTPacket/src/MQTTUnsubscribeClient.c                                       \
                                                                                        \
        src/${VENDOR}/device_network.c                                                  \
        src/${VENDOR}/device_serial_logger.c                                            \
        src/${VENDOR}/device_time.c                                                     \
        src/${VENDOR}/device_misc.c                                                     `


mkdir -p build/${VENDOR}
rm -f build/${VENDOR}/*


if [ ! -z ${IS_GSM_DEVICE} ];
then
    COMPILE_COMMAND="${COMPILE_COMMAND} -DGSM_INTERFACE_ENABLED"
fi


if [ ! -z ${IS_SSL} ];
then
    COMPILE_COMMAND="${COMPILE_COMMAND} -DSSL_ENABLED"
fi



# Compile the File-System-Module only if applicable.
if [ ! -z ${FS_COMMUNICATION_IMPL} ];
then
    SOURCES="${SOURCES} src/common/file_system.c src/${VENDOR}/device_file_system.c src/common/httpclient.c  "
    COMPILE_COMMAND="${COMPILE_COMMAND} -DFILE_SYSTEM_INTERFACE_ENABLED"
fi


TOTAL_COMPILE_COMMAND="${COMPILE_COMMAND} ${TOTAL_INCLUDES} ${SOURCES} -o build/${VENDOR}/instamsg"

for obj in ${EXTRA_OBJECT_FILES}
do
    TOTAL_COMPILE_COMMAND="${COMPILE_COMMAND} ${TOTAL_INCLUDES} ${SOURCES} build/${VENDOR}/* -o build/${VENDOR}/instamsg"
    cp ${obj} build/${VENDOR}
done

${TOTAL_COMPILE_COMMAND}

for cmd in "${FINAL_COMMANDS[@]}"
do
    $cmd
done
