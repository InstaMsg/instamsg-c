wget http://updatesite.sierrawireless.com/developerStudio3/debian/devstudio.list -O - | sudo tee /etc/apt/sources.list.d/devstudio.list
wget http://updatesite.sierrawireless.com/developerStudio3/debian/devstudio.key -O - | sudo apt-key add -
sudo apt-get update
sudo apt-get install devstudio-openat-gcc
