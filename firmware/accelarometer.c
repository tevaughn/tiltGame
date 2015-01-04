/*
 * ADC.c
 *
 *  April 4, 2014
 */

#include "defines.h"

#define X_AXIS	0
#define Y_AXIS	1
#define Z_AXIS	2
#define OFFSET	15

int X_NEG_THRESHOLD;
int X_0_THRESHOLD;
int X_POS_THRESHOLD;

int  Y_POS_THRESHOLD;
int  Y_0_THRESHOLD;
int  Y_NEG_THRESHOLD;

int count = 0;

unsigned int axisCount = X_AXIS;

typedef struct {
	volatile unsigned long x;	// x axis ADC
	volatile unsigned long y;	// y axis ADC
	//volatile unsigned long z;
} axis;

axis acceleromter = { 536 , 505};


void configADC10(void) {
	ADC10CTL1 = (INCH_0 | ADC10DIV_0 | CONSEQ_0);			// first channel, no clock div, single sample
	ADC10CTL0 = (ADC10SHT_0 | ADC10ON | ENC | ADC10SC );	// use ADC clock, turn on ADC and convert
	axisCount = X_AXIS;		// we are doing the x axis first

	X_0_THRESHOLD = acceleromter.x;		// calculate our thresholds;
	X_POS_THRESHOLD = X_0_THRESHOLD - OFFSET;
	X_NEG_THRESHOLD = X_0_THRESHOLD + OFFSET;

	Y_0_THRESHOLD = acceleromter.y;
	Y_POS_THRESHOLD = Y_0_THRESHOLD - OFFSET;
	Y_NEG_THRESHOLD = Y_0_THRESHOLD + OFFSET;
}

void configTimerA0(void) {
	TA0CTL |= TASSEL_2 + MC_1 + ID_3;	// SMCLK/8, up mode
	TA0CCR0 = QUARTERSECOND;	// 1/32 second
	TA0CCTL0 |= CCIE;
}

#pragma vector = TIMER0_A0_VECTOR	//says that the interrupt that follows will use the "TIMER0_A0_VECTOR" interrupt
__interrupt void Timer0_A0(void) {

		while(!(ADC10CTL0 & ADC10IFG));		// wait until we have a value
		acceleromter.x = (ADC10MEM);
		ADC10CTL0 &= ~(ADC10ON | ENC);		// reset adc
		ADC10CTL0 &= ~ADC10IFG;
		ADC10CTL1 = (INCH_1 | ADC10DIV_0 | CONSEQ_0);		// turn it on to sample y-axis
		ADC10CTL0 = (ADC10SHT_0 | ADC10ON | ENC | ADC10SC );

		while(!(ADC10CTL0 & ADC10IFG)); // wait until we have a value
		acceleromter.y = (ADC10MEM);
		ADC10CTL0 &= ~(ADC10ON | ENC); // reset adc
		ADC10CTL0 &= ~ADC10IFG;
		ADC10CTL1 = (INCH_0 | ADC10DIV_0 | CONSEQ_0); // turn it on to sample y-axis
		ADC10CTL0 = (ADC10SHT_0 | ADC10ON | ENC | ADC10SC );

		// check if we have reached a threshold
		if (acceleromter.x > X_NEG_THRESHOLD) {
			updateLocation(LEFT);
		} else if (acceleromter.x < X_POS_THRESHOLD) {
			updateLocation(RIGHT);
		}
		if (acceleromter.y > Y_NEG_THRESHOLD) {
			updateLocation(DOWN);
		} if (acceleromter.y < Y_POS_THRESHOLD) {
			updateLocation(UP);
		}

	TA0CCTL0 &= ~CCIFG; // clear flags
}
