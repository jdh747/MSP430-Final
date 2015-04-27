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
    unsigned char temp = 0;
    unsigned int CalValue = 0;
    
    SetupThermistor();                  // One time setup and calibration
    CalValue = CalibrateADC();
    TakeADCMeas();                      // Take 1 ADC Sample
    if (ADCResult >= CalValue)
    {
        temp = DOWN;
        ADCTemp = ADCResult - CalValue;
    }
    else
    {
        temp = UP;
        ADCTemp = CalValue - ADCResult;
    }
    ShutDownTherm();                    // turn off Thermistor bridge for low power
    return ADCTemp;
}

void SetupThermistor()
{
    // Declarations
    volatile unsigned char ThreshRange[3]={0,0,0};
    
    // ~16KHz sampling
    //Turn on Power
    P2DIR |= BIT7;
    P2OUT |= BIT7;
    
    // Configure ADC
    P1SEL1 |= BIT4;
    P1SEL0 |= BIT4;
    
    // Allow for settling delay
    __delay_cycles(50000);
    
    // Configure ADC
    ADC10CTL0 &= ~ADC10ENC;
    ADC10CTL0 = ADC10SHT_7 + ADC10ON;        // ADC10ON, S&H=192 ADC clks
    // ADCCLK = MODOSC = 5MHz
    ADC10CTL1 = ADC10SHS_0 + ADC10SHP + ADC10SSEL_0;
    ADC10CTL2 = ADC10RES;                    // 10-bit conversion results
    ADC10MCTL0 = ADC10INCH_4;                // A4 ADC input select; Vref=AVCC
    ADC10IE = ADC10IE0;                      // Enable ADC conv complete interrupt
    
    // Setup Thresholds for relative difference in Thermistor measurements
    ThreshRange[0]=15;
    ThreshRange[1]=25;
    ThreshRange[2]=45;
}

unsigned int CalibrateADC()
{
    unsigned char CalibCounter =0;
    unsigned int Value = 0;
    
    // Disable interrupts & user input during calibration
    DisableSwitches();
    while(CalibCounter <50)
    {
        P3OUT ^= BIT4;
        CalibCounter++;
        while (ADC10CTL1 & BUSY);
        ADC10CTL0 |= ADC10ENC | ADC10SC ;       // Start conversion
        __bis_SR_register(CPUOFF + GIE);        // LPM0, ADC10_ISR will force exit
        __no_operation();
        Value += ADCResult;
    }
    Value = Value/50;
    // Reenable switches after calibration
    EnableSwitches();
    return Value;
}

void TakeADCMeas()
{
    while (ADC10CTL1 & BUSY);
    ADC10CTL0 |= ADC10ENC | ADC10SC ;       // Start conversion
    __bis_SR_register(CPUOFF + GIE);        // LPM0, ADC10_ISR will force exit
    __no_operation();                       // For debug only
}

void ShutDownTherm()
{
    // Turn off Vcc
    P2DIR &= ~BIT7;
    P2OUT &= ~BIT7;
    
    // Turn off ADC Channel
    P1SEL1 &= ~BIT4;
    P1SEL0 &= ~BIT4;
    
    // Turn off ADC
    ADC10CTL0 &= ~(ADC10ENC + ADC10ON);
    ADC10IE &= ~ADC10IE0;
    ADC10IFG = 0;
}