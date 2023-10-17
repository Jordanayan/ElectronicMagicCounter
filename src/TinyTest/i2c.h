#ifndef I2C_H_
#define I2C_H_

#include <avr/io.h>

#define F_CPU 20000000UL
#define I2C_FREQ 400000UL

void i2c_init(void);
void i2c_start(uint8_t address);
void i2c_stop(void);
void i2c_write(uint8_t data);

#endif