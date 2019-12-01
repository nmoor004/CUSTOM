
#include <avr/io.h>
#include <stdlib.h>
#include <math.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

uint16_t x, y;
unsigned char xData, yData;





////////////////////////////////////////////////////
////////////////TIMER FUNCTIONS
//////////////////////////////////////////////////

volatile unsigned char TimerFlag = 0; // TimerISR() sets this to 1. C Programmer should clear to 0.
unsigned long _avr_timer_M = 1;	       // Start count from here, down to 0. Default 1 ms.
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks
unsigned short count = 0x00;
void TimerOn() {
	// AVR timer/counter controller register TCCR1
	TCCR1B = 0x0B;     // bit3 = 0: CTC mode (clear timer on compare)
	// bit2bit1bit0 = 011: pre-scaler /64
	// 00001011: 0x0B
	// SO, 8 MHz clock or 8,000,000 / 64 = 125,000 ticks/s
	// Thus, TCNT1 register will count at 125,000 tick/s
	// AVR output compare register OCR1A.
	OCR1A = 125;    // Timer interrupt will be generated when TCNT1==OCR1A
	// We want a 1 ms tick. 0.001s * 125,000 ticks/s = 125
	// So when TCNT1 register equals 125,
	// 1 ms has passed. Thus, we compare to 125.
	// AVR timer interrupt mask register
	TIMSK1 = 0x02; // bit1: OCIE1A -- enables compare match interrupt
	// Initialize avr counter
	TCNT1 = 0;
	_avr_timer_cntcurr = _avr_timer_M;
	// TimerISR will be called every _avr_timer_cntcurr milliseconds
	// Enable globla interrupts
	SREG |= 0x80; // 0x80: 1000000
}
void TimerOff() {
	TCCR1B = 0x00; /// bit3bit1bit0 = 000: timer off
}
void TimerISR() {
	TimerFlag = 1;
}
// In our approach, the C programmer does not touch this ISR, but rather TimerISR()
ISR(TIMER1_COMPA_vect) {
	// CPU automatically calls when TCNT1 == OCR1 (every 1 ms per TimerOn settings)
	_avr_timer_cntcurr--; // Count down to 0 rather than up to TOP
	if (_avr_timer_cntcurr == 0) { // results in a more efficient compare
		TimerISR(); 	      // Call the ISR that the user uses
		_avr_timer_cntcurr = _avr_timer_M;
	}
}
// Set TimerISR() to tick every M ms
void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}













void InitADC(void)
{
	ADMUX|=(1<<REFS0);
	ADCSRA|=(1<<ADEN)|(1<<ADPS0)|(1<<ADPS1)|(1<<ADPS2); //ENABLE ADC, PRESCALER 128
}

uint16_t readadc(uint8_t ch)
{
	ch&=0b00000111;         //ANDing to limit input to 7
	ADMUX = (ADMUX & 0xf8)|ch;  //Clear last 3 bits of ADMUX, OR with ch
	ADCSRA|=(1<<ADSC);        //START CONVERSION
	while((ADCSRA)&(1<<ADSC));    //WAIT UNTIL CONVERSION IS COMPLETE
	return(ADC);        //RETURN ADC VALUE
}




unsigned char LEDCombo = 0x00;
enum X_States {xinit, xidle, xtoggleD0} xstate;

void Xtick() {

	switch(xstate) {
		case xinit:
			x = 0;

			xData = 0;
			xstate = xidle;
			PORTC = 0;
		break;
		case xidle:
			PORTC &= 0xFD;
			PORTC |= 0x01;
			if (x > 700) {
				xstate = xtoggleD0;
			}
			
			
		break;
		case xtoggleD0:
		PORTC &=0xFE;
		PORTC |= 0x02;
			if (xData == 0) {
				LEDCombo |= 0x40;
				xData = 1;
				xstate = xidle;
			}
			else if (xData == 1){
				LEDCombo &= 0xBF;
				xData = 0;
				xstate = xidle;
				}
		break;
	}



}

enum Y_States {yinit, yidle, ytoggleD1} ystate;

void Ytick() {

	switch(ystate) {
		case yinit:
			y = 0;
			yData = 0;
			ystate = yidle;
			PORTC = 0;
		break;
		case yidle:
			PORTC &= 0xF7;
			PORTC |= 0x04;
			if (y > 700) {
				ystate = ytoggleD1;
			}
			
			
		break;
		case ytoggleD1:
			PORTC &=0xFB;
			PORTC |= 0x08;
			if (yData == 0) {
				LEDCombo |= 0x80;
				yData = 1;
				ystate = yidle;
			}
			else if (yData == 1){
				LEDCombo &= 0x7F;
				yData = 0;
				ystate = yidle;
				}
		break;
	}



}


enum Combine_States {cinit, ccombine} combine_state;

void combineTick() {
	switch(combine_state) {
		
		case cinit: 
			LEDCombo = 0;
			combine_state = ccombine;
		break;
		case ccombine:
			PORTC |= LEDCombo;
			if (yData == 0) {
				PORTC &= 0x7F;
			}
			if (xData == 0) {
				PORTC &= 0xBF;
				
			}
			combine_state = ccombine;
		break;
		
		
		
	}
	
	
	
	
	
	
}
int main(void) {
	/* Insert DDR and PORT initializations */
	DDRA = 0x00; PORTA = 0xFF; // JOYSTICK INPUT
	DDRC = 0xFF; PORTC = 0x00; //STATE OUTPUT
	ystate = yinit;
	xstate = xinit;
	combine_state = cinit;
	TimerSet(200);
	TimerOn();
	InitADC();
	//unsigned short UD, LR;
	/* Insert your solution below */
	while (1) {
		x = readadc(0); //Read A0
		y = readadc(1); //Read A1

		Xtick();
		Ytick();
		combineTick();
		while (!TimerFlag);
		TimerFlag = 0;

	}
	return 1;
}
