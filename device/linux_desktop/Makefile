# Device-specific extra includes
#
SYSTEM_INCLUDES=`echo                                               		\
        -isystem/usr/lib/gcc/i686-linux-gnu/4.8/include                   	\
        -isystem/usr/local/include                                        	\
        -isystem/usr/lib/gcc/i686-linux-gnu/4.8/include-fixed             	\
        -isystem/usr/include/i386-linux-gnu                               	\
        -isystem/usr/include  												`


# The platform on which InstaMsg needs to be built.
# This name must be equal to the name of the folder, created/copied from "src/stub".
#
VENDOR="linux_desktop"


# This parameter should be set, if file-system-support needs to be defined.
# If file-system-support is not required, then this parameter MUST NOT be defined.
#
FS_COMMUNICATION_IMPL="Y"


# This parameter should be set, if the device is a GSM-one.
# If not, then this parameter MUST NOT be defined.
#
# Setting of this parameter is required (when applicable), to enable provisioning.
#
#IS_GSM_DEVICE="Y"


# This parameter should be set, if the communication is over SSL.
# If not, then this parameter MUST NOT be defined.
#
#IS_SSL="Y"


# Device-specific extra source-files.
#
EXTRA_SOURCES=""


# Device-specific extra object-files that need to linked.
#
EXTRA_OBJECT_FILES=""


# Compile-cum-Link command for the platform.
#
COMPILE_COMMAND="gcc -g -nostdinc -pedantic "
