#ifndef INSTAMSG_WATCHDOG_COMMON
#define INSTAMSG_WATCHDOG_COMMON

void watchdog_init();
void watchdog_reset_and_enable(int n, char *callee);
void watchdog_disable();

#endif
