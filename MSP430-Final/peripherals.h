#ifndef PERIPHERALS
#define PERIPHERALS

#include "msp430fr5739.h"

// Macros
#define LCD_COMMAND 0x80    // Command Start
#define LCD_HITACHI 0x05    // Hitachi Commands Pass Through
#define LCD_HOME 0x02       // Clears Screen & Sets Pos 0
#define LCD_CLEAR 0x01		// Clear Command
#define LCD_POS 0x08		// Position Command
#define POS_0 0x0D			// Pos 13 - After "Temperature: "
#define CLEARCMDS_CNT 3		// Number of Commands To Process When Clearing
#define TRUE 1
#define FALSE 0

void SystemInit(void);
void TakeADCMeas(void);
unsigned char multiByte();
void clearStop();
void updateData();
unsigned char getDigits(unsigned int);
void negativeDetect(int *num);

typedef struct {
    unsigned char new_data;
    unsigned char negative;
    unsigned char size;
    unsigned char digit_array[20];
} data_t;

#endif