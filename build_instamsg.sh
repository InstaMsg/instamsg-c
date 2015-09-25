. ${1}

TOTAL_INCLUDES=`echo                                                \
        ${SYSTEM_INCLUDES}                                          \
        -Isrc/${VENDOR}/                                            \
        -Isrc/mqtt/                                                 `


SOURCES=`echo                                                                           \
        ${EXTRA_SOURCES}                                                                \
        src/driver/instamsg.c                                                           \
        src/driver/log.c                                                                \
        src/driver/json.c                                                               \
        src/driver/globals.c                                                            \
        src/driver/sg_mem.c                                                             \
        src/driver/sg_stdlib.c                                                          \
        src/driver/network.c                                                            \
        src/driver/serial_logger.c                                                      \
        src/driver/time.c                                                               \
                                                                                        \
        src/mqtt/src/MQTTFormat.c                                                       \
        src/mqtt/src/MQTTPacket.c                                                       \
        src/mqtt/src/MQTTDeserializePublish.c                                           \
        src/mqtt/src/MQTTConnectClient.c                                                \
        src/mqtt/src/MQTTSubscribeClient.c                                              \
        src/mqtt/src/MQTTSerializePublish.c                                             \
        src/mqtt/src/MQTTConnectServer.c                                                \
        src/mqtt/src/MQTTSubscribeServer.c                                              \
        src/mqtt/src/MQTTUnsubscribeServer.c                                            \
        src/mqtt/src/MQTTUnsubscribeClient.c                                            \
                                                                                        \
        src/${VENDOR}/device_network.c                                                  \
        src/${VENDOR}/device_serial_logger.c                                            \
        src/${VENDOR}/device_time.c                                                     \
        src/${VENDOR}/device_watchdog.c                                                 \
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
    SOURCES="${SOURCES} src/driver/file_system.c src/${VENDOR}/device_file_system.c src/driver/httpclient.c  "
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
