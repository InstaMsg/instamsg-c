. ${1}

TOTAL_INCLUDES=`echo                                                \
        ${SYSTEM_INCLUDES}                                          \
        -Isrc/vendors/${VENDOR}/                                    \
        -I ../MQTTPacket/src    `


if [ -z "${LOGGER_SERIAL_COMMUNICATION_IMPL}" ];
then
    LOGGER_SERIAL_COMMUNICATION_IMPL="logger_serial_empty"
fi

if [ -z "${COMMAND_SERIAL_COMMUNICATION_IMPL}" ];
then
    COMMAND_SERIAL_COMMUNICATION_IMPL="command_serial_empty"
fi


SOURCES=`echo                                                               \
        stdoutsub                                                           \
        src/common/instamsg                                                 \
        src/common/log                                                      \
        src/common/httpclient                                               \
        src/common/json                                                     \
                                                                            \
        ../MQTTPacket/src/MQTTFormat                                        \
        ../MQTTPacket/src/MQTTPacket                                        \
        ../MQTTPacket/src/MQTTDeserializePublish                            \
        ../MQTTPacket/src/MQTTConnectClient                                 \
        ../MQTTPacket/src/MQTTSubscribeClient                               \
        ../MQTTPacket/src/MQTTSerializePublish                              \
        ../MQTTPacket/src/MQTTConnectServer                                 \
        ../MQTTPacket/src/MQTTSubscribeServer                               \
        ../MQTTPacket/src/MQTTUnsubscribeServer                             \
        ../MQTTPacket/src/MQTTUnsubscribeClient                             \
                                                                            \
        src/system/${SYSTEM_IMPL}                                           \
        src/time/${TIME_IMPL}                                               \
        src/communication/network/${NETWORK_COMMUNICATION_IMPL}             \
        src/communication/command/${COMMAND_SERIAL_COMMUNICATION_IMPL}      \
        src/communication/serial/${LOGGER_SERIAL_COMMUNICATION_IMPL} `


mkdir -p build/${VENDOR}
rm -f build/${VENDOR}/*.o
rm -f build/${VENDOR}/instamsg

# Compile the File-System-Module only if applicable.
if [ ! -z ${FS_COMMUNICATION_IMPL} ];
then
    COMPILE_COMMAND="${COMPILE_COMMAND} -DFILE_SYSTEM_INTERFACE_ENABLED"
    ${COMPILE_COMMAND} ${TOTAL_INCLUDES} -c "src/communication/fs/${FS_COMMUNICATION_IMPL}.c" -o "build/${VENDOR}/${FS_COMMUNICATION_IMPL}.o"
fi

for source in ${SOURCES}
do
    basename=`echo "${source}" | rev | cut -d/ -f1 | rev`
    ${COMPILE_COMMAND} ${TOTAL_INCLUDES} -c "${source}.c" -o "build/${VENDOR}/${basename}.o"
done


for obj in ${EXTRA_OBJECT_FILES}
do
    cp "${EXTRA_OBJECT_FILES_FOLDER}/${obj}" build/${VENDOR}
done

${LINK_COMMAND} build/${VENDOR}/*.o -o build/${VENDOR}/instamsg
