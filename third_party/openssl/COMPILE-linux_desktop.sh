set -e
set -x

# Enable/Disable features via https://gist.github.com/steakknife/8247726
#
./Configure                     \
    linux-elf

make
sudo cp libssl.a /usr/lib/i386-linux-gnu
sudo cp libcrypto.a /usr/lib/i386-linux-gnu
ls -lrt /usr/lib/i386-linux-gnu/libssl.a
ls -lrt /usr/lib/i386-linux-gnu/libcrypto.a

