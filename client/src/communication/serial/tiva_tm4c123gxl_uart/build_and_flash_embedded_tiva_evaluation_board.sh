set -e

export PATH=$PATH:$PWD/../../../../../embedded-flash-environment/gcc-arm-none-eabi-4_9-2015q2/bin

mkdir -p tiva_build
rm -f tiva_build/*

rm -f *.o
rm -f *.d


MCU="TM4C123GH6PM"
TIVAWARE_PATH="../../../../../embedded-flash-environment/tivaware"

SOURCES=`echo   tiva_startup_gcc                                        \
                tiva_main                                               \
                ../tiva_tm4c123gxl_serial_uart `

EXTRA_OBJECT_FILES=`echo    gpio.o `
EXTRA_OBJECT_FILES_FOLDER="../../../../../embedded-flash-environment/tivaware/driverlib/gcc"

CC="arm-none-eabi-gcc"
LD="arm-none-eabi-ld"
OBJCOPY="arm-none-eabi-objcopy"
LD_SCRIPT="../../../../../embedded-flash-environment/tiva-template/${MCU}.ld"

CFLAGS=" -g -mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -Os -ffunction-sections -fdata-sections -MD -std=c99 -Wall -pedantic -DPART_${MCU} -c -I${TIVAWARE_PATH} -I../../../vendors/tiva_tm4c123gxl -DTARGET_IS_BLIZZARD_RA1 "
LDFLAGS=" -T ${LD_SCRIPT} --entry ResetISR --gc-sections "



for source in ${SOURCES}
do
    basename=`echo "${source}" | rev | cut -d/ -f1`
    ${CC} ${CFLAGS} ${INCLUDES} -c "${source}.c" -o "tiva_build/${basename}.o"
done


for obj in ${EXTRA_OBJECT_FILES}
do
    cp "${EXTRA_OBJECT_FILES_FOLDER}/${obj}" tiva_build
done


${LD} ${LDFLAGS} tiva_build/*.o -o tiva_build/a.out
${OBJCOPY} -O binary tiva_build/a.out tiva_build/image_to_flash

../../../../../embedded-flash-environment/lm4tools/lm4flash/lm4flash ./tiva_build/image_to_flash
