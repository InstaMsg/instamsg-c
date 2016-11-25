set -e

sudo rm -r stlink

unzip stlink.zip -d .
cd stlink
make
