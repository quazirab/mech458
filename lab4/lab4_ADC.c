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


//USE PD6 for pwm signal for led
//USE PC5 as pot input

volatile double dutyCycle=0;

int main(){

	DDRC = 0x00; //SET ALL OF THE PORT A TO INPUT BITS
  DDRD = 0xFF; //SET ALL OF THE PORT D TO OUTPUT BITS

  debug();	
	while(1){
    
	}//while
	
	return(0);
}/* main */
		
    
void setupADC(){
  ADMUX = (1<< REFS0)|(1<<MUX0)|(1<<MUX2);
  ADCSRA=(1<<ADEN)|(1<<ADIE)|(1<<ADPS0)|(1<<ADPS1)|(1<<ADPS2);
  DIDR0 = (1<<ADC5D);
  
  startConversion();
}

void startConversion(){
  ADCSRA |= (1<<ADSC);

}

ISR(ADC_vect){
  dutyCycle = ADC;
  startConversion();
}
void debug(){
	
		TCCR0A =(1<<COM0A1) | (1<<WGM00) | (1<<WGM01);    //fast PWM
    TIMSK0=(1<<TOIE0);    //set to overflow
  
    sei();
		TCCR0B = (1<<CS00);
		
}//debug
	
ISR(TIMER0_OVF_vect){
    OCR0A = dutyCycle;
}//isr

		
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
