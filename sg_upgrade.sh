#!/bin/sh

. ./upgrade_params

set -e

cd "${HOME_DIRECTORY}"
rm -f setup.zip*
rm -rf setup

wget "https://github.com/InstaMsg/instamsg-c/blob/master/setups/${PLATFORM}/setup.zip?raw=true"

mv setup.zip* setup.zip
unzip setup.zip -d .

cd setup

prev_version=`cat "${HOME_DIRECTORY}/current_version"`
new_version=`cat ./sg_setup.sh | grep "PKG_VERSION" | cut -d\" -f 2`

if [ -z "${prev_version}" ]
then
    echo "Current-Version not available ... aborting .."
    exit
fi

if [ -z "${new_version}" ]
then
    echo "New Package-Version not available ... aborting .."
    exit
fi

if [ "${new_version}" -gt "${prev_version}" ]
then
    chmod 777 ./sg_setup.sh
    ./sg_setup.sh

    echo "${new_version}" > "${HOME_DIRECTORY}/current_version"
else
    echo "No new version available !!"
fi

