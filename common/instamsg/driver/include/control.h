#ifndef INSTAMSG_CONTROL
#define INSTAMSG_CONTROL

struct ControlCommandParams
{
    char portName[50];
    char portAddress[50];
    char hostAddress[50];
    char hostPort[50];

    char command[100];
};

void processControlCommand(char *controlCommandPayload);


void do_process_control_action_command(struct ControlCommandParams *controlCommandParams);
#endif
