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

void setupADC();
void startConversion();
//void debug();
void timerCount(int tim);


/* Make sure you read it!!! */
/* global variables */
/* Avoid using these */


//USE PD6 for pwm signal for led
//USE PC5 as pot input

volatile double dutyCycle=0;
volatile int ADC_result_flag = 0;

int main(){
	 cli();
	DDRC = 0xFF;;	//SET ALL OF THE PORT C TO OUTPUT BITS
	DDRD = 0xFF;	//SET ALL OF THE PORT D TO OUTPUT BITS
	DDRF = 0x00;	//SET ALL OF PORT F TO INPUT BITS

	//debug();
	setupADC();	
	
	sei();
	startConversion();
	PORTC = 0xff;
	
	while(1){
		
		if(ADC_result_flag){
			PORTC = dutyCycle;
			ADC_result_flag = 0;
		}//if
		
	}//while
	
	return(0);
}/* main */
		
    
void setupADC(){
  
  ADMUX = (1<< REFS0)|(1<<MUX0)|(1<<ADLAR);											// USE EXTERNAL POWER SOURCE
  ADCSRA=(1<<ADEN)|(1<<ADIE)/*|(1<<ADPS0)|(1<<ADPS1)|(1<<ADPS2)*/;				//
  DIDR0 = (1<<ADC5D);
  
  
}

void startConversion(){
	ADCSRA |= (1<<ADSC);
}

ISR(ADC_vect){
  dutyCycle = ADC;
  ADC_result_flag = 1;
  startConversion();
}
