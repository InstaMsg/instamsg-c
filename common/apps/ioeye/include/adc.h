#ifndef IOEYE_ADC_COMMON
#define IOEYE_ADC_COMMON

#include "device_adc.h"

/*
 * Global-functions callable.
 */
void init_adc(ADC *adc, void *arg);
void release_adc(ADC *adc);


/*
 * Must not be called directly.
 * Instead must be called as ::
 *
 *      adc->read_value_sync
 */
int adc_read_value_sync(ADC *adc, void *extra_args);


/*
 * Internally Used.
 * Must not be called by anyone.
 */
void connect_underlying_adc_medium_guaranteed(ADC *adc);
void release_underlying_adc_medium_guaranteed(ADC *adc);

#endif
