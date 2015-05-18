#include "peripherals.h"

volatile unsigned int ADCResult;
unsigned int ADCTemp = 0;
data_t data = { FALSE, FALSE, 13, "Temperature: " };

void main()
{
    // variable initialization
    WDTCTL = WDTPW + WDTHOLD;					// Kill Watchdog Timer (Resets Board @ Timer Limit): WDTPW - Password; WDTHOLD - Stops Timer
    SystemInit();								// Init Pins, Clocks & Interrupts
    
    while (1)
    {
        __delay_cycles(1000000);				// Waits For 1000 Master Clock Cycles
        TakeADCMeas();							// Assigns Temperature Reading To Global ADCTemp Variable
        updateData();							// Updates Data Struct
        while (UCB0CTLW0 & UCTXSTP);            // Ack of Stop Clears UCTXSTP, Must Be Checked Before Initialising New Transmission
        UCB0CTLW0 |= UCTR + UCTXSTT; 	        // UCTR - Transmit Select; UCTXSTT - Generates Start
        __bis_SR_register(CPUOFF + GIE);        // Enter LPM0 w/ Interrupts
    }
}

#pragma vector = USCI_B0_VECTOR                     // Declares Interrupt On USCI_B0, For NACKs & Transmit Buffer
__interrupt void USCIB0_ISR(void)
{
    unsigned char trans_fin = FALSE;                // Returned Bool From clearStop() To Indicate End-of-Trans
    
    switch (__even_in_range(UCB0IV, 0x1E))          // __even_in_range(x,y) Tells Compliler x Must Be Even In Range 0-y;
    {                                               // UCB0IV - USCIB0 Interrupt Vector Register; 0x1E - 30
        case 0x00: break;							// No Interrupt Pending;
        case 0x02: break;							// Arbitration Lost
        case 0x04:									// NACK
            UCB0CTLW0 |= UCTXSTT;					// Resend Start Condition
            break;
        case 0x18:									// Transmit Buffer Empty
            if (data.new_data)						// First 3 Calls Invoke clearStop(), Which Clears data.new_data
                clearStop();						// Clears Screen On Startup, Returns Cursor When Printing A New Temp Value
            else
                trans_fin = multiByte();			// Prints Temp Value Digit-By-Digit (On Each Interrupt) To Buffer, Returns True On Completion
            
            if (trans_fin)							// When Transmission Complete, Exit LPM To Update Data
            {
                trans_fin = FALSE;					// So We Dont Exit LPM In Next New Data Loop
                __bic_SR_register_on_exit(CPUOFF);	// Exit LPM0 | ..._on_exit(x) clears x From Register
            }
            break;
        default: break;
    }
}

#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
    switch (__even_in_range(ADC10IV, ADC10IV_ADC10IFG))
    {
        case ADC10IV_NONE: break;               // No Interrupt
        case ADC10IV_ADC10OVIFG: break;         // Conversion Result Overflow
        case ADC10IV_ADC10TOVIFG: break;        // Conversion Time Overflow
        case ADC10IV_ADC10HIIFG: break;         // ADC10HI
        case ADC10IV_ADC10LOIFG: break;         // ADC10LO
        case ADC10IV_ADC10INIFG: break;         // ADC10IN
        case ADC10IV_ADC10IFG:					// ADC10_B Memory Interrupt
            ADCResult = ADC10MEM0;				// ADC Digital Result Stored In ADC10MEM0 Conversion Register
            __bic_SR_register_on_exit(CPUOFF);	// Clear CPUOFF bit from 0(SR) 
            break;                                                            
        default: break;
    }
}