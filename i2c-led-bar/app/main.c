#include <msp430fr2310.h>
#include <stdbool.h>
#include "../src/i2c.h"
#include "../src/led_bar.h"

// Globals for LED bar control
int base_transition_period = 16;  // Initial transition period
float BTP_multiplier = 1;         // Multiplier for delay timing
char curr_num = '0';              // Current pattern number
char prev_num = '0';              // Previous pattern number for reset logic
bool locked = true;               // System lock state
bool num_update = false;          // Flag for new pattern received
bool reset_pattern = false;       // Flag to reset pattern

#define SLAVE_ADDRESS SLAVE1_ADDR  // Use SLAVE1_ADDR from i2c.h

int main(void)
{
    // Stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    // Set P1.0 as output for debug
    P1DIR |= BIT1;
    P1OUT &= ~BIT1;  // Start low

    // Initialize LED bar
    led_bar_init();

    // Disable low-power mode / GPIO high-impedance
    PM5CTL0 &= ~LOCKLPM5;

    // Initialize I2C as slave
    i2c_slave_init(SLAVE_ADDRESS);

    // Note: Interrupt is disabled in i2c_slave_init by removing UCB0IE |= UCRXIE;

    while (true)
    {
        if (UCB0IFG & UCRXIFG)  // Poll the receive interrupt flag
        {
            char received = UCB0RXBUF;  // Read the received data

            // Process received data to update LED bar state
            if (received == 'D')
            {
                locked = true;          // Lock the system
                led_bar_update(0x00);   // Clear LED bar when locked
            }
            else if (received == 'U')
            {
                locked = false;         // Unlock the system
            }
            else if (received >= '0' && received <= '4')
            {
                prev_num = curr_num;    // Store previous pattern
                curr_num = received;    // Set new pattern
                num_update = true;      // Flag new pattern received
                reset_pattern = (prev_num == curr_num);  // Reset if same pattern
            }
            else if (received == 'A' && !locked)
            {
                base_transition_period -= 4;  // Decrease delay
                if (base_transition_period < 4) base_transition_period = 4;
            }
            else if (received == 'B' && !locked)
            {
                base_transition_period += 4;  // Increase delay
            }
        }

        // Update LED bar if not locked
        if (!locked)
        {
            led_bar_update_pattern();
            led_bar_delay();
        }
    }
}