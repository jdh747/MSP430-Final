//
//  main.c
//  MSP430-Final
//
//  Created by Joshua Hayes on 26/04/2015.
//  Copyright (c) 2015 Joshua Hayes. All rights reserved.
//*******************************************************//

#include "msp430fr5739.h"

unsigned char TXData = 0;                    // Pointer to TX data
unsigned char TXByteCtr;

void main(void)
{
    // Configure Clocks & Timers
    WDTCTL = WDTPW + WDTHOLD;				// Watchdog Timer (Resets Board @ Timer Limit): WDTPW - Password; WDTHOLD - Stops Timer
    // Init SMCLK = MCLk = ACLK = 1MHz
    CSCTL0_H = 0xA5;						// Unlock Clock Register, CS Control Register: CSKEY = 0xA5 - CS Password
    CSCTL1 |= DCOFSEL0 + DCOFSEL1;          // Set max. DCO setting = 8MHz		// Bit 0 + 1 = 11b = 8
    CSCTL2 = SELA_3 + SELS_3 + SELM_3;      // set ACLK = MCLK = DCO			// SELA_3 - ACLK (Aux Clock) Source Select 3 (DCO); SELS_3 - SMCLK (SubSys Clock) SS3 (DCO); SELM_3 - MCLK (Master Clock) SS3(DCO)
    CSCTL3 = DIVA_3 + DIVS_3 + DIVM_3;      // set all dividers to 1MHz			// Divide All 3 Clocks By 8
    
    // Configure Pins for I2C
    P1SEL1 |= BIT6 + BIT7;						// Pin init		// BIT6: 1.6 UCB0SDA; BIT7: 1.7 UCB0SCL
    
    UCB0CTLW0 |= UCSWRST;						// put eUSCI_B in reset state			// Stops Interrupts
    UCB0CTLW0 |= UCMODE_3 + UCMST + UCSSEL_2;	// I2C Master Mode, SMCLk				// UCMODE_3 - I2C Mode; UCMST - Master Mode; UCSSEL_2 - Clock Source SMCLK
    UCB0BRW = 0x8;								// baudrate = SMCLK / 8					// Serial Runs For Every 8 SubSys Clk Cycles
    UCB0I2CSA = 0x48;							// address slave is 48hex
    UCB0CTLW0 &=~ UCSWRST;						// clear reset register					// Enable Interrupts
    UCB0IE |= UCTXIE0 + UCNACKIE;				// transmit and NACK interrupt enable	// Enable eUSCI Interrupt: UCTXIE0 - Trans Interrupt; UCNACKIE - NACK (Negative Ack) Interrupt
    
    while(1)
    {
        __delay_cycles(1000);                   // Delay between transmissions		// Waits For 1000 Master Clock Cycles
        TXByteCtr = 1;                          // Load TX byte counter
        while (UCB0CTLW0 & UCTXSTP);            // Ack of Stop Clears UCTXSTP, Must Be Checked Before Initialising New Transmission
        UCB0CTLW0 |= UCTR + UCTXSTT; 			// UCTR - Transmit Select; UCTXSTT - Generates Start
        __bis_SR_register(CPUOFF + GIE);        // Enter LPM0 w/ interrupts
        // Remain in LPM0 until all data
        // is TX'd
        TXData++;								// Increment data byte
    }
}




#pragma vector = USCI_B0_VECTOR						// Declares Interrupt On USCI_B0, For ACKs?????
__interrupt void USCIB0_ISR(void)					// Should This Be USCI_B0_ISR?????
{
    switch(__even_in_range(UCB0IV,0x1E))			// __even_in_range(x,y) Tells Compliler x Must Be Even In Range 0-y;
    {												// UCB0IV - USCIB0 Interrupt Vector Register; 0x1E - 30
            
        case 0x00: break;							// No interrupt Pending;
        case 0x02: break;							// Arbitration Lost
        case 0x04:									// NACK
            UCB0CTLW0 |= UCTXSTT;					// resend start if NACK
            break;
        case 0x18:									// Transmit Buffer Empty
            if (TXByteCtr)							// Check TX byte counter
            {
                // On First Insance of New Data
                UCB0TXBUF = TXData;					// UCB0TXBUF - 8-Bit (1-Byte) Buffer For Writing;
                TXByteCtr--;						// Decrement TX byte counter
            }
            else
            {
                // 2nd Instance - No New Data, Buffer Empty Therefore Previous Data Written
                UCB0CTLW0 |= UCTXSTP;				// I2C stop condition
                UCB0IFG &= ~UCTXIFG;				// Clear TX Buffer Empty Interrupt Flag, Automatic When Writing Data
                __bic_SR_register_on_exit(CPUOFF);	// Exit LPM0		//..._on_exit(x) clears x
            }
            break;
        default: break;
    }
}
