#ifndef IOEYE_ADC
#define IOEYE_ADC

typedef struct ADC ADC;

struct ADC
{
    /* ============================= THIS SECTION MUST NOT BE TEMPERED ==================================== */
    int (*read_value_sync)(ADC *adc, void *extra_args);
    /* ============================= THIS SECTION MUST NOT BE TEMPERED ==================================== */



    /* ============================= ANY EXTRA FIELDS GO HERE ============================================= */
    /* ============================= ANY EXTRA FIELDS GO HERE ============================================= */
};

#endif
