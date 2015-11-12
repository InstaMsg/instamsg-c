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
 *
 * Note that any extra-parameters (for eg. in a multi-channel ADC, we need to specify the channel for the ADC),
 * "extra_args: parameter will be used.
 *
 * Of course, how and when the "extra_args" parameter is used, is upto the device-implementor.
 * That is why it has been made as a void-pointer :)
 */
int adc_read_value_sync(ADC *adc, void *extra_args)
{
    return FAILURE;
}


/*
 * This method cleans up the ADC-interface.
 */
void release_underlying_adc_medium_guaranteed(ADC *adc)
{
}
