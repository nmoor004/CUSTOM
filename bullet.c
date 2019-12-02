#include <avr/io.h>
#include <avr/interrupt.h>


void MATRIX();


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






/*

struct bullet {
	char x; //X & Y start from 0 for array access
	char y;
	char start;
	char direction; // 0 up, 1 down, 2 left, 3 right

};

bullet A;

bullet bulletArray[1] = {A};

int bulletCount = 0;
enum Bullet_States {idle, move} bullet_state;



void bulletTick() {
		switch(bullet_state) {
			case idle:
				break;
			case move:
				break;
			
			
		}
		
		switch(bullet_state) {
			case idle:
				break;
			case move:
				break;
			
		}
}

*/




enum Render_States {init, draw} render_state;
char rowArray[6] = {0b00000001, 0b00000010, 0b00000100}; //0b00100000, 0b00010000, 0b00001000, 0b00000100}; 
char colArray[6] = {0b00000001, 0b00000001, 0b00000001}; //0b00100000, 0b00010000, 0b00001000, 0b00000100};


void renderTick() {
	switch(render_state) {
		case init:
			render_state = draw;
			break;
		case draw: //Draw the board by getting data from bullets & player
				MATRIX();
				
			
			break;
		
		
	}
	
	
	
}

int main(void) {
	DDRD = 0xFF; PORTD = 0x00;
	DDRB = 0xFF; PORTB = 0x00;
	TimerOn();
	TimerSet(10); 
	render_state = init;
	
	while(1) {
		renderTick();
	
		while(!TimerFlag);
		TimerFlag = 0;
		
	}
	

	
	
	
	return 1;
}


int ROWS, COLUMNS;

void MATRIX() {
	 ROWS = 0;
	 COLUMNS = 0;
	for (ROWS = 0; ROWS < 3; ROWS++) {
		for (COLUMNS = 0; COLUMNS < 3; COLUMNS++) {
			PORTB = rowArray[ROWS];
			PORTD = ~colArray[COLUMNS];
			
		}
	
	
	}
	
}
	
