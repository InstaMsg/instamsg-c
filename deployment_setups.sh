rm -f pi_setup.zip
zip -r pi_setup.zip device/pi/setup

rm -f maestro_e220_setup.zip
zip -r maestro-e220_setup.zip device/maestro-e220/setup

rm -f lanner-pc_setup.zip
zip -r lanner-pc_setup.zip device/lanner-pc/setup
mv lanner-pc_setup.zip common/ioeye/binaries/energy-meter
