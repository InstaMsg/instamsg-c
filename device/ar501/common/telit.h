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

void SEND_CMD_AND_READ_RESPONSE_ON_UART1(const char *command, int len, char *desiredOutputBuffer, const char *delimiter);
void get_actual_command_output_for_command_results_with_ok_status(const char *command, const char *completeOutput, char *usefulOutput);
int parseNumberFromEndOfString(char *pch, char limiter);

#endif
