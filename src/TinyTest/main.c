/*
 * TinyTest.c
 *
 * Created: 3/25/2023 2:48:15 PM
 * Author : ashpl
 */ 

#define DEBUG_LOG 0

#include <avr/io.h>
#include <avr/cpufunc.h>


#define SERIAL_BAUD_RATE 115200
#include "i2c.h"
#include "serial.h"
#include "ssd1306.h"
#include "util.h"
#include "adc.h"
#include <util/delay.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

uint8_t charmap[64];

void send_hex(uint16_t val) {
	val = val & 0xf;
	
	if (val < 10) {
		serial_write_byte('0' + val);
	}
	else {
		serial_write_byte('A' + val - 10);
	}
}

void send_hex_reading(uint16_t val) {
	send_hex(val >> 12);
	send_hex(val >> 8);
	send_hex(val >> 4);
	send_hex(val >> 0);
	serial_write_byte('\n');
}

uint8_t within(uint16_t value, uint16_t target) {
	return (value >= target - 3) && (value <= target + 3);
}

uint8_t button_test() {
	
	uint8_t button_data = 0;
	
	if (!(PORTA_IN & 64)) {
		button_data += 8;
	}
	
	if (!(PORTA_IN & 32)) {
		button_data += 4;
	}
	
	if (!(PORTA_IN & 16)) {
		button_data += 2;
	}
	
	if (!(PORTA_IN & 128)) {
		button_data += 1;
	}
	
	return button_data;
	//uint16_t value = run_adc_sample();

	/*if (within(value, 0x2a9)) {
		return 0b001;
	}
	else if (within(value, 0x2db)) {
		return 0b010;
	}
	else if (within(value, 0x357)) {
		return 0b100;
	}
	else if (within(value, 0x344)) {
		return 0b011;
	}
	else if (within(value, 0x380)) {
		return 0b101;
	}
	else if (within(value, 0x39E)) {
		return 0b111;
	}
	else if (within(value, 0x387)) {
		return 0b110;
	}
	else {
		return 0;
	}*/
}

void clear_RTC();
void initialize_RTC();

void go_to_sleep(void) {	//Full power down with the display off, long term sleep while device is not in use
	disable_display_power();
	for (int x = 0; x < 10; x++) {
		generic_delay();	
	}
	sei();
	set_sleep_mode(SLEEP_MODE_PWR_DOWN); //Go *all the way* to sleep. Disable all peripherals, disable the main system clock. Wake ONLY on pin change. Current consumption should be <10uA.
	sleep_enable(); //Set the sleep enable bit, we need to undo this when we wake up.
	sleep_cpu(); //Call the cpu SLEEP instruction. This is the last instruction executed before the main system clock stops ticking
	
	//If we get here, then we woke up from sleep successfully
	sleep_disable();
	generic_delay();
	enable_display_power();
	_delay_ms(500); //Wait a long time for the display to power back up
	init_display();
	clear_RTC();
}

void take_a_nap(void) {	//Full power down with the display off, long term sleep while device is not in use
	sei();
	set_sleep_mode(SLEEP_MODE_STANDBY); //Go *all the way* to sleep. Disable all peripherals, disable the main system clock. Wake ONLY on pin change. Current consumption should be <10uA.
	sleep_enable(); //Set the sleep enable bit, we need to undo this when we wake up.
	sleep_cpu(); //Call the cpu SLEEP instruction. This is the last instruction executed before the main system clock stops ticking
	
	//If we get here, then we woke up from sleep successfully
	sleep_disable();
	clear_RTC();
}

void initialize_RTC() {
	//Real Time Counter Initialization. Do this after our safety pause because the RTC is used by the uC during boot up
	RTC.CLKSEL = 1;		//Use the 1KHz Clock
	RTC.CMPH = 255;		//We paid for the whole timer, we're going to use the whole timer
	RTC.CMPL = 255;		//
	RTC.INTCTRL = 0;	//Never Interrupt
	RTC.CTRLA = 0b10101001;	//Set prescaler, enable the clock, and set the RTC to run in Standby
}

void clear_RTC() {
	RTC.CTRLA = 0;
	while (RTC.STATUS & 2); // Wait for synchronization
	RTC.CNTH = 0;
	RTC.CNTL = 0;
	while (RTC.STATUS & 2); // Wait for synchronization
	initialize_RTC();
}

ISR(PORTA_PORT_vect) {
	//Do nothing since we just use the interrupt to wake from sleep
	sleep_disable();
	PORTA_INTFLAGS = 255;
}

int main(void)
{	
	
	PORTA_DIR |= 0b1110; //Enable I2C Pullups???
	PORTA_OUT |= 0b1110;
	
	//Setup the 4 button pins
	PORTA_PIN4CTRL = 9; //PULLUPEN + Interrupt on Falling Edge
	PORTA_PIN5CTRL = 9; //PULLUPEN + Interrupt on Falling Edge
	PORTA_PIN6CTRL = 8; //PULLUPEN + Interrupt on Falling Edge
	PORTA_PIN7CTRL = 9; //PULLUPEN + Interrupt on Falling Edge
	
	ccp_write_io((uint8_t*)&CLKCTRL.MCLKCTRLB, 0);
	
	_delay_ms(2000);
	
	initialize_RTC();
	
	enable_display_power();

	generic_delay();
	
	init_serial();
	init_adc();

	generic_delay();

	i2c_init();
	init_display();
	generic_delay();

	clear_display();
	for (int i = 0; i < 64; i++) {
		charmap[i] = 0xff;
	}

	int8_t counts[2] = { 0, 0 };
	uint8_t count_index = 2;
	uint8_t last_button = 0;
	uint8_t display_menu = 0;
	uint8_t menu_cursor = 0;
	uint8_t nap_counter = 0;

    while (1) 
    {
		
		if (RTC.CNT > 60000) {
			go_to_sleep();
		}
		
		uint8_t this_button = button_test();
		uint8_t button_update = (last_button ^ this_button) & this_button;
		last_button = this_button;
		
		nap_counter++;
		if (button_update == 0 && nap_counter > 100) {
			take_a_nap();
			nap_counter = 0;
			
		}
		
		if (button_update) {
			nap_counter = 0;
			clear_RTC();
		}

		if (display_menu == 1) { //Main Menu
			if (button_update == 1) {
				menu_cursor += 1;
				if (menu_cursor >= 3) {
					clear_display();
					menu_cursor = 0;
				}	
			}
			
			if (button_update == 4) {
				menu_cursor -= 1;
				if (menu_cursor >= 3) {
					clear_display();
					menu_cursor = 3;
				}
			}
			
			if ((button_update == 8) && (menu_cursor == 1)) {
				counts[0] = 0;
				counts[1] = 0;
			}
			
			if ((button_update == 8) && (menu_cursor == 2)) {
				clear_display();
				display_menu = 0;
			}
			
			if ((button_update == 8) && (menu_cursor == 0)) {
				clear_display();
				display_menu = 2;
			}
		} else if (display_menu == 2) { //Sleep Confirm Menu
			if (button_update == 8) {
				go_to_sleep();
			}
			
			if (button_update == 2 || button_update == 4 || button_update == 1) { //Change to the other page in Sleep confirm
				clear_display();
				display_menu = 3;
			}
			
		} else if (display_menu == 3) { //Exit sleep confirm menu
			if (button_update == 8) { //Go back to main menu
				clear_display();
				display_menu = 1;
			}
			
			if (button_update == 2 || button_update == 4 || button_update == 1) { //Go back to sleep confirm
				clear_display();
				display_menu = 2;
			}
		} else {
			if (button_update == 1 && (count_index == 0)) {
				counts[0]++;
			}
			if (button_update == 1 && (count_index == 1)) {
				counts[1]++;
			}
			if (button_update == 4 && (count_index == 0)) {
				counts[0]--;
			}
			if (button_update == 4 && (count_index == 1)) {
				counts[1]--;
			}
			
			if (button_update == 1 && (count_index == 2)) {
				counts[0]++;
				counts[1]++;
			}
			if (button_update == 4 && (count_index == 2)) {
				counts[0]--;
				counts[1]--;
			}
			if (button_update == 8) {
				display_menu = 1;
			}
			if (button_update == 2) {
				count_index += 1;
				if (count_index >= 3) {
					count_index = 0;
				}
			}	
		}
		
		display_large_dec_signed(counts[0], 1, 0);
		
		if (counts[1] < 10 && counts[1] > -10) {
			for (int i = 8; i < 12; i++) {
				display_character(' ', i, 2);
				display_character(' ', i, 3);
			}
			display_large_dec_signed(counts[1], 10, 2);
		} else {
			display_large_dec_signed(counts[1], 8, 2);
		}
		
		if (count_index == 0) {
			display_character('>', 1, 1);
		}
		
		if (count_index == 1) {
			if (counts[1] < 10 && counts[1] > -10) {
				display_character('>', 10, 3);
			} else {
				display_character('>', 8, 3);
			}
		}
		
		
		if (display_menu == 1) {
			if (menu_cursor == 0) {
				display_character('>', 0, 0);
			} else {
				display_character(' ', 0, 0);
			}
			display_character('O', 1, 0);
			display_character('P', 2, 0);
			display_character('T', 3, 0);
			display_character('N', 4, 0);
			display_character('S', 5, 0);
			
			if (menu_cursor == 1) {
				display_character('>', 6, 0);
				} else {
				display_character(' ', 6, 0);
			}
			display_character('C', 7, 0);
			display_character('L', 8, 0);
			display_character('R', 9, 0);
			
			if (menu_cursor == 2) {
				display_character('>', 10, 0);
				} else {
				display_character(' ', 10, 0);
			}
			display_character('E', 11, 0);
			display_character('X', 12, 0);
			display_character('I', 13, 0);
			display_character('T', 14, 0);
		} else if (display_menu == 2) {
			display_character('S', 1, 0);
			display_character('L', 2, 0);
			display_character('E', 3, 0);
			display_character('E', 4, 0);
			display_character('P', 5, 0);
		} else if (display_menu == 3) {
			display_character('D', 1, 0);
			display_character('O', 2, 0);
			display_character('N', 3, 0);
			display_character('E', 4, 0);
		} else if (display_menu == 4) {
			
		}
		
		//display_dec_unsigned(RTC.CNTH, 0, 3);
		//display_dec_unsigned(button_update, 5, 3);
		//display_dec_unsigned(((last_button_press + 60000) >> 8) & 0xff, 0, 2);
		//display_dec_unsigned((last_button_press + 60000) & 0xff, 5, 2);
		

		draw_charmap();
    }
}

