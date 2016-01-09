# Device-specific extra includes
#
SYSTEM_INCLUDES=`echo  	`


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
# Please note that compilation-and-linking is done all in one step
# (and not in two separate "compile" and "link" steps).
#
COMPILE_COMMAND="gcc "


# Any final libraries that need to be linked.
#
POST_LINKED_LIBS=""


# Any final commands that need to be run.
#
# Most-likely, these commands would (prepare to) flash the
# binaries on the end-device.
#
# For this, these commands would need to require the location
# of the output-directory of "instamsg" binary.
#
# Whenever this is required, use OUT_DIR (without any dollar
# sign or anything), and the main builder-script will take
# care of that.
#
# For example,
#
# 		device_flashing_binary OUT_DIR/instamsg /dev/devID
#
#declare -a FINAL_COMMANDS=("command-1" "command-2" "and" "so" "on")
