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

/* Make sure you read it!!! */
/* global variables */
/* Avoid using these */


//USE PB4 for pwm signal

int main(){

	//DDRA = 0x00; //SET ALL OF THE PORT A TO INPUT BITS
	DDRB = 0xFF; //SET ALL OF THE PORT B TO OUTPUT BITS //step 6

	TCCR2A =(1<<COM2A1) | (1<<WGM21) | (1<<WGM20);    //fast PWM Step 1 & 3
	TIMSK2=(1<<OCIE2A);    //set to Compare match //step 2 
	

	TCCR2B = (1<<CS20) | (1<<CS21) | (1<<CS22); //prescale to 1024

	OCR2A = 127; //step 5
	
	return(0);
}/* main */
		


		
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
