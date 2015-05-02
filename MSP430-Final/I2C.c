//
//  I2C.c
//  MSP430-Final
//
//  Created by Joshua Hayes on 26/04/2015.
//  Copyright (c) 2015 Joshua Hayes. All rights reserved.
//*******************************************************//

#include "I2C.h"


void setupClocks()
{
    // Configure Clocks & Timers
    WDTCTL = WDTPW + WDTHOLD;				// Kill Watchdog Timer (Resets Board @ Timer Limit): WDTPW - Password; WDTHOLD - Stops Timer
    CSCTL0_H = 0xA5;						// Unlock Clock Register: CSKEY = 0xA5
    CSCTL1 |= DCOFSEL0 + DCOFSEL1;          // Set DCO = 8MHz -- Bit 0 + 1 = 11b = 8MHz
    CSCTL2 = SELA_3 + SELS_3 + SELM_3;      // Set Clocks Source: SELA/S/M_3 - ACLK(Aux Clock)/SMCLK(SubSys Clock)/MCLK(Master Clock) = DCO
    CSCTL3 = DIVA_3 + DIVS_3 + DIVM_3;      // Divide All 3 Clocks By 8 ie 1MHz
    CSCTL0_H = 0x01;                        // Lock Clock Register, Any Write Other Than 0xA5 Will Do
}


void configureI2CPins()
{
    // Configure Pins for I2C
    P1SEL1 |= BIT6 + BIT7;						// Pin Function Select: BIT6: 1.6 UCB0SDA; BIT7: 1.7 UCB0SCL
    UCB0CTLW0 |= UCSWRST;						// Put eUSCI_B In Reset State -- Logic Held
    UCB0CTLW0 |= UCMODE_3 + UCMST + UCSSEL_2;	// UCB0 Control Register: UCMODE_3 - I2C Mode; UCMST - Master Mode; UCSSEL_2 - Clock Source SMCLK
    UCB0BRW = 0x8;								// Baudrate Clock Source Divider ie SMCLK / 8 -- Serial Runs For Every 8 SubSys Clk Cycles
    UCB0I2CSA = 0x20;							// Slave Address -- 20hex
    UCB0CTLW0 &=~ UCSWRST;						// Clear Reset -- Released For Operation
}


void updateData()
{
    // Declarations
    int temp;            // Stores Lookup Table Result
    unsigned char digits;   // Digit Count That Can Be Decremented
    
    // Temperature Lookup Table, Conversion From ADC Reading -> Deg Celsius
    int templookup[] = {277,229,188,167,154,144,136,129,124,119,115,111,108,105,102,100,98,95,93,92,90,88,87,85,84,82,81,80,79,78,77,75,74,74,73,72,71,70,69,68,68,67,66,65,65,64,63,63,62,61,61,60,60,59,59,58,57,57,56,56,55,55,55,54,54,53,53,52,52,51,51,51,50,50,49,49,49,48,48,48,47,47,47,46,46,46,45,45,45,44,44,44,43,43,43,42,42,42,42,41,41,41,40,40,40,40,39,39,39,39,38,38,38,38,37,37,37,37,36,36,36,36,36,35,35,35,35,34,34,34,34,34,33,33,33,33,33,32,32,32,32,32,31,31,31,31,31,30,30,30,30,30,30,29,29,29,29,29,28,28,28,28,28,28,27,27,27,27,27,27,27,26,26,26,26,26,26,25,25,25,25,25,25,24,24,24,24,24,24,24,23,23,23,23,23,23,23,22,22,22,22,22,22,22,22,21,21,21,21,21,21,21,20,20,20,20,20,20,20,20,19,19,19,19,19,19,19,19,18,18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,17,16,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,15,14,14,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,12,12,11,11,11,11,11,11,11,11,11,11,11,10,10,10,10,10,10,10,10,10,10,9,9,9,9,9,9,9,9,9,9,9,8,8,8,8,8,8,8,8,8,8,8,7,7,7,7,7,7,7,7,7,7,7,7,6,6,6,6,6,6,6,6,6,6,6,6,5,5,5,5,5,5,5,5,5,5,5,5,4,4,4,4,4,4,4,4,4,4,4,4,3,3,3,3,3,3,3,3,3,3,3,3,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,-3,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-6,-6,-6,-6,-6,-6,-6,-6,-6,-6,-6,-6,-6,-6,-7,-7,-7,-7,-7,-7,-7,-7,-7,-7,-7,-7,-7,-7,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-8,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-9,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-10,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-11,-12,-12,-12,-12,-12,-12,-12,-12,-12,-12,-12,-12,-12,-12,-13,-13,-13,-13,-13,-13,-13,-13,-13,-13,-13,-13,-13,-13,-13,-14,-14,-14,-14,-14,-14,-14,-14,-14,-14,-14,-14,-14,-14,-15,-15,-15,-15,-15,-15,-15,-15,-15,-15,-15,-15,-15,-15,-16,-16,-16,-16,-16,-16,-16,-16,-16,-16,-16,-16,-16,-16,-16,-17,-17,-17,-17,-17,-17,-17,-17,-17,-17,-17,-17,-17,-17,-18,-18,-18,-18,-18,-18,-18,-18,-18,-18,-18,-18,-18,-19,-19,-19,-19,-19,-19,-19,-19,-19,-19,-19,-19,-19,-19,-20,-20,-20,-20,-20,-20,-20,-20,-20,-20,-20,-20,-20,-21,-21,-21,-21,-21,-21,-21,-21,-21,-21,-21,-21,-21,-21,-22,-22,-22,-22,-22,-22,-22,-22,-22,-22,-22,-22,-22,-23,-23,-23,-23,-23,-23,-23,-23,-23,-23,-23,-23,-24,-24,-24,-24,-24,-24,-24,-24,-24,-24,-24,-24,-24,-25,-25,-25,-25,-25,-25,-25,-25,-25,-25,-25,-25,-26,-26,-26,-26,-26,-26,-26,-26,-26,-26,-26,-26,-27,-27,-27,-27,-27,-27,-27,-27,-27,-27,-27,-28,-28,-28,-28,-28,-28,-28,-28,-28,-28,-28,-28,-29,-29,-29,-29,-29,-29,-29,-29,-29,-29,-29,-30,-30,-30,-30,-30,-30,-30,-30,-30,-30,-31,-31,-31,-31,-31,-31,-31,-31,-31,-31,-32,-32,-32,-32,-32,-32,-32,-32,-32,-32,-33,-33,-33,-33,-33,-33,-33,-33,-33,-33,-34,-34,-34,-34,-34,-34,-34,-34,-34,-35,-35,-35,-35,-35,-35,-35,-35,-36,-36,-36,-36,-36,-36,-36,-36,-36,-37,-37,-37,-37,-37,-37,-37,-37,-38,-38,-38,-38,-38,-38,-38,-39,-39,-39,-39,-39,-39,-39,-39,-40,-40,-40,-40,-40,-40,-40,-41,-41,-41,-41,-41,-41,-42,-42,-42,-42,-42,-42,-42,-43,-43,-43,-43,-43,-43,-44,-44,-44,-44,-44,-45,-45,-45,-45,-45,-45,-46,-46,-46,-46,-46,-47,-47,-47,-47,-47,-48,-48,-48,-48,-49,-49,-49,-49,-50,-50,-50,-50,-51,-51,-51,-51,-52,-52,-52,-52,-53,-53,-53,-54,-54,-54,-55,-55,-55,-56,-56,-56,-57,-57,-58,-58,-59,-59,-59,-60,-60,-61,-62,-62,-63,-63,-64,-65,-65,-66,-67,-68,-69,-70,-71,-72,-74,-75,-77,-79,-82,-86,-92};
    
    temp = templookup[thermisterReading()];  // Take Thermister Reading & Store Converted Value
    negativeDetect(&temp);                   // Test For Negative Temp & If True, Amend data.negative & Remove ADCTemp Sign
    digits = getDigits(temp);                // Get Size of Temp -- In Digits
    if (data.negative)                          // data.digit_array Needs To Store Extra (-) Character, If Negative
        digits++;
    data.size = digits;                         // Store Digits In Global Data Struct For Other Functions Use
    data.digit_array = (unsigned char*)malloc(digits * sizeof(unsigned char));  // Dynamically Allocate Storge For Digits & Sign
    
    while(digits)
    {
        if ((digits == 1) && data.negative)     // If Value Is Negative We Store Sign On Final Iteration
        {
            data.digit_array[0] = '-';
        } else
        {
            data.digit_array[digits-1] = (unsigned char)((temp % 10) + 48);  // Attain Last Digit, Convert To ASCII, Typecast & Store
            temp /= 10;                                                      // Remove Last Digit
        }
        digits--;
    }
}


unsigned char multiByte()
{
    static unsigned char count = 0;                     // Keeps Track of Count Between Calls
    unsigned char returnvar = data.digit_array[count];  // Returns The Next Data Element For Transmission
    
    if (count < data.size-1)                            // If There Is Still Data To Come
        count++;
    else                                                // If This Is The Final Piece of Data, Reset Count & Amend new_data Indicator
    {
        data.new_data--;
        count = 0;
    }
    return returnvar;
}


void clearStop()            // ***Never Enters Clear Commands After Initial Run*** //
{
    static unsigned char clearstate = 1;
    unsigned char clearcmds[3] = {LCD_COMMAND, LCD_HITACHI, LCD_HOME};
    
    if (clearstate)                                         // If We're Still Incrementing Through The Clear Commands
    {
        UCB0TXBUF = clearcmds[clearstate-1];                // Next Clear Command To Output Buffer
        (clearstate >= CLEARCMDS_CNT) ? clearstate = 0 : clearstate++;  // Checks For Commands To Come
    } else
    {
        clearstate++;                       // Resets Clearstate For Future Calls
        UCB0CTLW0 |= UCTXSTP;				// I2C Stop Condition
        UCB0IFG &= ~UCTXIFG;				// Clear TX Buffer Empty Interrupt Flag, Automatic When Writing Data
        __bic_SR_register_on_exit(CPUOFF);	// Exit LPM0		//..._on_exit(x) clears x
    }
}


unsigned char getDigits(unsigned int num)
{
    unsigned char digits = 0;
    if (num == 0)       // Zero Would Break Loop
        digits = 1;
    else
    {
        while (num >= 1)
        {
            num /= 10;  // Removes Last Digit
            digits++;   // Increments Count
        }
    }
    return digits;
}


void negativeDetect(int *num)
{
    if (*num < 0)       // If Less Than Zero, Update Data Struct & Remove Sign
    {
        *num *= -1;
        data.negative = 1;
    } else
        data.negative = 0;
}


void DisableI2C()
{
    UCB0IE &= ~(UCTXIE0 + UCNACKIE);  // Disable eUSCI Interrupts
}


void EnableI2C(void)

{
    UCB0IE |= UCTXIE0 + UCNACKIE;  // Enable eUSCI Interrupt: UCTXIE0 - Trans Interrupt; UCNACKIE - NACK Interrupt
}








