sudo rm -r stlink

set -e
unzip stlink.zip -d .
cd stlink
cmake .
make
