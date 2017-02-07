/* Solution Set for the LinkedQueue.c */
/* 	
	Course		: UVic Mechatronics 458
	Milestone	: 3
	Title		: Data structures for MCUs and the Linked Queue Library

	Name 1:					Student ID:
	Name 2:					Student ID:
	
	Description: You can change the following after you read it.
	
	This main routine will only serve as a testing routine for now. At some point you can comment out
	The main routine, and can use the following library of functions in your other applications

	To do this...make sure both the .C file and the .H file are in the same directory as the .C file
	with the MAIN routine (this will make it more convenient)
*/

/* include libraries */
#include <stdlib.h>
#include <avr/io.h>
#include <util/delay_basic.h>
#include<avr/interrupt.h>

/* Make sure you read it!!! */
/* global variables */
/* Avoid using these */

	void debug();
	void elemExt();
	void display_led();

	volatile char readInput;
	volatile char Input1;
	volatile char Input2;
	volatile char Input3;
	volatile char Input4;
	volatile int state=1;

		
int main(){	

	
	//element eTest;		/* A variable to hold the aggregate data type known as element */

	DDRA = 0x00; //SET ALL OF THE PORT A TO INPUT BITS
  	DDRC = 0xFF; //SET ALL OF THE PORT B TO OUTPUT BITS

	while(1){
  
  		 
		
  		while((PINA & 0x04) == 0x04);
				timerCount(20);
		while((PINA & 0x04) == 0x00);
				timerCount(20);
				debug();			
	
			
		
	}//while
	
	return(0);
}/* main */
		
	
void debug(){
	readInput = (PINA & 0x03); //ONLY THE INPUT BITS ARE READ
	switch (state){
		case (1):				
		//Input1 = readInput;
		state++;
		break;
		
		case (2):
		Input2 = readInput;
		state++;
		break;
		
		case (3):
		Input3 = readInput;
		state++;
		break;
		
		case (4):
		Input4 = readInput;
		display_led();
		state++;
		break;

		case (5):
		PORTC = 0;
		state =1;
		break;
		
	}//switch
		
		
}//debug
	
	
void display_led(){

	PORTC = Input2;
	timerCount(2000);
	PORTC |= (Input3<<2);
	timerCount(2000);
	PORTC |= (Input4<<4);
	timerCount(2000);

	
//	Input1=NULL;
	Input2=NULL;
	Input3=NULL;
	Input4=NULL;

	
}//display
	


		
void timerCount(int tim){
	
	int i = 0;
	
	//Set Presaler TO 1
	TCCR1B = (1<<CS10);
	
	//Set timer clear on Comparision CTC
	TCCR1B |= (1<<WGM12);
	
	
	
	//Comparison Register to 1000 cycles for 1 ms
	OCR1A = 0x03e8;		
	
	//Set inital Value of the timer Counter to 0x0000
	TCNT1 = 0x0000;
	
	//Enable the output compare interrut enable
	TIMSK1 = TIMSK1|0x02;
	
	//Clear the timer interrupt flag and begin timer
	TIFR1 = (1<<OCF1A);
	
	//Poll the timer to determine when the timer has reached 1ms
	//Timer is set for 1 ms, so it will be in while for tim X 1ms.
	while(i<tim){
		if((TIFR1 & 0x02)==0x02){  // sees if the 0CF1A flag is up 
			
			TIFR1 =(1<<OCF1A);
			i++;
		}//if
	}//whil
	return;
}//timer count		
		
