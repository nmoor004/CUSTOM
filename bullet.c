#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>



  ///////////////////////////////////////////////////////////////////////////////////////////
 ////////////////////////////////-----------GLOBAL-----------///////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

//FUNCS
void updateArrays();
void updateBullets();

//VARS
char rowArray[6] = {0}; // The 6th element of col/rows is reserved for the player. 
char colArray[6] = {0};





  /////////////////////////////////////////////////////////////////////////////////////////
 ////////////////////////////////-----------TIMER-----------//////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

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








struct bullet {
	char x; //X & Y start from 0 for array access
	char y;
	char startX;
	char startY;
	char direction; // 0 up, 1 down, 2 left, 3 right

};


//struct bullet A = {1, 1, 0, 0, 0};

struct bullet bulletArray[5];


enum Bullet_States {bullet_init, update} bullet_state;

int bulletCounter = 0;
void bulletTick() {
		switch(bullet_state) {
			case bullet_init:
				bullet_state = update;
				break;
			case update:
				if (bulletCounter == 1000) {
					bulletCounter = 0;
				updateArrays();
				updateBullets();
				}
				bulletCounter++;
				break;
			
			
		}
		
		switch(bullet_state) {
			case bullet_init:
				break;
			case update:
				break;
			
		}
}






enum Render_States {render_init, draw} render_state;

int i = 0;

void renderTick() {
	switch(render_state) {
		case render_init:
			render_state = draw;
			break;
		case draw: //Draw the board by getting data from bullets & player
				
				PORTB = rowArray[i];
				PORTD = ~colArray[i];
				i++;
				if (i == 6) {
					i = 0;
				}
			break;
			
	}
	
	
	
}

int main(void) {
	DDRD = 0xFF; PORTD = 0x00;
	DDRB = 0xFF; PORTB = 0x00;
	TimerOn();
	TimerSet(1); 
	bullet_state = bullet_init;
	render_state = render_init;
	
	while(1) {
		bulletTick();
		renderTick();
	
		while(!TimerFlag);
		TimerFlag = 0;
		
	}
	

	
	
	
	return 1;
}


void updateBullets() {
	int b = 0;
	int bDirection;
	for (b = 0; b < 5; b++) {
		bDirection = bulletArray[b].direction;
		
		if (bDirection == 0) {
			bulletArray[b].y += 1;
			
		}
		else if (bDirection == 1) {
			bulletArray[b].y -= 1;
		}
		else if (bDirection == 2) {
			bulletArray[b].x -=1;
		} 
		else if (bDirection == 3) {
			bulletArray[b].x +=1;
		}
		
		if ((bulletArray[b].x > 8) || (bulletArray[b].y > 8) || (bulletArray[b].x < 0) || (bulletArray[b].y < 0)) { //Respawn bullet
			
			

			bulletArray[b].direction = rand() % 4; 
			if (bulletArray[b].direction == 0) { //UP     //Direction checks to make sure new bullets don't continue off screen based on their spawn point
				bulletArray[b].startX = rand() % 8;
				bulletArray[b].startY = 0;
			}
			else if (bulletArray[b].direction == 1) { //DOWN
				bulletArray[b].startX = rand() % 8;
				bulletArray[b].startY = 7;
			}
			else if (bulletArray[b].direction == 2) { //LEFT
				bulletArray[b].startX = 7;
				bulletArray[b].startY = rand() % 8;
			}
			else if (bulletArray[b].direction == 3) { //RIGHT
				bulletArray[b].startX = 0;
				bulletArray[b].startY = rand() % 8;
			}
			
						bulletArray[b].x = bulletArray[b].startX;
						bulletArray[b].y = bulletArray[b].startY;
		}
			
	}
		
		
		
}
	


void updateArrays() {
	int j = 0;
	for (j = 0; j < 5; j++) { //We only update the first 5 elements of the row/colArrays since element 6 is handled by updatePlayer
		int posX = bulletArray[j].x;
		int posY = bulletArray[j].y;
		
		rowArray[j] = 0b00000001 << (posX);
		colArray[j] = 0b00000001 << (posY);
	
	}
}