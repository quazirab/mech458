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
#include <avr/interrupt.h>

void timerCount();

/* Make sure you read it!!! */
/* global variables */
/* Avoid using these */
volatile int count=0;

void debug();
		
int main(){	
	char step[4] = {0x30,0x06,0x28,0x05};
		
	
	//element eTest;		/* A variable to hold the aggregate data type known as element */
  DDRA = 0xFF; //SET ALL OF THE PORT A TO Output BITS
  //DDRB = 0xFF; //SET ALL OF THE PORT B TO OUTPUT BITS
				
	DDRC = 0xFF;
	PORTC = 0xf0;
	timerCount(1000);
	PORTC = 0;
	
	while(1){
		int stepCount=100;
		int j=0;
		//int k;
		for(int i=0;i<stepCount;i++){
			PORTA = step[j];
			timerCount(20); 
			printf(step[j]);
			j++;
			
		if (j>3) j=0;
			}//for 
	 	//if(i == stepCount)break;
	  
	}//while
	
	return(0);
}/* main */
		


/**************************************************************************************/
/***************************** Timer***************************************************/
/**************************************************************************************/

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
		}
	}
	return;
}		
		
