#ifndef LCD_H
#define LCD_H

#include <stdint.h>

// go through the init routine for the lcd, this takes about 50ms
// will set P1.4 - P1.7, P2.0, P2.6, and P2.7 as outputs
void lcd_init(void);

// print the entire lcd_buffer to the lcd
// lcd_buffer is global so this function takes no parameters
void lcd_print_buffer(void);

// create a custom character at the specified index using lcd_buffer_custom
// the index may be a value within [0, 7]
// when printing these value the same index can be used
//  i.e., "\x00" would print custom character 0
//   "\x01" would print custom character 1
void lcd_create_character(uint8_t index);

// clear the lcd's display
void lcd_clear_display(void);

// set the cursor to the first display position on the lcd
void lcd_return_home(void);

// toggle the lcd's cursor between on or off
void lcd_toggle_cursor(void);

// toggle the lcd's cursor blink between on or off
void lcd_toggle_blink(void);

// toggles the current value for e, which is stored in P2.0
void lcd_clock_e(void);

// set the RS and R/W bits accordingly
void lcd_set_mode(uint8_t rs, uint8_t rw);

// send the input data to the lcd in 4-bit mode
// the RS and R/W bits should be appropriately set before calling
void lcd_cmd_send(uint8_t byte);

// send a read instruction to the lcd and obtain busy flag's value
// the value of busy flag will be updated
void lcd_cmd_status(void);

// send an lcd command instruction (command with RS and R/W cleared)
void lcd_cmd_inst(uint8_t byte);

// send an lcd write instruction (command with RS set and R/W cleared)
void lcd_cmd_write(uint8_t byte);

// wait until the busy flag no longer reads as busy
void lcd_poll_busy_flag(void);

#endif
