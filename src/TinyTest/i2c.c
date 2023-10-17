#include "i2c.h"

void i2c_init(void) {
	TWI0.MCTRLB = TWI_SMEN_bm; // Enable SMBus mode
	TWI0.MBAUD = 10; //((F_CPU/I2C_FREQ)-10)/2; // Set baud rate
	TWI0.MCTRLA = TWI_ENABLE_bm; // Enable TWI0
}

void i2c_start(uint8_t address) {
	TWI0.MADDR = (address<<1)|0; // Send write command to address
	while(!(TWI0.MSTATUS&TWI_WIF_bm)); // Wait for write interrupt flag
	// MADDR is updated with the slave address shifted left 1 bit and ORed with 0 to indicate a write operation.
	// The while loop waits for the write interrupt flag (TWI_WIF_bm) to be set before proceeding.
}

void i2c_stop(void) {
	TWI0.MCTRLB |= TWI_MCMD_STOP_gc; // Send stop condition
	// MCTRLB is updated with the stop command (TWI_MCMD_STOP_gc) to end the I2C communication.
}

void i2c_write(uint8_t data) {
	TWI0.MDATA = data; // Send data
	while(!(TWI0.MSTATUS&TWI_WIF_bm)); // Wait for write interrupt flag
	// MDATA is updated with the data to be sent to the slave device.
	// The while loop waits for the write interrupt flag (TWI_WIF_bm) to be set before proceeding.
}

uint8_t i2c_read_ack(void) {
	TWI0.MCTRLB &= ~TWI_ACKACT_bm; // Acknowledge byte
	TWI0.MCTRLB |= TWI_MCMD_RECVTRANS_gc; // Receive data
	while(!(TWI0.MSTATUS&TWI_RIF_bm)); // Wait for read interrupt flag
	return TWI0.MDATA; // Return data
	// MCTRLB is updated to acknowledge the byte received (TWI_ACKACT_bm) and to initiate a receive data transaction (TWI_MCMD_RECVTRANS_gc).
	// The while loop waits for the read interrupt flag (TWI_RIF_bm) to be set before proceeding.
	// The function returns the data received from the slave device.
}

uint8_t i2c_read_nack(void) {
	TWI0.MCTRLB |= TWI_ACKACT_bm; // Don't acknowledge byte
	TWI0.MCTRLB |= TWI_MCMD_RECVTRANS_gc; // Receive data
	while(!(TWI0.MSTATUS&TWI_RIF_bm)); // Wait for read interrupt flag
	return TWI0.MDATA; // Return data
	// MCTRLB is updated to not acknowledge the byte received (TWI_ACKACT_bm) and to initiate a receive data transaction (TWI_MCMD_RECVTRANS_gc).
	// The while loop waits for the read interrupt flag (TWI_RIF_bm) to be set before proceeding.
	// The function returns the data received from the slave device.
}