set -x
set -e

sed -i 's/GRUB_CMDLINE_LINUX_DEFAULT=.*/GRUB_CMDLINE_LINUX_DEFAULT="quiet splash intel_idle.max_cstate=0 processor.max_cstate=0 idle=poll"/g' /etc/default/grub
update-grub

apt-get -y remove pm-utils
dpkg -i pm-utils_1.4.1-13ubuntu0.2_all.deb

kill -9 `pgrep -x instamsg` || true
cp energy-meter_lanner-pc_4.1.0_4.1.0 /home/sensegrow/instamsg

echo
echo
echo "PLEASE REBOOT THE MACHINE NOW ..."
echo

