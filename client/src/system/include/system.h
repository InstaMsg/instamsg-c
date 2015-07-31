#ifndef INSTAMSG_SYSTEM
#define INSTAMSG_SYSTEM

typedef struct System System;
struct System
{
    void *obj;

    /*
     * Note that the "buf" will be all-0-initialized from the callee, so the vendor-implementation
     * does not need to bother about that.
     */
    void (*getManufacturer)(System *system, unsigned char *buf, int maxValueLenAllowed);


    /*
     * Note that the "buf" will be all-0-initialized from the callee, so the vendor-implementation
     * does not need to bother about that.
     */
    void (*getSerialNumber)(System *system, unsigned char *buf, int maxValueLenAllowed);

};

System* get_new_system();
void release_system(System*);

#endif
