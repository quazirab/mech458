/* 	
	Course		: UVic Mechatronics 458
	Milestone	: 4 (PWM Signal)
	Title		: PWM Signal
	Name 1:	Evan McMillan	Student ID: V00203520
	Name 2:	Quazi Rabbi		Student ID: V00778792
	
	Description: Set up a PWM signal with a 50% duty cycle at a frequency of 488 Hz.
*/

/* include libraries */
#include <stdlib.h>
#include <avr/io.h>
#include <util/delay_basic.h>
#include <avr/interrupt.h>


int main(){

	//DDRA = 0x00; //SET ALL OF THE PORT A TO INPUT BITS
	DDRB = 0xFF; //SET ALL OF THE PORT B TO OUTPUT BITS //step 6

	TCCR2A =(1<<COM2A1) | (1<<WGM21) | (1<<WGM20);    //fast PWM Step 1 & 3
	TIMSK2=(1<<OCIE2A);    //set to Compare match //step 2 
	
	
	TCCR2B = (1<<CS21); //prescale to 8

	//OCR2A = 127; //step 5
	OCR2A = 0x007f; //Step 5, set 
	
	
	
	return(0);
}/* main */
		
