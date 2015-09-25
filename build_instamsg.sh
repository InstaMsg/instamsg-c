. ${1}

TOTAL_INCLUDES=`echo                                                                            \
        ${SYSTEM_INCLUDES}                                                                      \
        -Iinstamsg/${VENDOR}/                                                                   \
        -Iinstamsg/mqtt/                                                                        `


SOURCES=`echo                                                                                   \
        ${EXTRA_SOURCES}                                                                        \
        instamsg/driver/instamsg.c                                                              \
        instamsg/driver/log.c                                                                   \
        instamsg/driver/json.c                                                                  \
        instamsg/driver/globals.c                                                               \
        instamsg/driver/sg_mem.c                                                                \
        instamsg/driver/sg_stdlib.c                                                             \
        instamsg/driver/network.c                                                               \
        instamsg/driver/serial_logger.c                                                         \
        instamsg/driver/time.c                                                                  \
                                                                                                \
        instamsg/mqtt/src/MQTTFormat.c                                                          \
        instamsg/mqtt/src/MQTTPacket.c                                                          \
        instamsg/mqtt/src/MQTTDeserializePublish.c                                              \
        instamsg/mqtt/src/MQTTConnectClient.c                                                   \
        instamsg/mqtt/src/MQTTSubscribeClient.c                                                 \
        instamsg/mqtt/src/MQTTSerializePublish.c                                                \
        instamsg/mqtt/src/MQTTConnectServer.c                                                   \
        instamsg/mqtt/src/MQTTSubscribeServer.c                                                 \
        instamsg/mqtt/src/MQTTUnsubscribeServer.c                                               \
        instamsg/mqtt/src/MQTTUnsubscribeClient.c                                               \
                                                                                                \
        instamsg/${VENDOR}/device_network.c                                                     \
        instamsg/${VENDOR}/device_serial_logger.c                                               \
        instamsg/${VENDOR}/device_time.c                                                        \
        instamsg/${VENDOR}/device_watchdog.c                                                    \
        instamsg/${VENDOR}/device_misc.c                                                        `


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
    SOURCES="${SOURCES} instamsg/driver/file_system.c instamsg/${VENDOR}/device_file_system.c instamsg/driver/httpclient.c  "
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
