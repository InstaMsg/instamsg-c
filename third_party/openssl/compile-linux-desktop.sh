set -e
set -x

./Configure no-ec linux-elf
make depend
make
sudo cp libssl.a /usr/lib/i386-linux-gnu
sudo cp libcrypto.a /usr/lib/i386-linux-gnu

