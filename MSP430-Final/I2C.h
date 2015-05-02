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

// Macros
#define LCD_COMMAND 0x80    // Command Start
#define LCD_HITACHI 0x05    // Hitachi Commands Pass Through
#define LCD_HOME 0x02       // Clears Screen & Sets Pos 0
#define CLEARCMDS_CNT 3

// Local Function Declarations
void setupClocks();
void configureI2CPins();
void updateData();
unsigned char multiByte();
void clearStop();
unsigned char getDigits(unsigned int);
void negativeDetect(int*);
void DisableI2C(void);
void EnableI2C(void);

#endif /* defined(__MSP430_Final__I2C__) */
