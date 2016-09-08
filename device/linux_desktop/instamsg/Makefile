# Device-specific extra includes
#
SYSTEM_INCLUDES=`echo                                               		\
        -isystem/usr/lib/gcc/i686-linux-gnu/4.8/include                   	\
        -isystem/usr/local/include                                        	\
        -isystem/usr/lib/gcc/i686-linux-gnu/4.8/include-fixed             	\
        -isystem/usr/include/i386-linux-gnu                               	\
        -isystem/usr/include  												\
		-isystem/usr/include/gstreamer-1.0                                  \
		-isystem/usr/include/glib-2.0                                       \
	   	-isystem/usr/lib/i386-linux-gnu/glib-2.0/include					`


# Device-specific extra source-files.
#
EXTRA_SOURCES=`echo															\
			  device/${VENDOR}/common/file_utils.c							\
			  device/${VENDOR}/common/serial_port_utils.c					`


# Device-specific extra object-files that need to linked.
#
EXTRA_OBJECT_FILES=`echo	`


# Compile-cum-Link command for the platform.
#
COMPILE_COMMAND="gcc -nostdinc -pedantic -pthread "


# Any final libraries that need to be linked.
#
POST_LINKED_LIBS=" -lgstreamer-1.0 -lgobject-2.0 -lglib-2.0 "


# Any final commands that need to be run.
#
# Most-likely, these commands would (prepare to) flash the binaries on the end-device.
# For this, these commands would need to require the location of the output-directory of "instamsg" binary.
# Whenever this is required, use OUT_DIR (without any dollar sign or anything), and the main builder-script
# will take care of that.
#
# For example, device_flashing_binary OUT_DIR/instamsg /dev/devID
#
declare -a FINAL_COMMANDS=("")
