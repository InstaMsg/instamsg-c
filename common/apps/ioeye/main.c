#include "../../instamsg/driver/include/instamsg.h"

#include "./include/modbus.h"
#include "./include/adc.h"
#include "./include/data_logger.h"


Modbus singletonModbusInterface;
ADC singletonADCInterface;


static int onConnect()
{
    modbusOnConnectProcedures(&singletonModbusInterface);
    return SUCCESS;
}


static void coreLoopyBusinessLogicInitiatedBySelf()
{
    static int tick = 0;
    tick++;

    if((tick % 60) == 0)
    {
        tick = 0;
        modbusProcedures(&singletonModbusInterface);
    }

    info_log("ADC value = [%u]", adc_read_value_sync(&singletonADCInterface));
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

    singletonADCInterface.pins = GPIO_PIN_1;
    singletonADCInterface.channel = ADC_CTL_CH2;
    init_adc(&singletonADCInterface, NULL);

    start(onConnect, NULL, NULL, coreLoopyBusinessLogicInitiatedBySelf, 1);
}
