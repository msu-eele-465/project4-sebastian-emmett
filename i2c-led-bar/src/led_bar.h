#ifndef LED_BAR_H
#define LED_BAR_H

void led_bar_init(void);
void led_bar_update_pattern(void);
void led_bar_delay(void);

// Macro to update LED bar with pin mapping for P1.0, P1.4, P1.5, P1.6, P1.7, P1.1, P2.6, P2.7
#define led_bar_update(update_value) \
    do { \
        P1OUT = (P1OUT & ~(BIT0 | BIT1 | BIT4 | BIT5 | BIT6 | BIT7)) | \
                (((update_value) & BIT0) << 0) | /* Bit 0 -> P1.0 */ \
                (((update_value) & BIT1) << 3) | /* Bit 1 -> P1.4 */ \
                (((update_value) & BIT2) << 3) | /* Bit 2 -> P1.5 */ \
                (((update_value) & BIT3) << 3) | /* Bit 3 -> P1.6 */ \
                (((update_value) & BIT4) << 3) | /* Bit 4 -> P1.7 */ \
                (((update_value) & BIT5) >> 4);  /* Bit 5 -> P1.1 */ \
        P2OUT = (P2OUT & ~(BIT6 | BIT7)) | \
                (((update_value) & BIT6) << 0) | /* Bit 6 -> P2.6 */ \
                (((update_value) & BIT7) << 0);  /* Bit 7 -> P2.7 */ \
    } while (0)

#endif
