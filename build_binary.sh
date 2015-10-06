APP=${1}
. apps/${APP}/Makefile

VENDOR=$2
. device/${VENDOR}/instamsg/Makefile


TOTAL_INCLUDES=`echo                                                                            \
        ${SYSTEM_INCLUDES}                                                                      \
        -Idevice/${VENDOR}/instamsg/                                                            \
        -Idevice/${VENDOR}/apps/ioeye/                                                          \
        -Iinstamsg/mqtt/                                                                        `


SOURCES=`echo                                                                                   \
        ${APP_SOURCES}                                                                          \
        ${EXTRA_SOURCES}                                                                        \
        instamsg/driver/instamsg.c                                                              \
        instamsg/driver/log.c                                                                   \
        instamsg/driver/json.c                                                                  \
        instamsg/driver/globals.c                                                               \
        instamsg/driver/sg_mem.c                                                                \
        instamsg/driver/sg_stdlib.c                                                             \
        instamsg/driver/socket.c                                                                \
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
        device/${VENDOR}/instamsg/device_socket.c                                               \
        device/${VENDOR}/instamsg/device_serial_logger.c                                        \
        device/${VENDOR}/instamsg/device_time.c                                                 \
        device/${VENDOR}/instamsg/device_watchdog.c                                             \
        device/${VENDOR}/instamsg/device_misc.c                                                 `


SOURCES=`echo ${SOURCES} | sed -e 's|VENDOR|'"${VENDOR}"'|g'`

OUT_DIR="build/${APP}/${VENDOR}"
mkdir -p ${OUT_DIR}
rm -f ${OUT_DIR}/*


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
    SOURCES="${SOURCES} instamsg/driver/file_system.c device/${VENDOR}/instamsg/device_file_system.c instamsg/driver/httpclient.c  "
    COMPILE_COMMAND="${COMPILE_COMMAND} -DFILE_SYSTEM_INTERFACE_ENABLED"
fi



TOTAL_COMPILE_COMMAND="${COMPILE_COMMAND} -DDEBUG_MODE ${TOTAL_INCLUDES} ${SOURCES} -o ${OUT_DIR}/instamsg"

for obj in ${EXTRA_OBJECT_FILES}
do
    TOTAL_COMPILE_COMMAND="${COMPILE_COMMAND} -DDEBUG_MODE ${TOTAL_INCLUDES} ${SOURCES} ${OUT_DIR}/* -o ${OUT_DIR}/instamsg"
    cp ${obj} ${OUT_DIR}
done

${TOTAL_COMPILE_COMMAND}

for cmd in "${FINAL_COMMANDS[@]}"
do
    cmd=`echo ${cmd} | sed -e 's|OUT_DIR|'"${OUT_DIR}"'|g'`
    $cmd
done
