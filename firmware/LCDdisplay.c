/*
 * LED.c
 *
 *  Created on: Apr 1, 2014
 *      Author: Taylor
 */

#include "PCD8544.h"
#include "defines.h"

#define LCD_SELECT			P1OUT &= ~SPI_ENABLE
#define LCD_DESELECT		P1OUT |= SPI_ENABLE
#define LCD_SET_COMMAND 	P2OUT &= ~LCD_MODE
#define LCD_SET_DATA 		P2OUT |= LCD_MODE
#define COMMAND 		0
#define DATA			1

#define SPI_MSB_FIRST UCB0CTL0 |= UCMSB // or UCA0CTL0 |= UCMSB (USCIA) or USICTL0 &= ~USILSB (USI)
#define SPI_LSB_FIRST UCB0CTL0 &= ~UCMSB // or UCA0CTL0 &= ~UCMSB or USICTL0 |= USILSB (USI)

void configLCD(void) {

    P1OUT |= SPI_ENABLE;
    P1DIR |= SPI_ENABLE;

    P2DIR |= LCD_MODE + LCD_RST;
    P2OUT |= (LCD_MODE + LCD_RST);

    // setup USIB
    P1SEL |= CLOCK + SIMO;
    P1SEL2 |= CLOCK + SIMO;

    P2OUT &= ~LCD_RST;
    //_delay_cycles(10);
    P2OUT |= LCD_RST;


    UCB0CTL0 |= UCCKPH + UCMSB + UCMST + UCSYNC;	// 3-pin, 8-bit SPI master
    UCB0CTL1 |= UCSSEL_2;							// SMCLK
    UCB0BR0 |= 0x01;            // 1:1
    UCB0BR1 = 0;
    UCB0CTL1 &= ~UCSWRST;       // clear SW

    initLCD();
    clearLCD();
    mazeInit();
}


/*
 * Writes a string of characters to the LCD
 * input: *string - pointer to a string you want to write to the LCD
 */
void writeStringToLCD(const char *string) {
    while(*string) {
        writeCharToLCD(*string++);
    }
}


/*
 * Writes a single character to the LCD
 * input: char - you want to write to the LCD
 */
void writeCharToLCD(char c) {
    unsigned char i;
    for(i = 0; i < 5; i++) {
        writeToLCD(DATA, font[c - 0x20][i]);
    }
    writeToLCD(DATA, 0);
}

/*
 * Writes one block of data to the LCD
 * inputs: *byte - pointer to a byte of data to write
 * 			lenght - how many bytes to write
 */
void writeBlockToLCD(char *byte, unsigned char length) {
    unsigned char c = 0;
    while(c < length) {
        writeToLCD(DATA, *byte++);
        c++;
    }
}

/*
 * Transforms a graphic by flipping or rotating a block and then writes to the LCD
 * inputs: *byte - pointer to a byte of data to write
 * 			transform - ROTATE, FLIP_V, FLIP_H
 */
void writeGraphicToLCD(char *byte, unsigned char transform) {
    int c = 0;
    char block[8];

    if(transform & FLIP_V) {
        SPI_LSB_FIRST;
    }
    if(transform & ROTATE) {
        c = 1;
        while(c != 0) {
            (*byte & 0x01) ? (block[7] |= c) : (block[7] &= ~c);
            (*byte & 0x02) ? (block[6] |= c) : (block[6] &= ~c);
            (*byte & 0x04) ? (block[5] |= c) : (block[5] &= ~c);
            (*byte & 0x08) ? (block[4] |= c) : (block[4] &= ~c);
            (*byte & 0x10) ? (block[3] |= c) : (block[3] &= ~c);
            (*byte & 0x20) ? (block[2] |= c) : (block[2] &= ~c);
            (*byte & 0x40) ? (block[1] |= c) : (block[1] &= ~c);
            (*byte & 0x80) ? (block[0] |= c) : (block[0] &= ~c);
            *byte++;
            c <<= 1;
        }
    } else {
        while(c < 8) {
            block[c++] = *byte++;
        }
    }

    if(transform & FLIP_H) {
        c = 7;
        while(c > -1) {
            writeToLCD(DATA, block[c--]);
        }
    } else {
        c = 0;
        while(c < 8) {
            writeToLCD(DATA, block[c++]);
        }
    }
    SPI_MSB_FIRST;
}

/*
 * The function which actually sends data to the LCD
 * inputs: dataCommand - DATA or COMMAND
 * 			data - the data or command you wish to write
 */
void writeToLCD(unsigned char dataCommand, unsigned char data) {
    LCD_SELECT;		// enable SPI comms with display
    if(dataCommand) {		// if sending data
    	LCD_SET_DATA;	// send data (set MODE)
    } else {
    	LCD_SET_COMMAND;	// else, reset MODE
    }
    UCB0TXBUF = data;		// write data
    while(!(IFG2 & UCB0TXIFG)); // wait
    LCD_DESELECT;		// disable SPI comms with display
}


/*
 * Increments through the entire dispay setting every pixel to 0
 */
void clearLCD() {
    setAddr(0, 0);
    int c = 0;
    while(c < PCD8544_MAXBYTES) {
        writeToLCD(DATA, 0);
        c++;
    }
    setAddr(0, 0);
}

/*
 * Increments through a single bank and sets those pixles to 0
 */
void clearBank(unsigned char bank) {
    setAddr(0, bank);
    int c = 0;
    while(c < PCD8544_HPIXELS) {
        writeToLCD(DATA, 0);
        c++;
    }
    setAddr(0, bank);
}

/*
 * Sets a new address where to write the next graphic
 * inputs: xAddr - x axis address
 * 			yAddr - y axis address
 */
void setAddr(unsigned char xAddr, unsigned char yAddr) {
    writeToLCD(COMMAND, PCD8544_SETXADDR | xAddr);
    writeToLCD(COMMAND, PCD8544_SETYADDR | yAddr);
    //_delay_cycles(200);
}

/*
 * Initilizes the display. I don't really know what this does.
 */
void initLCD() {
    writeToLCD(COMMAND, PCD8544_FUNCTIONSET | PCD8544_EXTENDEDINSTRUCTION);
    writeToLCD(COMMAND, PCD8544_SETVOP | 0x3F);
    writeToLCD(COMMAND, PCD8544_SETTEMP | 0x02);
    writeToLCD(COMMAND, PCD8544_SETBIAS | 0x03);
    writeToLCD(COMMAND, PCD8544_FUNCTIONSET);
    writeToLCD(COMMAND, PCD8544_DISPLAYCONTROL | PCD8544_DISPLAYNORMAL);
}
