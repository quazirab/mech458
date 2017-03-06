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

void setupADC();
void startConversion();

//PF1 for ADC input 
//___ for PWM output 

int main(){

	DDRB = 0xFF; 	//SET ALL OF THE PORT B TO OUTPUT BITS //step 6
	DDRC = 0xFF;;	//SET ALL OF THE PORT C TO OUTPUT BITS
	DDRD = 0xFF;	//SET ALL OF THE PORT D TO OUTPUT BITS
	DDRF = 0x00;	//SET ALL OF PORT F TO INPUT BITS
	
	cli();
	setupADC();
	setupPWM();
	sei();
	//startConversion(); change if it doesnt work
	PORTC = 0xff;
	
	while(1){
		if(ADC_result_flag){
			PORTC = dutyCycle;
			ADC_result_flag = 0;
		}//if
		
	}//while
	return(0);
}/* main */

/**************************************************************************************/
/************************************PWM***********************************************/
/**************************************************************************************/
void setupPWM(){
	TCCR2A =(1<<COM2A1) | (1<<WGM21) | (1<<WGM20);    //fast PWM Step 1 & 3
	TIMSK2=(1<<OCIE2A);    //set to Compare match //step 2 
	TCCR2B = (1<<CS21); //prescale to 9
}//setupPWM

ISR(TIMER2_OVF_vect){
	if(ADC_result_flag){
		OCR2A = dutyCycle;
		ADC_result_flag = 0;
		}//if
}//isrPWM

/**************************************************************************************/
/*************************************ADC**********************************************/
/**************************************************************************************/
void setupADC(){
	ADMUX = (1<< REFS0)|(1<<MUX0)|(1<<ADLAR);				// Left adjust, and use VCC as top reference
	ADCSRA=(1<<ADEN)|(1<<ADIE);                     //
	DIDR0 = (1<<ADC5D);
	startConversion();
 
}/*setupADC*/

void startConversion(){
	ADCSRA |= (1<<ADSC);
}/*startConversion*/

ISR(ADC_vect){
	dutyCycle = ADCH;
	ADC_result_flag = 1;
	startConversion();
}/*ISR*/