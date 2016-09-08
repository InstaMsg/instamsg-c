Provisioning-Id on InstaMsg-Server
==================================

MAC-Address of "eth0" interface, as displayed by running the command “ifconfig”..





Provisioning on InstaMsg-Server
==================================

1)    
Enter test in (Re-)Activate Provisioning




[ONLY FOR energy-meter BINARY] Setting up the modbus-serial port on /dev/ttyUSB0
==================================================================================

1)    
Run the following command, with the energy-meter USB attached ::

lsusb

Let the output be ::

Bus 001 Device 001: ID 1d6b:0002 Linux Foundation 2.0 root hub    
Bus 001 Device 010: ID 0403:6001 FTDI FT232 USB-Serial (UART) IC    
Bus 001 Device 005: ID 0402:5632 ALi Corp. USB 2.0 Host-to-Host Link     


2)     
Also, let the USB identifier be /dev/ttyUSB1 (with the energy-meter USB attached) ::     
So, run the following ::     

udevadm info -a -n /dev/ttyUSB1 | grep '{serial}' | head -n1   

Let the output be    

ATTRS{serial}=="A6008isP"    



3)    
Run the following command, with the the energy-meter USB removed ::   

lsusb    

Let the output be ::     


Bus 001 Device 001: ID 1d6b:0002 Linux Foundation 2.0 root hub     
Bus 001 Device 005: ID 0402:5632 ALi Corp. USB 2.0 Host-to-Host Link     



4)    
There must be a difference of exactly one line in the outputs of 1) and 3).    
Identify that line.    

In the above case, the differentiating-line is ::     

Bus 001 Device 010: ID 0403:6001 FTDI FT232 USB-Serial (UART) IC     


5)    
Run the following commands ::    

sudo touch /etc/udev/rules.d/99-usb-serial.rules     
sudo chmod 777 /etc/udev/rules.d/99-usb-serial.rules    


6)    
Append the following line in /etc/udev/rules.d/99-usb-serial.rules    

SUBSYSTEM=="tty", ATTRS{idVendor}=="0403", ATTRS{idProduct}=="6001", ATTRS{serial}=="A6008isP", SYMLINK+="ttyUSB0"    


The values for the bolded-variables, will be as per the outputs in the previous commands.    



PART-B : Setting up the binary
==================================

All these steps need to be done on the linux-machine device itself.


1)     
Run the following commands ::   

[ONLY FOR BINARIES REQUIRING CAMERA-STREAMING]    

sudo apt-get update    
sudo apt-get install libgstreamer1.0*    
sudo apt-get install gstreamer1.0*    

[COMMON]     

sudo apt-get install git    
cd ~
git clone https://github.com/InstaMsg/instamsg-c.git     
cd instamsg-c/setups/linux_desktop    
sh setup.sh /path/of/binary    
tail -f /home/sensegrow/instamsg.log    


2)    
Running logs should be seen by the use of “tail” command.     
Also, ensure everything is fine on the InstaMsg-Server.      


3)     
Once step 3 is satisfactory, run the following commands ::     

cd ~    
sudo rm -r instamsg-c    


4)     
[ONLY FOR BINARIES REQUIRING CAMERA-STREAMING]   

i)    
Change the value of config-parameter MEDIA_REPLY_MESSAGE_WAIT_INTERVAL from 120 to 600 on the InstaMsg-Server.     

ii)    
Change the value of config-parameter MEDIA_STREAMING_ENABLED from 0 to 1 on the InstaMsg-Server.     

iii)     
Restart-Client from InstaMsg-Server.      

iv)    
Now, a new config-parameter MEDIA_STREAMING_URL will emerge.     
Fill its value, after confirming that the Camera-URL works fine on VLC-media-server     

For example, let’s say the Camera-URL that works fine on VLC-Media-Server is ::    

rtsp://admin:accrete@192.168.0.5:554/cam/realmonitor?channel=1&subtype=0    

So, the URL that needs to be filled in the config is ::    

rtspsrc location=rtsp://admin:accrete@192.168.0.5:554/cam/realmonitor?channel=1&subtype=0 ! rtph264depay ! avdec_h264 ! videoscale ! videorate skip-to-first=true ! videoconvert ! queue ! capsfilter caps=video/x-raw,framerate=1/1 ! x264enc byte-stream=true me=1 subme=6 bitrate=50 speed-preset=1 ! video/x-h264 ! rtph264pay pt=96 mtu=65000 ! udpsink host=~HOST~ port=~PORT~     

v)    
After filling in the MEDIA_STREAMING_URL config, restart client from InstaMsg-Server.    
