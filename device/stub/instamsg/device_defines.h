#ifndef DEVICE_DEFINES
#define DEVICE_DEFINES

/*
 * By default.. we use the in-built functionality, which uses variable-argument lists.
 *
 * However, this implementation has shown to give problems for Harvard/RISC architectures, so such devices will need
 * to provide the reference to an implementation shipped with their compiler-environment.
 */
#define sg_sprintf DEFAULT_SPRINTF

#define MAX_BUFFER_SIZE 1000
#define MAX_HEAP_SIZE (6 * (MAX_BUFFER_SIZE + HEADER_SIZE))

#define MODBUS_COMMAND_BUFFER_SIZE  150

#endif
