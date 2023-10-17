/*
 * adc.h
 *
 * Created: 3/26/2023 12:02:30 PM
 *  Author: ashpl
 */ 


#ifndef ADC_H_
#define ADC_H_

#include <stdint.h>

void init_adc();
uint16_t run_adc_sample();

#endif /* ADC_H_ */