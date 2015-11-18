#include "../../instamsg/driver/include/instamsg.h"

#include "./include/modbus.h"
#include "./include/data_logger.h"


Modbus singletonModbusInterface;


static int onConnect()
{
    modbusOnConnectProcedures(&singletonModbusInterface);
    return SUCCESS;
}


static void coreLoopyBusinessLogicInitiatedBySelf()
{
    modbusProcedures(&singletonModbusInterface);
}


int main(int argc, char** argv)
{
#ifdef FILE_SYSTEM_INTERFACE_ENABLED
    globalSystemInit(LOG_FILE_PATH);
#else
    globalSystemInit(NULL);
#endif
    init_data_logger();
    init_modbus(&singletonModbusInterface, NULL);

    start(onConnect, NULL, NULL, coreLoopyBusinessLogicInitiatedBySelf, 60);
}
