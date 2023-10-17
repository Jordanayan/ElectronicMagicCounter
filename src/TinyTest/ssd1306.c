/*
 * ssd1306.c
 *
 * Created: 3/26/2023 11:33:36 AM
 *  Author: ashpl
 */ 

#include <stdint.h>

#include "ssd1306.h"
#include "util.h"
#include "characters.h"

#include "i2c.h"

extern uint8_t charmap[64];

void send_command(uint8_t cmd) {
	i2c_start(0x3C); // SSD1306 I2C Address + Write Bit
	i2c_write(0x00); // Control Byte - Command
	i2c_write(cmd); // Send Command
	i2c_stop(); // Stop Condition
}

void send_data(uint8_t data) {
	i2c_start(0x3C); // SSD1306 I2C Address + Write Bit
	i2c_write(data); // Send Command
	i2c_stop(); // Stop Condition
}

void write_to_disp(uint8_t v) {
	i2c_start(0x3C); // SSD1306 I2C Address + Write Bit
	i2c_write(0x40); // Control Byte - Command
	i2c_write(v); // Send Command
	i2c_stop(); // Stop Condition
}

void clear_display() {
	for (int x = 0; x < 16; x++) {
		for (int y = 0; y < 4; y++) {
			display_character(' ', x, y);
		}
	}
	
	draw_charmap();
}

void init_display() {
	TWI0_MSTATUS = 1;
	uint8_t cmd[27] = { 0xAE,
		0xD5,
		0x80,
		0xA8,
		0x3F,
		0xD3,
		0x00,
		0x40,
		0x8D,
		0x14,
		0x20,
		0x00,
		0xA1,
		0xC8,
		0xDA,
		0x12,
		0x81,
		0xCF,
		0xD9,
		0xF1,
		0xDB,
		0x40,
		0xA4,
		0xA6,
		0xAF, };
	
	for (int index = 0; index < 25; index++) {
		send_command(cmd[index]);
		generic_delay();
	}
}

void draw_charmap() {
	send_command(0x40);
	
	for (int y = 0; y < 8; y++) {
		for (int x = 0; x < 128; x++) {
			char c = charmap[x / 8 + 16 * (y / 2)];
			int x_off = x % 8;
			int y_off = y % 2;
			
			if (c >= 0 && c <= 39) {
				uint8_t v = (character_data[c][x_off] >> (8 * (1 - y_off))) & 0xff;
				uint8_t other = 0;
				
				for (int i = 0; i < 8; i++) {
					other |= ((v & (1 << i)) >> i) << (7 - i);
				}
				write_to_disp(other);
			}
			else if (c > 127 && c < 200) {
				x_off = ((x >> 1) % 4) + (4>>(3*(1-((c>>1)%2))));
				y_off = c % 2;
				
				uint8_t v = (character_data[(c>>2)-32][x_off] >> (8 * (1 - y_off))) & 0xff;
				if (y % 2 == 0) {
					v = v >> 4;
				}
				
				uint8_t v_ = (v & 0b0001)*3;
				v_ += ((v & 0b0010)>>1) * 12;
				v_ += ((v & 0b0100)>>2) * 48;
				v_ += ((v & 0b1000)>>3) * 192;
				v = v_;
				
				uint8_t other = 0;
				
				for (int i = 0; i < 8; i++) {
					other |= ((v & (1 << i)) >> i) << (7 - i);
				}
				write_to_disp(other);
			}
			else {
				write_to_disp(0x00);
			}
		}
	}
}


void enable_display_power() {
	PORTA_DIR |= 0b1110;
	PORTA_OUT |= 0b1110;
}

void disable_display_power() {
	
	PORTA_OUT = 0;
	PORTA_DIR = 0;
}


void display_symbol(uint8_t symbol, uint8_t x, uint8_t y) {
charmap[(x + 16 * y) % 64] = symbol % 16;
}

void display_hex_digit(uint8_t number, uint8_t x, uint8_t y) {
	charmap[(x + 16 * y) % 64] = number % 16;
}

char convert_number_to_char(uint8_t number) {
	if (number >= 0 && number < 10) {
		return '0' + number;
	} else {
		return '>';
	}
}

void display_hex_byte(uint8_t number, uint8_t x, uint8_t y) {
	display_hex_digit(number >> 4, x, y);
	display_hex_digit(number, x + 1, y);
}

void display_hex_short(uint16_t number, uint8_t x, uint8_t y) {
	display_hex_digit(number >> 12, x, y);
	display_hex_digit(number >> 8, x + 1, y);
	display_hex_digit(number >> 4, x + 2, y);
	display_hex_digit(number, x + 3, y);
}

void display_large_character(char c, uint8_t x, uint8_t y) {
	if (c >= '0' && c <= '9') {
		display_character(((c-'0')<<2)+128, x, y);
		display_character(((c-'0')<<2)+129, x, y+1);
		display_character(((c-'0')<<2)+130, x+1, y);
		display_character(((c-'0')<<2)+131, x+1, y+1);
	}
}

void display_character(char c, uint8_t x, uint8_t y) {
	if (c >= '0' && c <= '9') {
		c -= '0';
	}
	else if (c >= 'A' && c <= 'Z') {
		c = c - 'A' + 10;
	}
	else if (c >= 'a' && c <= 'z') {
		c = c - 'a' + 10;
	}
	else if (c == '<') {
		c = 36;
	}
	else if (c == '>') {
		c = 38;
	}
	else if (c == '-') {
		c = 37;
	}
	else if (c == '+') {
		c = 39;
	}
	else if (c > 127) {
		c = c;
	}
	else {
		c = 0xff;
	}
	
	charmap[(x + 16 * y) % 64] = c;
}

void display_dec_signed(int8_t number, uint8_t x, uint8_t y) {
	if (number < 0) {
		display_character('-', x, y);
		number = -number;
	}
	else {
		display_character('+', x, y);
	}
	
	if (number < 10) {
		display_character(' ', x + 2, y);
		display_hex_digit(number, x + 1, y);
	}
	else {
		display_hex_digit((number / 10) % 10, x + 1, y);
		display_hex_digit(number % 10, x + 2, y);
	}
}

void display_dec_unsigned(uint8_t number, uint8_t x, uint8_t y) {
	if (number < 10) {
		display_character(' ', x + 2, y);
		display_hex_digit(number, x + 1, y);
	}
	else {
		display_hex_digit((number / 10) % 10, x + 1, y);
		display_hex_digit(number % 10, x + 2, y);
	}
}

void display_large_dec_signed(int8_t number, uint8_t x, uint8_t y) {
	if (number < 0) {
		display_character('-', x, y);
		display_character(' ', x, y+1);
		number = -number;
	}
	else {
		display_character('+', x, y);
		display_character(' ', x, y+1);
	}
	
	if (number < 10) {
		display_large_character(convert_number_to_char(number), x + 2, y);
		display_character(' ', x + 4, y);
		display_character(' ', x + 5, y);
		display_character(' ', x + 4, y+1);
		display_character(' ', x + 5, y+1);
	}
	else {
		display_large_character(convert_number_to_char((number / 10) % 10), x + 2, y);
		display_large_character(convert_number_to_char(number % 10), x + 4, y);
	}
}

void display_text(char* c, uint8_t x, uint8_t y) {
	while (*c) {
		display_character(*c++, x++, y);
	}
}