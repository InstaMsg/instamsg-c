#ifndef DEVICE_DEFINES
#define DEVICE_DEFINES

/*
 * By default.. we use the in-built functionality, which uses variable-argument lists.
 *
 * However, this implementation has shown to give problems for Harvard/RISC architectures, so such devices will need
 * to provide the reference to an implementation shipped with their compiler-environment.
 */
#define sg_sprintf DEFAULT_SPRINTF

#endif