//
//  I2C.h
//  MSP430-Final
//
//  Created by Joshua Hayes on 26/04/2015.
//  Copyright (c) 2015 Joshua Hayes. All rights reserved.
//*******************************************************//

#ifndef __MSP430_Final__I2C__
#define __MSP430_Final__I2C__

// Dependencies
#include <stdio.h>
#include <stdlib.h>
#include "msp430fr5739.h"
#include "thermister.h"

// Global Variables
extern struct {
    unsigned char new_data;
    unsigned char negative;
    unsigned char size;
    unsigned char *digit_array;
} data;

// Local Function Declarations
void setupClocks();
void configureI2CPins();
void updateData();
unsigned char multiByte();
void clearStop();
unsigned char getDigits(unsigned int);
void negativeDetect(int*);

#endif /* defined(__MSP430_Final__I2C__) */
