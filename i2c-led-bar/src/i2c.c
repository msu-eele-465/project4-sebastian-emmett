#include <msp430.h>
#include "../src/i2c.h"
#include "intrinsics.h"
#include "msp430fr2355.h"

// Helpful Brock video :D https://www.youtube.com/watch?v=BvITEarUMkc

volatile char i2c_received_data;
volatile bool i2c_data_ready = false;

volatile char tx_data;              // Data to send
volatile bool i2c_tx_complete = true; // Transmission complete flag (true when idle)

void i2c_master_init(void)
{
    // Configure pins P1.2 (SDA) and P1.3 (SCL)
    P1SEL1 &= ~(BIT2 | BIT3); // P1SEL1 = 0
    P1SEL0 |= BIT2 | BIT3;    // P1SEL0 = 1 for I2C function

    // Configure USCI_B0 for I2C master mode
    UCB0CTLW0 |= UCSWRST;     // Put module in reset
    UCB0CTLW0 |= UCSSEL_3;    // Use SMCLK (ensure SMCLK is ~1MHz for UCB0BRW=10)
    UCB0BRW = 10;             // Prescaler for ~100kHz I2C clock
    UCB0CTLW0 |= UCMODE_3;    // I2C mode
    UCB0CTLW0 |= UCMST;       // Master mode
    UCB0CTLW0 |= UCTR;        // Transmitter mode

    UCB0CTLW1 |= UCASTP_2;    // Auto STOP after UCB0TBCNT bytes
    UCB0TBCNT = 1;            // Send 1 byte per transaction

    UCB0CTLW0 &= ~UCSWRST;    // Clear reset to enable module

    // Enable interrupts for TX, NACK, and STOP
    UCB0IE |= UCTXIE0 | UCNACKIE | UCSTPIE;
    __enable_interrupt();     // Enable global interrupts
}

void i2c_slave_init(uint8_t address)
{
    // Configure pins P1.2 (SDA) and P1.3 (SCL)
    P1SEL0 |= BIT2 | BIT3;    // Select I2C function for P1.2 and P1.3
    P1SEL1 &= ~(BIT2 | BIT3); // Ensure P1SEL1 bits are 0

    // Configure USCI_B0 for I2C slave mode
    UCB0CTLW0 |= UCSWRST;     // Put module in reset
    UCB0CTLW0 |= UCMODE_3 | UCSYNC; // I2C mode, slave, synchronous
    UCB0I2COA0 = address | UCOAEN;  // Set own address and enable
    UCB0CTLW0 &= ~UCSWRST;    // Clear reset to enable module

    // Enable receive interrupt
    UCB0IE |= UCRXIE;
}

void i2c_send(uint8_t slave_address, char data)
{
    while (!i2c_tx_complete);  // Wait for previous transmission to complete
    i2c_tx_complete = false;   // Mark as in progress
    tx_data = data;            // Store data for ISR
    UCB0I2CSA = slave_address; // Set slave address
    UCB0CTLW0 |= UCTXSTT;      // Generate START
}

bool i2c_get_received_data(char* data)
{
    if (i2c_data_ready)
    {
        *data = i2c_received_data;
        i2c_data_ready = false;
        return true;
    }
    return false;
}

#pragma vector=USCI_B0_VECTOR
__interrupt void USCI_B0_ISR(void)
{
    if (UCB0IFG & UCRXIFG)
    {
        i2c_received_data = UCB0RXBUF;
        i2c_data_ready = true;
    }

    switch (UCB0IV)  // Automatically clears the handled flag
    {
        case USCI_I2C_UCNACKIFG:  // Slave sent NACK
            UCB0CTLW0 |= UCTXSTP;  // Generate STOP
            i2c_tx_complete = true; // Transaction aborted
            break;

        case USCI_I2C_UCTXIFG0:   // TX buffer empty (address sent, ready for data)
            UCB0TXBUF = tx_data;   // Send the data (clears UCTXIFG0)
            break;

        case USCI_I2C_UCSTPIFG:   // STOP condition generated
            i2c_tx_complete = true; // Transaction complete
            break;

        default:
            break;  // Ignore other interrupts for now
    }
}

void i2c_send_to_both(char data)
{
    i2c_send(SLAVE1_ADDR, data);       // Send to first slave
    //i2c_send(SLAVE2_ADDR, data);       // Send to second slave
}