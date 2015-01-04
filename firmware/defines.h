#include <msp430.h>
#include "stdint.h"

/*
 * defines.h
 *
 *  Created on: Apr 1, 2014
 *      Author: Taylor
 */

#ifndef DEFINES_H_
#define DEFINES_H_

/* Port 2 */
#define BUTTON_A	BIT0
#define BUTTON_B	BIT1
#define LED_WHITE	BIT2
#define LED_BLUE	BIT3

#define LCD_MODE	BIT4
#define LCD_RST		BIT5

/* Port 1 */
#define XAXIS	BIT0
#define YAXIS	BIT1
#define ZAXIS	BIT2

#define LCD_PWM		BIT3
#define SPI_ENABLE	BIT4
#define CLOCK		BIT5
#define SIMO		BIT7


#define SECOND			0x2EE0 		// VLO runs at 12KHz
#define HALFSECOND		(SECOND >> 1)
#define QUARTERSECOND	(SECOND >> 2) // VLO runs at 12KHz
#define EIGHTSECOND		(SECOND >> 3) // VLO runs at 12KHz
#define SIXTEENTHSECOND (SECOND >> 4) // VLO runs at 12KHz

#define RIGHT	2
#define LEFT	3
#define DOWN	1
#define UP		0

#define HORZ_MAX	15
#define VERT_MAX	9

#define BLOCK_MAX 	5
#define COLUMN_MAX	77

// function prototypes
void buttonsConfig(void);
void configADC10(void);
void configTimerA0(void);
void configLCD(void);
void writeStringToLCD(const char *string);
void writeCharToLCD(char c);
void writeBlockToLCD(char *byte, unsigned char length);
void writeGraphicToLCD(char *byte, unsigned char transform);
void writeToLCD(unsigned char dataCommand, unsigned char data);
void clearLCD();
void clearBank(unsigned char bank);
void setAddr(unsigned char xAddr, unsigned char yAddr);
void initLCD();
void mazeInit(void);
void updateLocation(int);




#endif /* DEFINES_H_ */
