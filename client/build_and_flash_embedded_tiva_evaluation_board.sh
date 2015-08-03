set -e

. ./tiva_Env.ini
make -f tiva_Makefile

../embedded-flash-environment/lm4tools/lm4flash/lm4flash ./tiva_build/tiva_main.bin
