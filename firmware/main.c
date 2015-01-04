
#include "defines.h"

/*
 *
 * main.c
 */

typedef struct {
	char horz;
	char vert;
} posistion;

posistion cursor;	// current position
posistion start;	// the beginning of the maze
posistion end;		// the end
/*
char wallHorz[HORZ_MAX+1][VERT_MAX] = {
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,
	START,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	1,
		1,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	1,
		1,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	1,
		1,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	1,
		1,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	1,
		1,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	1,
		1,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	END,
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,
		};
char wallVert[HORZ_MAX][VERT_MAX+1] = {
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1, 0
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	0,	1, 0
		1,	0,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1, 0
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	0,	1, 0
		1,	0,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1, 0
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	0,	1, 0
		1,	0,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1, 0
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	0,	1, 0
		1,	0,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1, 0
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1, 0
};
*/
// 20x8 cells
const char wallVert[10][3] = {
		0x00, 0x00, 0x10,
		0x80, 0x00, 0x10,
		0x80, 0x00, 0x10,
		0x80, 0x00, 0x10,
		0x80, 0x00, 0x10,
		0x80, 0x00, 0x10,
		0x80, 0x00, 0x10,
		0x00, 0x00, 0x10,
		0x00, 0x00, 0x00,
		0x00, 0x00, 0x00,
};

const char wallHorz[12][3] = {
		0xFF, 0xFF, 0xF0,
		0xFF, 0xFF, 0xC0,
		0x7F, 0xFF, 0xF0,
		0xFF, 0xFF, 0xC0,
		0x7F, 0xFF, 0xF0,
		0xFF, 0xFF, 0xC0,
		0x7F, 0xFF, 0xF0,
		0xFF, 0xFF, 0xC0,
		0xFF, 0xFF, 0xF0,
		0x00, 0x00, 0x00,
		0x00, 0x00, 0x00,
		0x00, 0x00, 0x00,
};


void clockConfig(){
	BCSCTL1 &= ~XTS;		// Put in low frequency
	BCSCTL2 &= ~SELS;		// Selects DCO for SMLCK
	BCSCTL3 |= LFXT1S_2;	// Select VLO for ACLK

	BCSCTL1 = CALBC1_1MHZ;            // 1MHz clock
	DCOCTL = CALDCO_1MHZ;

	_enable_interrupt();	//turn on interrupts, we're ready to go
}

int main(void) {
 	WDTCTL = WDTPW + WDTHOLD;	//Stop WDT

	clockConfig();		// set up clocks
	configLCD();		// initilize maze
	configTimerA0();	// set up time for ADC polling
	configADC10();		// turn on ADC for accelerometer

    __bis_SR_register(GIE);	// turn on global interrupts
	_BIS_SR(LPM1_bits);	// go into low power mode



    while(1) {

    	// shouldn't ever get here
    }


}

/*
 * Does a "backwards shift" for negative numbers, other wise performs the expected forward shift
 *
 */
char cShift (char x, signed char shift) {
	return (shift < 0) ? (x << (~shift+1)) : (x >> shift);
}

/*
 * updates the entire display, including the cursor
 * takes the one bit horz and vert lines and interprets them to 5x5 sized cells of pixels and puts them into 8 bit verticle blocks
 */
void updateDisplay(void) {

	int column;
	unsigned char columnOut;
	unsigned char block;
	unsigned char row;
	char data;
	// Very complex bit arithmetic.
	for(block = 0; block < BLOCK_MAX; block++) {
		column = -1;
		setAddr(0, block + 1);
		row = (block << 1);
		for (columnOut = 0; columnOut < COLUMN_MAX; columnOut++) {
			if (columnOut%4 != 0) {		// If it's not an intersection
				data = cShift((wallHorz[row][(column >> 3)]  & (0x80 >> (column%8))), (7 - column%8)) |		//is there a horz wall on the top
						(((cursor.vert == row) && (cursor.horz == column) && (columnOut%4 == 2)) << 2) |	//is the cursor in this cell
						cShift((wallHorz[row+1][(column >> 3)]  & (0x80 >> (column%8))),(3 - column%8)) |	// is there a horz wall on the bot
						(((cursor.vert == (row+1)) && (cursor.horz == column)&& (columnOut%4 == 2)) << 6);	// is the cursor in this cell
			} else if (columnOut%4 == 0) {
				column++;
				data = cShift(((wallHorz[row][(column >> 3)] | wallVert[row][(column >> 8)])  & (0x80 >> (column%8))), 7 - column%8) |	//is there a horz or a vert wall at the top
					cShift((wallVert[row][(column >> 3)]  & (0x80 >> (column%8))), 6 - column%8) |	// is there a vert wall on the top
					cShift((wallVert[row][(column >> 3)]  & (0x80 >> (column%8))), 5 - column%8) |	// is there a vert wall on the top
					cShift((wallVert[row][(column >> 3)]  & (0x80 >> (column%8))), 4 - column%8) |	// is there a vert wall on the top
					cShift(((wallHorz[row+1][(column >> 3)] | wallVert[row+1][(column >> 8)])  & (0x80 >> (column%8))), 3 - column%8) | //is there a horz or a vert wall at the bot
					cShift((wallVert[row+1][(column >> 3)]  & (0x80 >> (column%8))), 2 - column%8) | // is there a vert wall on the bot
					cShift((wallVert[row+1][(column >> 3)]  & (0x80 >> (column%8))), 1 - column%8) | // is there a vert wall on the bot
					cShift((wallVert[row+1][(column >> 3)]  & (0x80 >> (column%8))), 0 - column%8);	// is there a vert wall on the bot
			}



			writeBlockToLCD(&data, 1); // send
		}
	}

}

void updateLocation(int direction) {
	if (direction ==  LEFT) {	// check if there is wall there, make sure this isn't the start (we don't want to fall off the screen!)
		if (!(wallVert[(cursor.vert)][(cursor.horz >> 3)] & (0x80 >> (cursor.horz)%8)) && !((cursor.vert == start.vert) && (cursor.horz == start.horz))) {
			cursor.horz--;
			updateDisplay();
		}
	}
	if (direction ==  RIGHT) { // check if there is wall
		if (!(wallVert[(cursor.vert)][((cursor.horz + 1)>> 3)] & (0x80 >> (cursor.horz + 1)%8))) {
			cursor.horz++;
			updateDisplay();
		}
	}
	if (direction ==  UP) { // check if there is wall
		if (!(wallHorz[cursor.vert][(cursor.horz >> 3)] & (0x80 >> (cursor.horz)%8))){
			cursor.vert--;
			updateDisplay();
		}
	}
	if (direction ==  DOWN) { // check if there is wall
		if (!(wallHorz[cursor.vert + 1][(cursor.horz >> 3)] & (0x80 >> (cursor.horz)%8))){
			cursor.vert++;
			updateDisplay();
		}
	}
		// check for the win condition
	if (cursor.vert == end.vert && cursor.horz == end.horz) {
		clearLCD();
		setAddr(20, 10);
		writeStringToLCD("WINNER!!!!"); // display win screen
		_BIS_SR(LPM4_bits);		// turn off

	}
}



void mazeInit(void) {
	start.vert = 0;	// set beginning
	start.horz = 0;

	end.vert = 7; 	// set end
	end.horz = 0;

	cursor.horz = start.horz; // start off at the beginning of the maze
	cursor.vert = start.vert;

	clearLCD(); //CLEAR because we do

	writeStringToLCD("MAZE GAME");	// write the NAME
	updateDisplay();		// start the GAME!!

}
