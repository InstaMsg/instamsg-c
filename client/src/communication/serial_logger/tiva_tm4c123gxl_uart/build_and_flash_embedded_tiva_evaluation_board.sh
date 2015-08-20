set -e

export PATH=$PATH:$PWD/../../../../../embedded-flash-environment/gcc-arm-none-eabi-4_9-2015q2/bin

mkdir -p tiva_build
rm -f tiva_build/*


TIVAWARE_PATH="../../../../../embedded-flash-environment/tivaware"

MCU="TM4C123GH6PM"
LD_SCRIPT="../../../../../embedded-flash-environment/tiva-template/${MCU}.ld"

SOURCES=`echo tiva_startup_gcc tiva_main ../../../common/globals`


for source in ${SOURCES}
do
    basename=`echo "${source}" | rev | cut -d/ -f1`
    arm-none-eabi-gcc -g -mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -Os -ffunction-sections -fdata-sections -MD -std=c99 -Wall -pedantic -DPART_${MCU} -c -I${TIVAWARE_PATH} -I../../../vendors/tiva_tm4c123gxl -DTARGET_IS_BLIZZARD_RA1 -c "${source}.c" -o "tiva_build/${basename}.o"
done


arm-none-eabi-gcc -T ${LD_SCRIPT} --entry ResetISR -Wl,--gc-sections tiva_build/*.o -o tiva_build/a.out
arm-none-eabi-objcopy -O binary tiva_build/a.out tiva_build/image_to_flash

../../../../../embedded-flash-environment/lm4tools/lm4flash/lm4flash ./tiva_build/image_to_flash
