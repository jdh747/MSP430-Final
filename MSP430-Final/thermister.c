//
//  thermister.c
//  MSP430-Final
//
//  Created by Joshua Hayes on 26/04/2015.
//  Copyright (c) 2015 Joshua Hayes. All rights reserved.
//*******************************************************//

#include "thermister.h"


unsigned int thermisterReading()
{
    // Declarations
    unsigned int ADCTemp = 0;
    unsigned int CalValue = 0;
    
    SetupThermistor();                  // Setup Thermistor Ports & Registers
    CalValue = CalibrateADC();          // Calibration Reading
    TakeADCMeas();                      // Take ADC Sample
    ADCTemp = (ADCResult >= CalValue) ? (ADCResult - CalValue) : (CalValue - ADCResult);
    ShutDownTherm();                    // turn off Thermistor bridge for low power
    return ADCTemp;
}


void SetupThermistor()
{
    // ~16KHz sampling
    
    // Used For Vcc, To Power Voltage Divider??
    P2DIR |= BIT7;
    P2OUT |= BIT7;
    
    // P1.4 Is Used As Input From NTC Voltage Divider
    P1OUT &= ~BIT4;
    P1DIR |= BIT4;
    
    // Set P1.4 As Analog Input -> ADC
    P1SEL1 |= BIT4;
    P1SEL0 |= BIT4;
    
    // Allow For Settling Delay
    __delay_cycles(50000);
    
    // Configure ADC10 Control Registers
    ADC10CTL0 &= ~ADC10ENC;                             // Disable Conversion, ReEnable When Wanting To Take Reading
    ADC10CTL0 = ADC10SHT_7 + ADC10ON;                   // ADC10SHT_7: Sampling Timer Interval = 192xADC10CLK Cycles (192x5MHz) | ADC10ON: Enables ADC Core
    ADC10CTL1 = ADC10SHS_0 + ADC10SHP + ADC10SSEL_0;    // ADC10SHS_0: S&H Samples On ADC10SC Rising Edge | ADC10SHP: S&H Pulse Mode | ADC10SSEL_0: ADC10 Clock Source - MODCLK (5MHz Clock)
    ADC10CTL2 = ADC10RES;                    // 10-bit Conversion Resolution (12 Clock Cycle)
    ADC10MCTL0 = ADC10INCH_4;                // ADC10MCTL0: Conversion Memory Control Register | ADC10INCH_4: A4 Input Channel (P1.4) | Vref=AVCC (By Default)
    ADC10IE = ADC10IE0;                      // ADC10IE: ADC10B Interrupt Register | ADC10IE0: Enable Completed Conversion Interrupt
}


unsigned int CalibrateADC()
{
    unsigned char CalibCounter = 0;
    unsigned int Value = 0;
    
    while(CalibCounter < 50)
    {
        P3OUT ^= BIT4;                          // Switches P3.4 On/Off On Each Loop, Why?
        CalibCounter++;
        while (ADC10CTL1 & BUSY);               // Busy Bit (Bit0) - Active Sample or Conversion
        ADC10CTL0 |= ADC10ENC | ADC10SC ;       // ADC10ENC: Enable Conversion | ADC10SC: Start Conversion
        __bis_SR_register(CPUOFF + GIE);        // LPM0, ADC10_ISR Forces Exit
        
        __no_operation();                       // Single Cycle Delay
        Value += ADCResult;
    }
    Value = Value/50;                           // Averages ADCResult Between P3.4 Low/High State
    return Value;
}


void TakeADCMeas()
{
    while (ADC10CTL1 & BUSY);               // Waits For Active Conversions To Finish
    ADC10CTL0 |= ADC10ENC | ADC10SC ;       // Enable & Start Conversion
    __bis_SR_register(CPUOFF + GIE);        // LPM0, ADC10_ISR Forces Exit
    
    __no_operation();                       // Single Cycle Delay, For Debug
}


void ShutDownTherm()
{
    // Turn Off Vcc
    P2DIR &= ~BIT7;
    P2OUT &= ~BIT7;
    
    // Turn Off ADC Input Channel
    P1SEL1 &= ~BIT4;
    P1SEL0 &= ~BIT4;
    
    // Turn Off ADC
    ADC10CTL0 &= ~(ADC10ENC + ADC10ON);     // Disable Conversion & Turn Off ADC Core
    ADC10IE &= ~ADC10IE0;                   // Disable Conversion Interrupt
    ADC10IFG = 0;                           // Clear ADC Interrupt Flag Register
}










