//
//  I2C.h
//  MSP430-Final
//
//  Created by Joshua Hayes on 26/04/2015.
//  Copyright (c) 2015 Joshua Hayes. All rights reserved.
//

#ifndef __MSP430_Final__I2C__
#define __MSP430_Final__I2C__

// Dependencies
#include <stdio.h>
#include <stdlib.h>
#include "msp430fr5739.h"
#include "thermister.h"

// Global Variables
extern unsigned char *data;
extern unsigned char new_data;
extern unsigned char data_size;

// Local Function Declarations
void setupClocks();
void configureI2CPins();
void updateData();
unsigned char multiByte();
void clearStop();
unsigned char getDigits(unsigned int);

#endif /* defined(__MSP430_Final__I2C__) */
