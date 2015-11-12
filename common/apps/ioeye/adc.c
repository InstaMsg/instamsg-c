#include "./include/adc.h"

void init_adc(ADC *adc, void *arg)
{
    /* Register the callback */
	adc->read_value_sync = adc_read_value_sync;

    connect_underlying_adc_medium_guaranteed(adc);
}


void release_adc(ADC *adc)
{
    release_underlying_adc_medium_guaranteed(adc);
}

