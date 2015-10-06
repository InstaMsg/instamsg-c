# Device-specific extra includes
#
SYSTEM_INCLUDES=`echo                                               		\
        -isystem/usr/lib/gcc/i686-linux-gnu/4.8/include                   	\
        -isystem/usr/local/include                                        	\
        -isystem/usr/lib/gcc/i686-linux-gnu/4.8/include-fixed             	\
        -isystem/usr/include/i386-linux-gnu                               	\
        -isystem/usr/include  												`


# Uncomment the following, if device provides File-System-Implementation.
#
FS_COMMUNICATION_IMPL="Y"


# Uncomment the following, if device is a GSM-device.
# Uncommenting this, causes the device to wait for a SMS from the InstaMsg-server, containing the following params ::
#
# 		APN
# 		APN-Username
# 		APN-Password
#
# Above parameters are required to make a connection to the Mobile-APN.
#
#IS_GSM_DEVICE="Y"


# Uncomment the following, if the socket-implementation provided by the device is secured.
#
#IS_SSL="Y"


# Device-specific extra source-files.
#
EXTRA_SOURCES=`echo   `


# Device-specific extra object-files that need to linked.
#
EXTRA_OBJECT_FILES=`echo   `


# Compile-cum-Link command for the platform.
#
COMPILE_COMMAND="gcc -g -nostdinc -pedantic "


# Any final commands that need to be run.
#
declare -a FINAL_COMMANDS=("")
