#include <msp430.h>
#include "../src/i2c.h"

volatile char i2c_received_data;
volatile bool i2c_data_ready = false;

void i2c_master_init(void)
{
    // Configure pins P1.2 (SDA) and P1.3 (SCL)
    P1SEL0 |= BIT2 | BIT3;    // Select I2C function for P1.2 and P1.3
    P1SEL1 &= ~(BIT2 | BIT3); // Ensure P1SEL1 bits are 0

    // Configure USCI_B0 for I2C master mode
    UCB0CTLW0 |= UCSWRST;     // Put module in reset
    UCB0CTLW0 |= UCMODE_3 | UCMST | UCSYNC; // I2C mode, master, synchronous
    UCB0CTLW1 |= UCSSEL_2;    // Use SMCLK as clock source
    UCB0BRW = 10;             // Set baud rate to 100kHz (SMCLK 1MHz / 10)
    UCB0CTLW0 &= ~UCSWRST;    // Clear reset to enable module
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
    UCB0I2CSA = slave_address;         // Set slave address
    UCB0CTLW0 |= UCTR | UCTXSTT;      // Transmit mode, generate START
    while (!(UCB0IFG & UCTXIFG));      // Wait for TX buffer to be ready
    UCB0TXBUF = data;                  // Write data to TX buffer
    UCB0CTLW0 |= UCTXSTP;             // Generate STOP condition
    while (UCB0CTLW0 & UCTXSTP);       // Wait for STOP to complete
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
}

void i2c_send_to_both(char data)
{
    i2c_send(SLAVE1_ADDR, data);       // Send to first slave
    i2c_send(SLAVE2_ADDR, data);       // Send to second slave
}