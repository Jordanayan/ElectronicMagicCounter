/*
 * serial.h
 *
 * Created: 3/25/2023 4:33:53 PM
 *  Author: ashpl
 */ 


#ifndef SERIAL_H_
#define SERIAL_H_

#include <stdint.h>

#define SERIAL_BAUD_RATE 115200

void init_serial();
void serial_write_byte(int8_t data);
void serial_write_bytes(int8_t* pointer, uint16_t length);
void serial_write_text(const char* text);

#endif /* SERIAL_H_ */