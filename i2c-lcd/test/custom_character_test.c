#include <msp430fr2310.h>

#include "../src/lcd.h"

int main(void)
{
    // Stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

	lcd_init();

    // Disable low-power mode / GPIO high-impedance
    PM5CTL0 &= ~LOCKLPM5;

    lcd_clear_display();

    lcd_toggle_cursor();
    lcd_toggle_blink();

	lcd_buffer_custom[0] = '\x04';
	lcd_buffer_custom[1] = '\x0E';
	lcd_buffer_custom[2] = '\x0A';
	lcd_buffer_custom[3] = '\x0A';
	lcd_buffer_custom[4] = '\x0A';
	lcd_buffer_custom[5] = '\x11';
	lcd_buffer_custom[6] = '\x15';
	lcd_buffer_custom[7] = '\x0A';

	lcd_create_character(0);

    lcd_buffer[0] = '\x00';

    lcd_print_buffer();

    lcd_update_current_char('@');

    while (1);
}
