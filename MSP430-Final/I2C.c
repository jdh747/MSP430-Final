//
//  I2C.c
//  MSP430-Final
//
//  Created by Joshua Hayes on 26/04/2015.
//  Copyright (c) 2015 Joshua Hayes. All rights reserved.
//

#include "I2C.h"


void setupClocks()
{
    // Configure Clocks & Timers
    WDTCTL = WDTPW + WDTHOLD;				// Kill Watchdog Timer (Resets Board @ Timer Limit): WDTPW - Password; WDTHOLD - Stops Timer
    CSCTL0_H = 0xA5;						// Unlock Clock Register: CSKEY = 0xA5
    CSCTL1 |= DCOFSEL0 + DCOFSEL1;          // Set DCO = 8MHz -- Bit 0 + 1 = 11b = 8MHz
    CSCTL2 = SELA_3 + SELS_3 + SELM_3;      // Set Clocks Source: SELA/S/M_3 - ACLK(Aux Clock)/SMCLK(SubSys Clock)/MCLK(Master Clock) = DCO
    CSCTL3 = DIVA_3 + DIVS_3 + DIVM_3;      // Divide All 3 Clocks By 8 ie 1MHz
}


void configureI2CPins()
{
    // Configure Pins for I2C
    P1SEL1 |= BIT6 + BIT7;						// Pin Function Select: BIT6: 1.6 UCB0SDA; BIT7: 1.7 UCB0SCL
    UCB0CTLW0 |= UCSWRST;						// Put eUSCI_B In Reset State -- Stops Interrupts
    UCB0CTLW0 |= UCMODE_3 + UCMST + UCSSEL_2;	// UCB0 Control Register: UCMODE_3 - I2C Mode; UCMST - Master Mode; UCSSEL_2 - Clock Source SMCLK
    UCB0BRW = 0x8;								// Baudrate Clock Source Divider ie SMCLK / 8 -- Serial Runs For Every 8 SubSys Clk Cycles
    UCB0I2CSA = 0x20;							// Slave Address -- 20hex
    UCB0CTLW0 &=~ UCSWRST;						// Clear Reset -- Enable Interrupts
    UCB0IE |= UCTXIE0 + UCNACKIE;				// ReEnable eUSCI Interrupt: UCTXIE0 - Trans Interrupt; UCNACKIE - NACK Interrupt
}


void updateData()
{
    unsigned int templookup[] = {};                             // ***Insert Lookup Table
    unsigned int ADCTemp = templookup[thermisterReading()];
    unsigned char digits = getDigits(ADCTemp);
    data = (unsigned char*)malloc(digits * sizeof(unsigned char));
    data_size = digits;
    while(digits)
    {
        data[digits-1] = (unsigned char)((ADCTemp % 10) + 48);
        ADCTemp /= 10;
        digits--;
    }
}


unsigned char multiByte()
{
    static unsigned char count = 0;
    unsigned char returnvar = data[count];
    
    if (count < data_size-1)
        count++;
    else {
        new_data--;
        count = 0;
    }
    return returnvar;
}


void clearStop()
{
    static unsigned char clearstate = 1;
    unsigned char clearcmds[3] = {0x00, 0x00, 0x00};    // ***Insert LCD Clear Commands: Clear, Cursor, Pos
    
    if (clearstate) {
        UCB0TXBUF = clearcmds[clearstate-1];
        (clearstate >= 3) ? clearstate = 0 : clearstate++;
    } else {
        UCB0CTLW0 |= UCTXSTP;				// I2C Stop Condition
        UCB0IFG &= ~UCTXIFG;				// Clear TX Buffer Empty Interrupt Flag, Automatic When Writing Data
        __bic_SR_register_on_exit(CPUOFF);	// Exit LPM0		//..._on_exit(x) clears x
    }
}


unsigned char getDigits(unsigned int num)
{
    unsigned char digits = 0;
    while (num >= 1)
    {
        num /= 10;
        digits++;
    }
    return digits;
}














