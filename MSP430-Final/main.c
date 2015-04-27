//
//  main.c
//  MSP430-Final
//
//  Created by Joshua Hayes on 26/04/2015.
//  Copyright (c) 2015 Joshua Hayes. All rights reserved.
//*******************************************************//

// Dependencies
#include <stdlib.h>
#include "msp430fr5739.h"
#include "thermister.h"
#include "I2C.h"


void main(void)
{
    setupClocks();
    configureI2CPins();
    updateData();
    
    // Program Loop
    while(1)
    {
        __delay_cycles(1000);                   // Waits For 1000 Master Clock Cycles
        data.new_data = 1;                      // New Data Available
        while (UCB0CTLW0 & UCTXSTP);            // Ack of Stop Clears UCTXSTP, Must Be Checked Before Initialising New Transmission
        UCB0CTLW0 |= UCTR + UCTXSTT; 			// UCTR - Transmit Select; UCTXSTT - Generates Start
        __bis_SR_register(CPUOFF + GIE);        // Enter LPM0 w/ Interrupts
        
        free(data.digit_array);					// updateData Mallocs Dynamic Memory, Freeing Prevents Leaks
        updateData();                           // LPM Ends When All Data Has Been Transmitted, So We Aquire Another Reading To Transmit
    }
}


#pragma vector = USCI_B0_VECTOR						// Declares Interrupt On USCI_B0, For NACKs & Transmit Buffer
__interrupt void USCIB0_ISR(void)
{
    switch(__even_in_range(UCB0IV,0x1E))			// __even_in_range(x,y) Tells Compliler x Must Be Even In Range 0-y;
    {												// UCB0IV - USCIB0 Interrupt Vector Register; 0x1E - 30
        case 0x00: break;							// No interrupt Pending;
        case 0x02: break;							// Arbitration Lost
        case 0x04:									// NACK
            UCB0CTLW0 |= UCTXSTT;					// Resend Start Condition
            break;
        case 0x18:                                                      // Transmit Buffer Empty
            (data.new_data) ? UCB0TXBUF = multiByte() :	clearStop();    // Check For New Byte
            break;                                                      // UCB0TXBUF - 8-Bit (1-Byte) Buffer For Writing;
        default: break;                                                 // 2nd Instance - No New Data, Buffer Empty, Clear Screen
    }
}

