//
//  thermister.h
//  MSP430-Final
//
//  Created by Joshua Hayes on 26/04/2015.
//  Copyright (c) 2015 Joshua Hayes. All rights reserved.
//*******************************************************//

#ifndef __MSP430_Final__thermister__
#define __MSP430_Final__thermister__

// Dependencies
#include <stdio.h>
#include <stdlib.h>
#include "msp430fr5739.h"

// Global Definitions & Declarations
#define ADC_START_ADD 0xD400
#define ADC_END_ADD 0xF000
#define DOWN 0
#define UP 1
extern volatile unsigned int ADCResult;

// Local Function Declarations
unsigned int thermisterReading();
void SetupThermistor(void);
unsigned int CalibrateADC(void);
void TakeADCMeas(void);
void ShutDownTherm(void);

#endif /* defined(__MSP430_Final__thermister__) */
