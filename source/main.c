#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>




  //////////////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////---------TIMER & ADC-------//////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////////

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







  ///////////////////////////////////////////////////////////////////////////////////////////
 ////////////////////////////////-----------GLOBAL-----------///////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

//FUNCS
void updateArrays();
void updateBullets();
void updatePlayer();
void resetBullet(int);

//VARS
char rowArray[6] = {0}; // The 6th element of col/rows is reserved for the player. 
char colArray[6] = {0};
int score = 0;
uint16_t x, y;
int reset_Counter = 0;
int lose = 0;
int win = 0;
int reset; //PIN A2







////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////-----------BULLET-----------///////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////


struct bullet {
	char x; //X & Y start from 0 for array access
	char y;
	char startX;
	char startY;
	char direction; // 0 up, 1 down, 2 left, 3 right

};

struct bullet bulletArray[5]; 


enum Bullet_States {bullet_init, update, bullet_wait} bullet_state;
int binit;
int bulletCounter = 0;
void bulletTick() {
		switch(bullet_state) {
			case bullet_init:
				updateArrays();
				for (binit = 0; binit < 5; binit++) {
					resetBullet(binit);
				}
			
				bullet_state = update;
				break;
			case update:
				if ((win == 1 ) || (lose == 1)) {
					bullet_state = bullet_wait;
					bulletCounter = 0;
				}
				if (bulletCounter == 1000) {
					bulletCounter = 0;
				updateArrays();
				updateBullets();
				}
				bulletCounter++;
				break;
			case bullet_wait:
				if ((win == 0) && (lose == 0)) {
									for (binit = 0; binit < 5; binit++) {
										resetBullet(binit);
									}
					updateArrays();
					bullet_state = update;	
				}
					
				break;
			
		}
		
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////-----------PLAYER-----------///////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
struct bullet Player = {4, 4, 0, 0, 0}; // We only use Player.x/y 

enum Player_States {player_init, player_idle, player_move, player_wait} player_state;
enum Player_Directions {Up, Down, Left, Right, init} player_direction;
int player_count = 0;

void playerTick() {
	switch(player_state) {
		case player_init: 
				Player.x = 4;
				Player.y = 4;
				player_direction = init;
				player_state = player_idle;
			break;
		case player_idle:

			
			if (x > 600) {
				player_direction = Right;
				player_state = player_move;
			}
			else if (x < 400) {
				player_direction = Left;
				player_state = player_move;
			}
			else if (y > 600) {
				player_direction = Up;
				player_state = player_move;
			}
			else if (y < 400) {
				player_direction = Down;
				player_state = player_move;
				
			}
			updatePlayer();
						if ((win == 1 )|| (lose == 1)) {
							player_state = player_wait;
						}

			break;
		case player_move:
			if ((win == 1 )|| (lose == 1)) {
				player_state = player_wait;
				player_count = 0;
			}
			if (player_direction == Up) {
				if (Player.y < 7) {
					Player.y += 1;
				}
			}
			else if (player_direction == Down) {
				if (Player.y > 0) {
					Player.y -= 1;
					
				}
			}
			else if (player_direction == Left) {
				if (Player.x > 0) {
					Player.x -= 1;
				}
			}
			else if (player_direction == Right) {
				if (Player.x < 7) {
					Player.x += 1;
				}
				
			}
					updatePlayer();
			player_direction = init;
			player_count++;
			
			if (player_count == 100) {
				player_state = player_idle;
				player_count = 0;
			}
			
		
			break;
		case player_wait:
			if ((win == 0) && (lose == 0)) {
						player_direction = init;
						player_state = player_idle;
						Player.x = 4;
						Player.y = 4;
						player_count = 0;
						win = 0;
			}	
			
			
						
			break;
		
		
	}
	
	
	
}



////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////-----------RENDER-----------///////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////




enum Render_States {render_init, draw, GAME_OVER, WINNER} render_state;

int i = 0;

void renderTick() {
	switch(render_state) {
		case render_init:
			render_state = draw;
			i = 0;
			break;
		case draw: //Draw the board by getting data from bullets & player
				if (reset == 1) {
					
				}
				if (lose == 1) {
					render_state = GAME_OVER;
				}
				else if (win == 1) {
					render_state = WINNER;
				}
				PORTB = rowArray[i];
				PORTD = ~colArray[i];
				i++;
				if (i == 6) {
					i = 0;
				}
								if (reset == 1) {
									lose = 1;
								}
			break;
		case GAME_OVER:
			PORTB = 0xFF;
			PORTD = 0x00;
			score = 0;
			lose = 0;
			render_state = render_init;
		
			break;
		case WINNER:
			PORTB = 0xFF;
			PORTD = 0x00;
			lose = 0;
			if (reset == 1) {
				win = 0;
				render_state = render_init;
			}
			
			
	}
	
	
	
}




























int main(void) {
	DDRD = 0xFF; PORTD = 0x00; // MATRIX ROWS
	DDRB = 0xFF; PORTB = 0x00; // MATRIX COLUMNS
	DDRA = 0x00; PORTA = 0xFF; // JOYSTICK INPUT
	TimerOn();
	TimerSet(1); 
	InitADC();
	bullet_state = bullet_init;
	render_state = render_init;
	player_state = player_init;
	int w;
	while(1) {
		reset = ((~PINA & 0x04) >> 2);
		x = readadc(1); //For some reason this is backwards, so 1 and 0 are switched
		y = readadc(0);
		if (score != 10) {
			for (w = 0; w < 5; w++) {
				if  ( (bulletArray[w].x == Player.x) && (bulletArray[w].y == Player.y) ) {
					lose = 1;
					Player.x = -1;
					Player.y = -1;
					
					
				}
				
			}
		}
		else if (score >= 10) {
			win = 1;
			score = 0;
			Player.x = -1;
			Player.y = -1;
		}
		bulletTick();
		renderTick();
		playerTick();
		

						

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
			score++;
			resetBullet(b);
			
		}
	}
		
		
		
}

void resetBullet(int current_Bullet) {
	
	bulletArray[current_Bullet].direction = rand() % 4;
	if (bulletArray[current_Bullet].direction == 0) { //UP     //Direction checks to make sure new bullets don't continue off screen based on their spawn point
		bulletArray[current_Bullet].startX = rand() % 8;
		bulletArray[current_Bullet].startY = 0;
	}
	else if (bulletArray[current_Bullet].direction == 1) { //DOWN
		bulletArray[current_Bullet].startX = rand() % 8;
		bulletArray[current_Bullet].startY = 7;
	}
	else if (bulletArray[current_Bullet].direction == 2) { //LEFT
		bulletArray[current_Bullet].startX = 7;
		bulletArray[current_Bullet].startY = rand() % 8;
	}
	else if (bulletArray[current_Bullet].direction == 3) { //RIGHT
		bulletArray[current_Bullet].startX = 0;
		bulletArray[current_Bullet].startY = rand() % 8;
	}
	
	bulletArray[current_Bullet].x = bulletArray[current_Bullet].startX;
	bulletArray[current_Bullet].y = bulletArray[current_Bullet].startY;

	
}

void updatePlayer() {
	
	rowArray[5] = 0b00000001 << (Player.x);
	colArray[5] = 0b00000001 << (Player.y);
	
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



