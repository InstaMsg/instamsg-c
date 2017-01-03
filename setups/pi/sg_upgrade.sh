#!/bin/sh


#######################################################################
HOME_DIRECTORY="/home/sensegrow"
PLATFORM="pi"
#######################################################################

set -e

cd "${HOME_DIRECTORY}"
rm -f setup.zip*
rm -rf setup

wget "https://github.com/InstaMsg/instamsg-c/blob/master/setups/${PLATFORM}/setup.zip?raw=true"

mv setup.zip* setup.zip
unzip setup.zip -d .

cd setup
chmod 777 ./sg_setup.sh
./sg_setup.sh

