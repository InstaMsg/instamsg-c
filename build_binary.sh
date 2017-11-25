APP=${1}
. common/apps/${APP}/Makefile

VENDOR=$2
. device/${VENDOR}/instamsg/Makefile


TOTAL_INCLUDES=`echo                                                                            \
        ${SYSTEM_INCLUDES}                                                                      \
        -Idevice/${VENDOR}/instamsg/                                                            \
        -Idevice/${VENDOR}/ioeye/                                                               \
        -Icommon/instamsg/mqtt/                                                                 `


SOURCES=`echo                                                                                   \
        ${APP_SOURCES}                                                                          \
        ${EXTRA_SOURCES}                                                                        \
        common/instamsg/driver/instamsg.c                                                       \
        common/instamsg/driver/log.c                                                            \
        common/instamsg/driver/json.c                                                           \
        common/instamsg/driver/globals.c                                                        \
        common/instamsg/driver/sg_mem.c                                                         \
        common/instamsg/driver/sg_stdlib.c                                                      \
        common/instamsg/driver/socket.c                                                         \
        common/instamsg/driver/config.c                                                         \
        common/instamsg/driver/watchdog.c                                                       \
        common/instamsg/driver/file_system.c                                                    \
        common/instamsg/driver/gps.c                                                            \
        common/instamsg/driver/time.c                                                           \
        common/instamsg/driver/hex.c                                                            \
        common/instamsg/driver/misc.c                                                           \
        common/instamsg/driver/at.c                                                             \
        common/instamsg/driver/gpio.c                                                           \
        common/instamsg/driver/cron.c                                                           \
        common/instamsg/driver/payload.c                                                        \
        common/instamsg/driver/control.c                                                        \
        common/instamsg/driver/fs_config.c                                                      \
        common/instamsg/driver/fs_data_logger.c                                                 \
                                                                                                \
        common/instamsg/wolfssl/ssl.c                                                           \
        common/instamsg/wolfssl/tls.c                                                           \
        common/instamsg/wolfssl/keys.c                                                          \
        common/instamsg/wolfssl/internal.c                                                      \
        common/instamsg/wolfssl/wolfio.c                                                        \
        common/instamsg/wolfcrypt/src/memory.c                                                  \
        common/instamsg/wolfcrypt/src/wc_port.c                                                 \
        common/instamsg/wolfcrypt/src/asn.c                                                     \
        common/instamsg/wolfcrypt/src/rsa.c                                                     \
        common/instamsg/wolfcrypt/src/coding.c                                                  \
        common/instamsg/wolfcrypt/src/hash.c                                                    \
        common/instamsg/wolfcrypt/src/hmac.c                                                    \
        common/instamsg/wolfcrypt/src/sha.c                                                     \
        common/instamsg/wolfcrypt/src/sha256.c                                                  \
        common/instamsg/wolfcrypt/src/arc4.c                                                    \
        common/instamsg/wolfcrypt/src/random.c                                                  \
        common/instamsg/wolfcrypt/src/error.c                                                   \
        common/instamsg/wolfcrypt/src/tfm.c                                                     \
        common/instamsg/wolfcrypt/src/integer.c                                                 \
        common/instamsg/wolfcrypt/src/logging.c                                                 \
        common/instamsg/wolfcrypt/src/ecc.c                                                     \
        common/instamsg/wolfcrypt/src/wolfmath.c                                                \
                                                                                                \
        common/instamsg/mqtt/src/MQTTFormat.c                                                   \
        common/instamsg/mqtt/src/MQTTPacket.c                                                   \
        common/instamsg/mqtt/src/MQTTDeserializePublish.c                                       \
        common/instamsg/mqtt/src/MQTTConnectClient.c                                            \
        common/instamsg/mqtt/src/MQTTSubscribeClient.c                                          \
        common/instamsg/mqtt/src/MQTTSerializePublish.c                                         \
        common/instamsg/mqtt/src/MQTTConnectServer.c                                            \
        common/instamsg/mqtt/src/MQTTSubscribeServer.c                                          \
        common/instamsg/mqtt/src/MQTTUnsubscribeServer.c                                        \
        common/instamsg/mqtt/src/MQTTUnsubscribeClient.c                                        \
                                                                                                \
        device/${VENDOR}/instamsg/device_socket.c                                               \
        device/${VENDOR}/instamsg/device_serial_logger.c                                        \
        device/${VENDOR}/instamsg/device_time.c                                                 \
        device/${VENDOR}/instamsg/device_watchdog.c                                             \
        device/${VENDOR}/instamsg/device_config.c                                               \
        device/${VENDOR}/instamsg/device_proxy.c                                                \
        device/${VENDOR}/instamsg/device_gpio.c                                                 \
        device/${VENDOR}/instamsg/device_file_system.c                                          \
        device/${VENDOR}/instamsg/device_data_logger.c                                          \
        device/${VENDOR}/instamsg/device_gps.c                                                  \
        device/${VENDOR}/instamsg/device_misc.c                                                 \
        device/${VENDOR}/instamsg/device_cron.c                                                 \
        device/${VENDOR}/instamsg/device_control.c                                              \
        device/${VENDOR}/instamsg/device_at.c                                                   `


SOURCES=`echo ${SOURCES} | sed -e 's|VENDOR|'"${VENDOR}"'|g'`

OUT_DIR="../../ioeye-client-libs/ioeye-${VENDOR}"
mkdir -p ${OUT_DIR}


COMPILE_COMMAND="${COMPILE_COMMAND} ${APP_COMPILER_DIRECTIVES} "

INSTAMSG_VERSION=`grep INSTAMSG_VERSION common/instamsg/driver/include/globals.h | cut -d\  -f 3 | cut -d\" -f 2`
DEVICE_VERSION=`grep DEVICE_VERSION device/${VENDOR}/instamsg/device_defines.h | cut -d\  -f 3 | cut -d\" -f 2`
COMPILED_FILE_NAME="${APP}_${VENDOR}_${INSTAMSG_VERSION}_${DEVICE_VERSION}"

TOTAL_COMPILE_COMMAND="${COMPILE_COMMAND} ${TOTAL_INCLUDES} ${SOURCES} -o ${OUT_DIR}/${COMPILED_FILE_NAME}"

for obj in ${EXTRA_OBJECT_FILES}
do
    TOTAL_COMPILE_COMMAND="${COMPILE_COMMAND} ${TOTAL_INCLUDES} ${SOURCES} ${OUT_DIR}/* -o ${OUT_DIR}/${COMPILED_FILE_NAME}"
    cp ${obj} ${OUT_DIR}
done

TOTAL_COMPILE_COMMAND="${TOTAL_COMPILE_COMMAND} ${POST_LINKED_LIBS} "
${TOTAL_COMPILE_COMMAND}

FINAL_FILE_NAME="${COMPILED_FILE_NAME}_to-flash"
for cmd in "${FINAL_COMMANDS[@]}"
do
    cmd=`echo ${cmd} | sed -e 's|OUT_DIR|'"${OUT_DIR}"'|g'`
    cmd=`echo ${cmd} | sed -e 's|COMPILED_FILE_NAME|'"${COMPILED_FILE_NAME}"'|g'`
    cmd=`echo ${cmd} | sed -e 's|FINAL_FILE_NAME|'"${FINAL_FILE_NAME}"'|g'`

    $cmd
done
