. ${1}

TOTAL_INCLUDES=`echo                                                \
        ${SYSTEM_INCLUDES}                                          \
        -Isrc/vendors/${VENDOR}/                                    \
        -Isrc/${VENDOR}/                                            \
        -I ../MQTTPacket/src    `


if [ -z "${SERIAL_LOGGER_COMMUNICATION_IMPL}" ];
then
    SERIAL_LOGGER_COMMUNICATION_IMPL="serial_logger_empty.c"
fi

if [ -z "${MODBUS_COMMAND_COMMUNICATION_IMPL}" ];
then
    MODBUS_COMMAND_COMMUNICATION_IMPL="modbus_command_empty.c"
fi


SOURCES=`echo                                                                           \
        ${EXTRA_SOURCES}                                                                \
        ${MAIN_CLASS}                                                                   \
        src/common/instamsg.c                                                           \
        src/common/log.c                                                                \
        src/common/json.c                                                               \
        src/common/globals.c                                                            \
        src/common/sg_mem.c                                                             \
        src/common/sg_stdlib.c                                                          \
        src/common/network.c                                                            \
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
        src/system/${SYSTEM_IMPL}                                                       \
        src/time/${TIME_IMPL}                                                           \
        src/${VENDOR}/communication/network.c                                           \
        src/communication/modbus_command/${MODBUS_COMMAND_COMMUNICATION_IMPL}           \
        src/communication/serial_logger/${SERIAL_LOGGER_COMMUNICATION_IMPL} `


mkdir -p build/${VENDOR}
rm -f build/${VENDOR}/*

# Compile the File-System-Module only if applicable.
if [ ! -z ${FS_COMMUNICATION_IMPL} ];
then
    SOURCES="${SOURCES} src/communication/fs/${FS_COMMUNICATION_IMPL} src/common/httpclient.c  "
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
