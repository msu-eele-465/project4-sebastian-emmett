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
    lcd_buffer[0] = 'a';
    lcd_buffer[1] = 'b';

    lcd_print_buffer();

    while (1);
}
