/*
 * util.c
 *
 * Created: 3/26/2023 11:44:02 AM
 *  Author: ashpl
 */ 

#include "util.h"

#include <avr/io.h>
#include <avr/cpufunc.h>

void disable_prescaler() {
	uint8_t value = *((uint8_t*)&CLKCTRL.MCLKCTRLB);
	ccp_write_io((uint8_t*)&CLKCTRL.MCLKCTRLB, value & (~1));
}

void enable_prescaler() {
	ccp_write_io((uint8_t*)&CLKCTRL.MCLKCTRLB, 1);
}

void generic_delay() {
	for (int i = 0; i < 1024; i++) {}
}
