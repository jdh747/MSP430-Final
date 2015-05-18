#include "peripherals.h"

extern data_t data;
extern volatile unsigned int ADCResult;
extern unsigned int ADCTemp;

void SystemInit(void)
{
    // Configure Clocks & Timers
    CSCTL0_H = 0xA5;						// Unlock Clock Register: CSKEY = 0xA5
    CSCTL1 |= DCOFSEL0 + DCOFSEL1;          // Set DCO = 8MHz -- Bit 0 + 1 = 11b = 8MHz
    CSCTL2 = SELA_3 + SELS_3 + SELM_3;      // Set Clocks Source: SELA/S/M_3 - ACLK(Aux Clock)/SMCLK(SubSys Clock)/MCLK(Master Clock) = DCO
    CSCTL3 = DIVA_3 + DIVS_3 + DIVM_3;      // Divide All 3 Clocks By 8 ie 1MHz
    CSCTL0_H = 0x01;                        // Lock Clock Register, Any Write Other Than 0xA5 Will Do
    
    // Turn off temp.
    REFCTL0 |= REFTCOFF;
    REFCTL0 &= ~REFON;
    
    // Enable LED
    P3OUT &= ~BIT4;
    P3DIR |= BIT4;
    
    // P1.4 Is Used As Input From NTC Voltage Divider
    // Set it to output low
    P1OUT &= ~BIT4;
    P1DIR |= BIT4;
    
    // P2.7 is used to power the voltage divider for the NTC thermistor
    // Used For Vcc, To Power Voltage Divider??
    P2OUT &= ~BIT7;
    P2DIR |= BIT7;
    
    // ~16KHz sampling
    //Turn on Power
    P2DIR |= BIT7;
    P2OUT |= BIT7;
    
    // Set P1.4 As Analog Input -> ADC
    P1SEL1 |= BIT4;
    P1SEL0 |= BIT4;
    
    // Allow For Settling Delay
    __delay_cycles(50000);
    
    // Configure ADC10 Control Registers
    ADC10CTL0 &= ~ADC10ENC;								// Disable Conversion, ReEnable When Wanting To Take Reading
    ADC10CTL0 = ADC10SHT_7 + ADC10ON;					// ADC10SHT_7: Sampling Timer Interval = 192xADC10CLK Cycles (192x5MHz) | ADC10ON: Enables ADC Core
    ADC10CTL1 = ADC10SHS_0 + ADC10SHP + ADC10SSEL_0;	// ADC10SHS_0: S&H Samples On ADC10SC Rising Edge | ADC10SHP: S&H Pulse Mode | ADC10SSEL_0: ADC10 Clock Source - MODCLK (5MHz Clock)
    ADC10CTL2 = ADC10RES;								// 10-bit Conversion Resolution (12 Clock Cycle)
    ADC10MCTL0 = ADC10INCH_4;							// ADC10MCTL0: Conversion Memory Control Register | ADC10INCH_4: A4 Input Channel (P1.4) | Vref=AVCC (By Default)
    ADC10IE = ADC10IE0;									// ADC10IE: ADC10B Interrupt Register | ADC10IE0: Enable Completed Conversion Interrupt
    
    
    // Configure Pins for I2C
    P1SEL1 |= BIT6 + BIT7;                              // Pin Function Select: BIT6: 1.6 UCB0SDA; BIT7: 1.7 UCB0SCL
    UCB0CTLW0 |= UCSWRST;                               // Put eUSCI_B In Reset State -- Logic Held
    UCB0CTLW0 |= UCMODE_3 + UCMST + UCSSEL_2;           // UCB0 Control Register: UCMODE_3 - I2C Mode; UCMST - Master Mode; UCSSEL_2 - Clock Source SMCLK
    UCB0BRW = 0x8;                                      // Baudrate Clock Source Divider ie SMCLK / 8 -- Serial Runs For Every 8 SubSys Clk Cycles
    UCB0I2CSA = 0x20;                                   // Slave Address -- 20hex
    UCB0CTLW0 &= ~UCSWRST;                              // Clear Reset -- Released For Operation
    UCB0IE |= UCTXIE0 + UCNACKIE;                       // transmit and NACK interrupt enable
}

void TakeADCMeas(void)
{
    unsigned int reading = 0;
    
    for (int i = 0; i < 4; i++)
    {
        while (ADC10CTL1 & BUSY);				// Waits For Active Conversions To Finish
        ADC10CTL0 |= ADC10ENC | ADC10SC;		// Enable & Start Conversion
        __bis_SR_register(CPUOFF + GIE);        // LPM0, ADC10_ISR Forces Exit
        __no_operation();                       // Single Cycle Delay, For Debug
        reading += ADCResult;
    }
    ADCTemp = reading / 4;						// 4-Point Average of ADC Reading Assigned To Global
}

unsigned char multiByte()
{
    static unsigned char transmitting = 1;				// Keeps Track of Count Between Calls
    unsigned char trans_fin = FALSE;					// Tells Interrupt Calling To Exit LPM
    
    if (transmitting)                                   // If There Is Still Data To Come
    {													// UCB0TXBUF - 8-Bit (1-Byte) Buffer For Writing;
        UCB0TXBUF = data.digit_array[transmitting - 1]; // Returns The Next Data Element For Transmission
        transmitting++;
        if (transmitting > data.size)					// End of Digit, Next Call To multiByte() Sends Stop Condition
            transmitting = 0;
    }
    else                                                // If This Is The Final Piece of Data, Reset Count & Amend new_data Indicator
    {
        transmitting = 14;                              // Reset For Future Calls
        UCB0CTLW0 |= UCTXSTP;							// I2C Stop Condition
        UCB0IFG &= ~UCTXIFG;							// Clear TX Buffer Empty Interrupt Flag, Automatic When Writing Data
        trans_fin = TRUE;
    }
    return trans_fin;
}

void clearStop()
{
    static unsigned char setup = TRUE;
    static unsigned char clearstate = 0;
    unsigned char clearcmds[12] = { LCD_COMMAND, LCD_HITACHI, LCD_CLEAR, LCD_COMMAND, LCD_POS, POS_0 };
    
    // Clear Screen @ Startup Or Change Position To Re-write Temp Value
    if (setup)
        UCB0TXBUF = clearcmds[clearstate];		// First 3 Clear Commands Clear Screen On Setup
    else
        UCB0TXBUF = clearcmds[clearstate + 3];	// Offset To The Last 3 Command For Subsequent Calls
    
    clearstate++;
    
    if (clearstate > CLEARCMDS_CNT - 1)			// Done Clearing/Changing Pos
    {
        clearstate = 0;                         // Reset For Next Call
        data.new_data = FALSE;                  // Redirect Next Buffer Allocation To multiByte()
        setup = FALSE;                          // All Future Calls Move Cursor, Not Clear
    }
}

void updateData()
{
    // Declarations
    int temp;               // Stores Lookup Table Result
    unsigned char digits;   // Digit Count That Can Be Decremented
    
    // Temperature Lookup Table, Conversion From ADC Reading -> Deg Celsius
    const static int templookup[1024] = {277, 229, 188, 167, 154, 144, 136, 129, 124, 119, 115, 111, 108, 105, 102, 100, 98, 95, 93, 92, 90, 88, 87, 85, 84, 82, 81, 80, 79, 78, 77, 75, 74, 74, 73, 72, 71, 70, 69, 68, 68, 67, 66, 65, 65, 64, 63, 63, 62, 61, 61, 60, 60, 59, 59, 58, 57, 57, 56, 56, 55, 55, 55, 54, 54, 53, 53, 52, 52, 51, 51, 51, 50, 50, 49, 49, 49, 48, 48, 48, 47, 47, 47, 46, 46, 46, 45, 45, 45, 44, 44, 44, 43, 43, 43, 42, 42, 42, 42, 41, 41, 41, 40, 40, 40, 40, 39, 39, 39, 39, 38, 38, 38, 38, 37, 37, 37, 37, 36, 36, 36, 36, 36, 35, 35, 35, 35, 34, 34, 34, 34, 34, 33, 33, 33, 33, 33, 32, 32, 32, 32, 32, 31, 31, 31, 31, 31, 30, 30, 30, 30, 30, 30, 29, 29, 29, 29, 29, 28, 28, 28, 28, 28, 28, 27, 27, 27, 27, 27, 27, 27, 26, 26, 26, 26, 26, 26, 25, 25, 25, 25, 25, 25, 24, 24, 24, 24, 24, 24, 24, 23, 23, 23, 23, 23, 23, 23, 22, 22, 22, 22, 22, 22, 22, 22, 21, 21, 21, 21, 21, 21, 21, 20, 20, 20, 20, 20, 20, 20, 20, 19, 19, 19, 19, 19, 19, 19, 19, 18, 18, 18, 18, 18, 18, 18, 18, 17, 17, 17, 17, 17, 17, 17, 17, 17, 16, 16, 16, 16, 16, 16, 16, 16, 16, 15, 15, 15, 15, 15, 15, 15, 15, 15, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 13, 13, 13, 13, 13, 13, 13, 13, 13, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -4, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -5, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, -6, -7, -7, -7, -7, -7, -7, -7, -7, -7, -7, -7, -7, -7, -7, -8, -8, -8, -8, -8, -8, -8, -8, -8, -8, -8, -8, -8, -8, -8, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -9, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -10, -11, -11, -11, -11, -11, -11, -11, -11, -11, -11, -11, -11, -11, -11, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -12, -13, -13, -13, -13, -13, -13, -13, -13, -13, -13, -13, -13, -13, -13, -13, -14, -14, -14, -14, -14, -14, -14, -14, -14, -14, -14, -14, -14, -14, -15, -15, -15, -15, -15, -15, -15, -15, -15, -15, -15, -15, -15, -15, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -16, -17, -17, -17, -17, -17, -17, -17, -17, -17, -17, -17, -17, -17, -17, -18, -18, -18, -18, -18, -18, -18, -18, -18, -18, -18, -18, -18, -19, -19, -19, -19, -19, -19, -19, -19, -19, -19, -19, -19, -19, -19, -20, -20, -20, -20, -20, -20, -20, -20, -20, -20, -20, -20, -20, -21, -21, -21, -21, -21, -21, -21, -21, -21, -21, -21, -21, -21, -21, -22, -22, -22, -22, -22, -22, -22, -22, -22, -22, -22, -22, -22, -23, -23, -23, -23, -23, -23, -23, -23, -23, -23, -23, -23, -24, -24, -24, -24, -24, -24, -24, -24, -24, -24, -24, -24, -24, -25, -25, -25, -25, -25, -25, -25, -25, -25, -25, -25, -25, -26, -26, -26, -26, -26, -26, -26, -26, -26, -26, -26, -26, -27, -27, -27, -27, -27, -27, -27, -27, -27, -27, -27, -28, -28, -28, -28, -28, -28, -28, -28, -28, -28, -28, -28, -29, -29, -29, -29, -29, -29, -29, -29, -29, -29, -29, -30, -30, -30, -30, -30, -30, -30, -30, -30, -30, -31, -31, -31, -31, -31, -31, -31, -31, -31, -31, -32, -32, -32, -32, -32, -32, -32, -32, -32, -32, -33, -33, -33, -33, -33, -33, -33, -33, -33, -33, -34, -34, -34, -34, -34, -34, -34, -34, -34, -35, -35, -35, -35, -35, -35, -35, -35, -36, -36, -36, -36, -36, -36, -36, -36, -36, -37, -37, -37, -37, -37, -37, -37, -37, -38, -38, -38, -38, -38, -38, -38, -39, -39, -39, -39, -39, -39, -39, -39, -40, -40, -40, -40, -40, -40, -40, -41, -41, -41, -41, -41, -41, -42, -42, -42, -42, -42, -42, -42, -43, -43, -43, -43, -43, -43, -44, -44, -44, -44, -44, -45, -45, -45, -45, -45, -45, -46, -46, -46, -46, -46, -47, -47, -47, -47, -47, -48, -48, -48, -48, -49, -49, -49, -49, -50, -50, -50, -50, -51, -51, -51, -51, -52, -52, -52, -52, -53, -53, -53, -54, -54, -54, -55, -55, -55, -56, -56, -56, -57, -57, -58, -58, -59, -59, -59, -60, -60, -61, -62, -62, -63, -63, -64, -65, -65, -66, -67, -68, -69, -70, -71, -72, -74, -75, -77, -79, -82, -86, -92};
    
    temp = templookup[ADCTemp];					// Store Converted Value
    negativeDetect(&temp);						// Test For Negative Temp & If True, Amend data.negative & Remove ADCTemp Sign
    digits = getDigits(temp) + 13;              // Get Size of Temp -- In Digits
    if (data.negative)                          // data.digit_array Needs To Store Extra (-) Character, If Negative
        digits++;
    data.size = digits;                         // Store Digits In Global Data Struct For Other Functions Use
    
    while (digits > 13)
    {
        if ((digits == 14) && data.negative)    // If Value Is Negative We Store Sign On Final Iteration
        {
            data.digit_array[14] = '-';
        } else
        {
            data.digit_array[digits - 1] = (unsigned char)((temp % 10) + 48);	// Attain Last Digit, Convert To ASCII, Typecast & Store
            temp /= 10;															// Remove Last Digit
        }
        digits--;
    }
    data.new_data = TRUE;						// New Data Available
}

unsigned char getDigits(unsigned int num)
{
    unsigned char digits = 0;
    
    if (num == 0)				// Zero Would Break Loop
        digits = 1;
    else
    {
        while (num >= 1)
        {
            num /= 10;          // Removes Last Digit
            digits++;           // Increments Count
        }
    }
    return digits;
}

void negativeDetect(int *num)
{
    if (*num < 0)               // If Less Than Zero, Update Data Struct & Remove Sign
    {
        *num *= -1;
        data.negative = TRUE;
    }
    else
        data.negative = FALSE;
}