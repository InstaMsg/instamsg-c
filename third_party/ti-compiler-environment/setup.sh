# All courtesy the GOD at http://chrisrm.com/howto-develop-on-the-ti-tiva-launchpad-using-linux/

# Some repositories might not be updatable, so using "set -e" from here afterwards.
sudo apt-get update

set -e

sudo apt-get install flex bison libgmp3-dev libmpfr-dev libncurses5-dev libmpc-dev autoconf texinfo build-essential libftdi-dev python-yaml zlib1g-dev libtool
# I needed these as well flashing over USB
sudo apt-get install libusb-1.0-0 libusb-1.0-0-dev


sudo rm -rf gcc-arm-none-eabi-4_9-2015q2
tar -xvf gcc-arm-none-eabi-4_9-2015q2-20150609-linux.tar.bz2 -C .


export PATH=$PATH:$PWD/../ti-compiler-environment/gcc-arm-none-eabi-4_9-2015q2/bin

sudo rm -r tivaware
mkdir -p tivaware
cd tivaware
cp ../SW-TM4C-2.1.1.71.exe .
unzip SW-TM4C-2.1.1.71.exe
make
cd ..


sudo rm -r tiva-template
git clone git@github.com:uctools/tiva-template


sudo rm -r lm4tools
git clone git://github.com/utzig/lm4tools.git
cd lm4tools/lm4flash/
make
cd ../..


sudo rm -r openocd
git clone http://openocd.zylin.com/openocd
cd openocd
git fetch http://openocd.zylin.com/openocd refs/changes/63/2063/1
git checkout FETCH_HEAD
git submodule init
git submodule update
./bootstrap
./configure --enable-ti-icdi --prefix=`pwd`/..
make -j3
make install
cd ..


