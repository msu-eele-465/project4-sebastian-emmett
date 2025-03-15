#include <msp430fr2310.h>
#include <stdint.h>

#include "./lcd.h"
#include "./lcd_defines.h"


/* --- init --- */


// period= will always be present, so the bufer is initialized to contain it
char lcd_buffer[LCD_BUFFER_SIZE] = "                period=         ";

// the bitmap is initialized to 0 in all spots, so that if the system is switched
//  to 5x10, but 5x8 custom characters are still used, garbage will not be present
//  in the last two spots
char lcd_buffer_custom[LCD_BUFFER_CUSTOM_SIZE] = {'\x00'};

// used to control 3 options on the lcd:
//  display on/off, cursor, blink
uint8_t display_ctrl;


void lcd_init(void){
	// set P1.4 - P1.7, P2.0, P2.6, and P2.7 as outputs
    P1SEL0 &= ~(BIT4 | BIT5 | BIT6 | BIT7);
    P1SEL1 &= ~(BIT4 | BIT5 | BIT6 | BIT7);
	P1OUT &= ~(BIT4 | BIT5 | BIT6 | BIT7);
	P1DIR |= BIT4 | BIT5 | BIT6 | BIT7;

    P2SEL0 &= ~(BIT0 | BIT6 | BIT7);
    P2SEL1 &= ~(BIT0 | BIT6 | BIT7);
	P2OUT &= ~(BIT0 | BIT6 | BIT7);
	P2DIR |= BIT0 | BIT6 | BIT7;

	// assuming each cycle is 1 us then 50 ms is 50000 us
	__delay_cycles(50000);

	// incomplete function set, this will tell the lcd to use
	//  4-bit mode, but we will be sending it in '8-bit' mode
	lcd_set_mode(0, 0);

	lcd_clock_e();

	P1OUT &= ~(BIT4 | BIT5 | BIT6 | BIT7);
	P1OUT |= BIT5;	// function set; 4-bit mode

	lcd_clock_e();
	__delay_cycles(100);

	// now another function set must be performed, but this time
	//  it is in 4-bit mode
	lcd_cmd_inst(0x28);	// function set; 4-bit mode, 2-line mode, 5x8 font

	display_ctrl = 0x0C;
	lcd_cmd_inst(display_ctrl);	// display control; display on, cursor off, blink off
	
	lcd_clear_display();

	lcd_cmd_inst(0x06);	// entry mode set; increment, no shift
}


/* --- general use --- */


void lcd_print_buffer(void){
	lcd_return_home();

	char *lcd_buffer_ptr = lcd_buffer;
	char *lcd_buffer_end = lcd_buffer + LCD_BUFFER_SIZE / 2;

	while(lcd_buffer_ptr < lcd_buffer_end)
		lcd_cmd_write((uint8_t) *lcd_buffer_ptr++);

	// this is necessary because the display does not automatically
	//	jump to the correct address
	lcd_set_ddram_addr(0x40);

	lcd_buffer_end = lcd_buffer + LCD_BUFFER_SIZE;

	while(lcd_buffer_ptr < lcd_buffer_end)
		lcd_cmd_write((uint8_t) *lcd_buffer_ptr++);

	// this questionable command is a result of the project
	// 	specs. It is necessary to be able to turn the cursor and/or
	// 	blink on and off, but if they are not visible it cannot
	// 	be proven, so here the cursor must be set to the last position.
	lcd_set_ddram_addr(0x4F);
}

void lcd_update_current_char(uint8_t new_char){
	lcd_buffer[LCD_BUFFER_SIZE - 1] = new_char;

	lcd_set_ddram_addr(0x4F);

	lcd_cmd_write(new_char);

	lcd_set_ddram_addr(0x4F);
}

void lcd_create_character(uint8_t index){
	lcd_set_cgram_addr(index, 0);

	// write the bitmap into the selected spot
	char *lcd_buffer_custom_ptr = lcd_buffer_custom;
	char *lcd_buffer_custom_end = lcd_buffer_custom + LCD_BUFFER_CUSTOM_SIZE;

	while(lcd_buffer_custom_ptr < lcd_buffer_custom_end)
		lcd_cmd_write((uint8_t) *lcd_buffer_custom_ptr++);

	// and reset the cursor to the last display position
	lcd_set_ddram_addr(0x4F);
}

void lcd_clear_display(void){
	lcd_set_mode(0, 0);

	lcd_cmd_send(0x01);

	__delay_cycles(3000);	// 2ms = 2000us
}

void lcd_toggle_cursor(void){
	display_ctrl ^= BIT1;
	lcd_cmd_inst(display_ctrl);
}

void lcd_toggle_blink(void){
	display_ctrl ^= BIT0;
	lcd_cmd_inst(display_ctrl);
}


/* --- advanced use --- */


void lcd_return_home(void){
	lcd_set_mode(0, 0);

	lcd_cmd_send(0x02);

	__delay_cycles(3000);	// 2ms = 2000us
}

void lcd_clock_e(void){
	__delay_cycles(2);
	P2OUT ^= BIT0;	// toggle the enable bit
	__delay_cycles(2);
}

void lcd_set_mode(uint8_t rs, uint8_t rw){
	if(rs) P2OUT |= BIT6;
	else P2OUT &= ~BIT6;

	if(rw) P2OUT |= BIT7;
	else P2OUT &= ~BIT7;

	__delay_cycles(2);
}

void lcd_set_cgram_addr(uint8_t index, uint8_t sub_index){
	lcd_set_mode(0, 0);

	// the index is shifted left three so it will select the
	//	appropriate character, and the sub_index will select
	//	the position within that character
	lcd_cmd_inst(0x40 | (index << 3 & 0x38) | (sub_index & 0x07));
}

void lcd_set_ddram_addr(uint8_t address){
	lcd_set_mode(0, 0);
	lcd_cmd_inst(0x80 | (address & 0x7F));
}

void lcd_cmd_send(uint8_t byte){
	lcd_clock_e();

	// update gpio with upper nibble
	P1OUT &= ~(BIT4 | BIT5 | BIT6 | BIT7);
	P1OUT |= byte & 0xF0;

	lcd_clock_e();

	// after the upper nibble is sent the lower nibble
	//  is then sent since the lcd is in 4 bit mode
	lcd_clock_e();

	// update gpio with lower nibble
	P1OUT &= ~(BIT4 | BIT5 | BIT6 | BIT7);
	P1OUT |= byte << 4;

	lcd_clock_e();
}

void lcd_cmd_inst(uint8_t byte){
	lcd_set_mode(0, 0);

	lcd_cmd_send(byte);

	__delay_cycles(100);
}

void lcd_cmd_write(uint8_t byte){
	lcd_set_mode(1, 0);	// RS set, R/W cleared

	lcd_cmd_send(byte);

	__delay_cycles(100);
}
