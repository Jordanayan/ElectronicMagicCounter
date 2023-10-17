/*
 * serial.c
 *
 * Created: 3/25/2023 4:34:06 PM
 *  Author: ashpl
 */ 

#include "serial.h"

#include <avr/io.h>

// Initialize the serial interface
void init_serial() {
	// Set PA6 to an OUTPUT and HIGH
	PORTA_DIRSET |= 0b1000000;
	PORTA_OUT |= 0b1000000;
	
	// Set the BAUD rate
	USART0_BAUD = 4 * 20000000 / (SERIAL_BAUD_RATE);
	
	// Set the mode of operation
	USART0_CTRLB = 0b10000000;
	USART0_CTRLC = 0b00000011;
}

void serial_write_byte(int8_t data) {
	while (!(USART0_STATUS & (1 << 5))){}
	USART0_TXDATAL = data;
}

void serial_write_bytes(int8_t* pointer, uint16_t length) {
	for (; length; length--) {
		serial_write_byte(*pointer++);
	}
}

void serial_write_text(const char* text) {
	while (*text) {
		serial_write_byte(*text++);
	}
}