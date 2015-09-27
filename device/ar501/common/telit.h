#ifndef AR501_TELIT
#define AR501_TELIT

#include "../driver/include/instamsg.h"

volatile char errorObtained;
volatile char noCarrierObtained;
volatile char showCommandOutput;

unsigned int actualBytesRead;
unsigned int ind;
char *readBuffer;
char result[MAX_BUFFER_SIZE];

#define LENGTH_OF_COMMAND 0


/*
 * Global-functions callable.
 */
void run_simple_at_command_and_get_output(const char *command, char *usefulOutput);


/*
 * Internally Used.
 * Must not be called by anyone.
 */
void SEND_CMD_AND_READ_RESPONSE_ON_UART1(const char *command, int len, char *desiredOutputBuffer, const char *delimiter);
int parseNumberFromEndOfString(char *pch, char limiter);

#endif
