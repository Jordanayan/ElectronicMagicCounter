/*
 * ssd1306.h
 *
 * Created: 3/26/2023 11:33:24 AM
 *  Author: ashpl
 */ 


#ifndef SSD1306_H_
#define SSD1306_H_

#include <avr/io.h>

void send_command(uint8_t cmd);
void send_data(uint8_t data);
void write_to_disp(uint8_t v);

void clear_display();
void init_display();
void draw_charmap();

void enable_display_power();
void disable_display_power();

void display_symbol(uint8_t symbol, uint8_t x, uint8_t y);
void display_hex_digit(uint8_t number, uint8_t x, uint8_t y);
void display_hex_byte(uint8_t number, uint8_t x, uint8_t y);
void display_hex_short(uint16_t number, uint8_t x, uint8_t y);
void display_dec_signed(int8_t number, uint8_t x, uint8_t y);
void display_character(char c, uint8_t x, uint8_t y);
void display_text(char* c, uint8_t x, uint8_t y);

#endif /* SSD1306_H_ */