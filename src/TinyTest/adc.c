/*
 * adc.c
 *
 * Created: 3/26/2023 12:02:21 PM
 *  Author: ashpl
 */ 

#include "adc.h"

#include <avr/io.h>
#include "util.h"

void init_adc() {
	// Set resolution
	ADC0_CTRLA = 0b00000000;
	ADC0_CTRLB = 0b00000110;
	ADC0_CTRLC = 0b01010000;
	ADC0_CTRLD = 0b10110000;
	ADC0_CTRLE = 0b00000000;
	ADC0_MUXPOS = 0b00000111;
	ADC0_CALIB = 0b0;
	
}

uint16_t run_adc_sample() {
	enable_prescaler();
	// Enable the ADC
	ADC0_CTRLA = 0b00000001;
	
	// Command a read
	ADC0_COMMAND = 0b1;
	
	// Wait for the read to occur
	while (ADC0_COMMAND & 0b1) {}

	// Then wait for 
	while (!(ADC0_INTFLAGS & 0b1)) {}
	
	uint16_t result = ADC0_RES / 64;
	
	// Disable
	ADC0_CTRLA = 0b00000000;

	disable_prescaler();
	
	return result;
}
