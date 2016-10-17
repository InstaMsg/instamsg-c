set -e
set -x

# Enable/Disable features via https://gist.github.com/steakknife/8247726
#
./Configure                     \
                no-ec           \
                no-ecdsa        \
                no-ecdh         \
                no-ec2m         \
                no-mdc2         \
                no-idea         \
                no-capieng      \
                no-bf           \
                no-camellia     \
                no-whirlpool    \
                no-ssl2         \
                no-threads      \
                no-krb5         \
    linux-elf

make depend
make
sudo cp libssl.a /usr/lib/i386-linux-gnu
sudo cp libcrypto.a /usr/lib/i386-linux-gnu
ls -lrt /usr/lib/i386-linux-gnu/libssl.a
ls -lrt /usr/lib/i386-linux-gnu/libcrypto.a

