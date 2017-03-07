/* 	
	Course		: UVic Mechatronics 458
	Milestone	: 5 (PWM Signal)
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
void setupPWM();
void timerCount(int tim);
void breakVcc(int tim);

//PF1 for ADC input 
//PB7 for PWM output 
//PB5 for Direction Switch
//PB0 TO EB
//PB1 TO EA
//PB2 TO IB
//PB3 TO IA

volatile double dutyCycle=0;
volatile int ADC_result_flag = 0;
const char dcDrive[4] = {0x00,0x04,0x08,0x0c};

int main(){

	DDRB = 0b10001111; 	//SET PORT 
	DDRC = 0xFF;;	//SET ALL OF THE PORT C TO OUTPUT BITS
	DDRD = 0xFF;	//SET ALL OF THE PORT D TO OUTPUT BITS
	DDRF = 0x00;	//SET ALL OF PORT F TO INPUT BITS
	
	cli();
	setupADC();
	setupPWM();
	sei();
	startConversion();	

	while(1){
		/*if(ADC_result_flag){
			//PORTC = dutyCycle;
			OCR0A = dutyCycle;
			ADC_result_flag = 0;
		}//if*/
			
		if((PINB & 0x10) == 0x10){
			breakVcc(20);
			PORTB = dcDrive[1];	//Drive DC motor CW
			while((PINB & 0x10) == 0x10){
				if(ADC_result_flag){
					PORTC = dutyCycle;
					OCR0A = dutyCycle;
					ADC_result_flag = 0;
				}//if
			}//while
		}//if
		if((PINB & 0x10) == 0x00){

			breakVcc(20);
			PORTB = dcDrive[2];	//DRIVE DC MOTOR CCW
			while((PINB & 0x10) == 0x00){
				if(ADC_result_flag){
					PORTC = dutyCycle;
					OCR0A = dutyCycle;
					ADC_result_flag = 0;
				}//if
			}//while
			
		}//if
	}//while
	return(0);
}/* main */

/**************************************************************************************/
/************************************PWM***********************************************/
/**************************************************************************************/
void setupPWM(){
	TCCR0A =(1<<COM0A1) | (1<<WGM01) | (1<<WGM00);    //fast PWM Step 1 & 3
	//TIMSK0=(1<<OCIE0A);    //set to Compare match //step 2 
	TCCR0B = (1<<CS01); //prescale to 9
}//setupPWM

void breakVcc(int tim){
	PORTB  = dcDrive[0];	//Brake to VCC before changing direction
	timerCount(tim);
}

/**************************************************************************************/
/*************************************ADC**********************************************/
/**************************************************************************************/
void setupADC(){
	ADMUX = (1<< REFS0)|(1<<MUX0)|(1<<ADLAR);				// Left adjust, and use VCC as top reference
	ADCSRA=(1<<ADEN)|(1<<ADIE);                     //
	DIDR0 = (1<<ADC5D); 
}/*setupADC*/

void startConversion(){
	ADCSRA |= (1<<ADSC);
}/*startConversion*/

ISR(ADC_vect){
	dutyCycle = ADCH;
	ADC_result_flag = 1;
	startConversion();
}/*ISR*/

/**************************************************************************************/
/*************************************Timer********************************************/
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
	//TIMSK1 = TIMSK1|0x02;
	
	//Clear the timer interrupt flag and begin timer
	TIFR1 = (1<<OCF1A);
	
	//Poll the timer to determine when the timer has reached 1ms
	//Timer is set for 1 ms, so it will be in while for tim X 1ms.
	while(i<tim){
		if((TIFR1 & 0x02)==0x02){  // sees if the 0CF1A flag is up
			
			TIFR1 =(1<<OCF1A);
			i++;
		}//if
	}//while
	return;
}
