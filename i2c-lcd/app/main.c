#include <msp430fr2310.h>
#include <stdbool.h>

#include "../src/lcd.h"


/* --- global variables --- */


// used to indentify if a new key has been pressed, and thus
// 	that the appropriate logic should take place
extern bool new_key;


/* --- program --- */


// this contains the logic to update the transition_period
// 	string according to the base_transition_period
void _update_transition_period(char *transition_period, uint8_t base_transition_period){

	// soft cap of base_transition_period at 9.75, since it
	// 	seems unreasonable to display anything more than that
	if(base_transition_period > 39) base_transition_period = 39;

	// first turn the integer part into a string
	transition_period[7] = '0' + (base_transition_period >> 2);

	// because the base_transition_period moves in increments of
	//  0.25 it is possible to switch on the lower 4 bits of it
	//  and insert the correct string with no further calculation
	switch(base_transition_period & 0x03){

		case 0x00:
			transition_period[9] = '0';
			transition_period[10] = '0';
			break;

		case 0x01:
			transition_period[9] = '2';
			transition_period[10] = '5';
			break;

		case 0x02:
			transition_period[9] = '5';
			transition_period[10] = '0';
			break;

		case 0x03:
			transition_period[9] = '7';
			transition_period[10] = '5';
			break;
	}
}

int main(void)
{
    // Stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

	lcd_init();

	const char *pattern_0 = "static          ";
	const char *pattern_1 = "toggle          ";
	const char *pattern_2 = "up counter      ";
	const char *pattern_3 = "in and out      ";
	const char *pattern_4 = "down counter    ";
	const char *pattern_5 = "rotate 1 left   ";
	const char *pattern_6 = "rotate 7 left   ";
	const char *pattern_7 = "fill left       ";

	char transition_period[] = "period=0.00     ";

	// each integer value will represent a change of 0.25 in the
	//  actual transition_period, due to this the current value
	//  is interpreted as 1
	// this interpretation makes string conversion much faster
	uint8_t base_transition_period = 4;	

	// the state of the system, whether is is locked or unlocked
	uint8_t locked = 1;

    // Disable low-power mode / GPIO high-impedance
    PM5CTL0 &= ~LOCKLPM5;

    while (1)
    {
		// TODO: i2c_init_as_slave

		if(new_key){
			switch(curr_key){
				case 'D':
					locked = 1;
					lcd_clear_display();
					lcd_set_ddram_addr(0x20);	// this is so the cursor does not show up

					break;

				case 'U':
					locked = 0;

					break;

				case 'A':
					base_transition_period++;
					_update_transition_period(transition_period, base_transition_period);

					lcd_print_line(transition_period, 1);

					break;

				case 'B':
					base_transition_period--;
					_update_transition_period(transition_period, base_transition_period);

					lcd_print_line(transition_period, 1);

					break;

				case 'C':
					lcd_toggle_cursor();

					break;

				case '0':
					lcd_print_line(pattern_0, 0);

					break;

				case '1':
					lcd_print_line(pattern_1, 0);

					break;

				case '2':
					lcd_print_line(pattern_2, 0);

					break;

				case '3':
					lcd_print_line(pattern_3, 0);

					break;

				case '4':
					lcd_print_line(pattern_4, 0);

					break;

				case '5':
					lcd_print_line(pattern_5, 0);

					break;

				case '6':
					lcd_print_line(pattern_6, 0);

					break;

				case '7':
					lcd_print_line(pattern_7, 0);

					break;

				case '9':
					lcd_toggle_blink();

					break;

				default:
					break;
			}

			lcd_update_current_key();
			new_key = false;
		}
		else{
			if(locked) lcd_clear_display();
		}
    }

	return 0;
}
