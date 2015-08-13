. ${1}

TOTAL_INCLUDES=`echo                                                \
        ${SYSTEM_INCLUDES}                                          \
        -Isrc/vendors/${VENDOR}/                                    \
        -I ../MQTTPacket/src    `


SOURCES=`echo                                                       \
        stdoutsub                                                   \
        src/common/instamsg                                         \
        src/common/log                                              \
        src/common/httpclient                                       \
        src/common/json                                             \
                                                                    \
        ../MQTTPacket/src/MQTTFormat                                \
        ../MQTTPacket/src/MQTTPacket                                \
        ../MQTTPacket/src/MQTTDeserializePublish                    \
        ../MQTTPacket/src/MQTTConnectClient                         \
        ../MQTTPacket/src/MQTTSubscribeClient                       \
        ../MQTTPacket/src/MQTTSerializePublish                      \
        ../MQTTPacket/src/MQTTConnectServer                         \
        ../MQTTPacket/src/MQTTSubscribeServer                       \
        ../MQTTPacket/src/MQTTUnsubscribeServer                     \
        ../MQTTPacket/src/MQTTUnsubscribeClient                     \
                                                                    \
        src/system/${SYSTEM_IMPL}                                   \
        src/time/${TIME_IMPL}                                       \
        src/communication/network/${NETWORK_COMMUNICATION_IMPL}     \
        src/communication/fs/${FS_COMMUNICATION_IMPL}               \
        src/communication/command/${COMMAND_COMMUNICATION_IMPL}     \
        src/communication/serial/${SERIAL_COMMUNICATION_IMPL} `


mkdir -p build/${VENDOR}
rm -f build/${VENDOR}/*.o
rm -f build/${VENDOR}/instamsg

for source in ${SOURCES}
do
    basename=`echo "${source}" | rev | cut -d/ -f1 | rev`
    ${COMPILE_COMMAND} ${TOTAL_INCLUDES} -c "${source}.c" -o "build/${VENDOR}/${basename}.o"
done


for obj in ${EXTRA_OBJECT_FILES}
do
    cp "${EXTRA_OBJECT_FILES_FOLDER}/${obj}" build/${VENDOR}
done

TOTAL_OBJECT_FILES=""
for obj in `ls build/${VENDOR}`
do
    TOTAL_OBJECT_FILES=`echo ${TOTAL_OBJECT_FILES} build/${VENDOR}/${obj}`
done

${LINK_COMMAND} -o build/${VENDOR}/instamsg ${TOTAL_OBJECT_FILES}
