#ifndef AR501_TELIT
#define AR501_TELIT

#include "../driver/include/instamsg.h"

static volatile char errorObtained;
static volatile char noCarrierObtained;
static volatile char showCommandOutput;

static unsigned int actualBytesRead;
static unsigned int ind;
static char *readBuffer;
static char result[MAX_BUFFER_SIZE];

#define LENGTH_OF_COMMAND 0

void SEND_CMD_AND_READ_RESPONSE_ON_UART1(const char *command, int len, char *desiredOutputBuffer, const char *delimiter);
void get_actual_command_output_for_command_results_with_ok_status(const char *command, const char *completeOutput, char *usefulOutput);
int parseNumberFromEndOfString(char *pch, char limiter);

#endif
