#include "../../../common/instamsg/driver/include/globals.h"

#include "./device_adc.h"

/*
 * This method initializes and connects to the ADC-interface.
 */
void connect_underlying_adc_medium_guaranteed(ADC *adc)
{
}


/*
 * This method read the value from the ADC.
 */
long adc_read_value_sync(ADC *adc)
{
    return FAILURE;
}


/*
 * This method cleans up the ADC-interface.
 */
void release_underlying_adc_medium_guaranteed(ADC *adc)
{
}
