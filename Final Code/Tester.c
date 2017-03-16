/* 	
	Course		: UVic Mechatronics 458
	Milestone	: 5 (DC Motor Control)
	Title		: DC Motor Control with PWM and ADC
	Name 1:	Evan McMillan	Student ID: V00203520
	Name 2:	Quazi Rabbi		Student ID: V00778792
	
	Description: Set up an ADC reading off PIN F1 to read from a potentiometer, using Vcc as the top reference voltage and GND as
	the bottom reference. Output an 8-bit representation of the converted value to the PORTC LEDs, and set the 8-bit value to the
	PWM output compare register to set the PWM duty cycle.
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
void setupInterrupt();

/*Usage of pins in this program*/
//PORTC for LEDs
//PF1 for ADC input 
//PB7 for PWM output 
//PB5 for Direction Switch
//PB0 TO EB
//PB1 TO EA
//PB2 TO IB
//PB3 TO IA

volatile unsigned char ADClowerbit;
volatile unsigned char ADChigherbit;								//Global variable for storing the current ADC conversion value
volatile int ADC_result_flag = 0;							//Global variable for the ADC conversion flag
const char dcDrive[4] = {0x00,0x04,0x08,0x0c};				//Array containing the DC motor driver truth table
volatile int STATE=0;
volatile unsigned short lowVal1;
volatile unsigned short lowVal2;							//short is 16 bit(2byte)


int main(){

	DDRB = 0b10001111; 	//SET PORT B LOWER 4 BITS TO OUTPUT, WITH PB4,PB5,PB6 TO INPUT, AND PB7 TO OUTPUT
	DDRC = 0xFF;;	//SET ALL OF THE PORT C TO OUTPUT BITS
	DDRD = 0xF0;	//SET ALL OF THE PORT D TO OUTPUT BITS
	DDRF = 0x00;	//SET ALL OF PORT F TO INPUT BITS
	
	PORTB = dcDrive[1];
	cli();				//Disable Global interrupts
	setupADC();			//Run the ADC Setup Subroutine below
	setupPWM();			//Run the PWM setup Subroutine below
	setupInterrupt();
	
	sei();				//Enable global interrupts
	
	
	
	while(1){
	if ((PINB & 0x10) == 0x10){
		PORTB = dcDrive[1];
	}
	if((PINB & 0x10) == 0x00){
		
		PORTB = 0;
	}
	
			 
	}//while
	return(0);
}/* main */

/**************************************************************************************/
/************************************Interrupt***********************************************/
/**************************************************************************************/
void setupInterrupt(){
	EICRA |= _BV(ISC00);			//sets to any edges
	EIMSK |= 0x01;
}

ISR(INT0_vect){
			
	if(STATE==0){
		STATE=1;			//changes the STATE to 1
		lowVal2=0x03FF;			//intial highest vale to 1023
		startConversion();		//start conversion, goes to ADC	
		return;				//gets out of the loop
		}//if
	
	else {
		STATE=0;			//next edge
		ADClowerbit = lowVal2 & 0xFF;	//takes the lower 8 bits of the 16 bit
		ADChigherbit = lowVal2>>8;	//takes the higer 8 bits of the 16 bit
		PORTD = (ADChigherbit<<5);	//15-8bits, shifting it right by 5because Adlar =0
		PORTC = ADClowerbit;		//7-0 bits, lower full bits
		return;				//gets out of the loop
	}//else
}//ISR
/**************************************************************************************/
/************************************PWM***********************************************/
/**************************************************************************************/
void setupPWM(){
	TCCR0A =(1<<COM0A1) | (1<<WGM01) | (1<<WGM00);	//Set timer counter compare register to Fast PWM
	TCCR0B = (1<<CS01);								//prescale to 9
	OCR0A = 100;		
}//setupPWM


/*Subroutine to brake to VCC for 'tim' milliseconds. New motor direction is set in the main code after this is called.*/
void brakeVcc(int tim){
	PORTB  = dcDrive[0];	//Brake to VCC before changing direction
	timerCount(tim);		//Delay 
}

/**************************************************************************************/
/*************************************ADC**********************************************/
/**************************************************************************************/
void setupADC(){
	ADMUX = (1<< REFS0)|(1<<MUX0);						// Left adjust, and use VCC as top reference
	ADCSRA=(1<<ADEN)|(1<<ADIE);							//Set the values of the ADC Enable and ADC Interrupt Enable bits to 1
	DIDR0 = (1<<ADC1D);									//Turns off the digital input buffer for ADC1 on PF1
}/*setupADC*/

void startConversion(){
	ADCSRA |= (1<<ADSC);								//Set the ADC Start Conversion bit on ADCSRA to 1.
}/*startConversion*/

ISR(ADC_vect){
	lowVal1 = ADC;			//puts the full 10 bit value to lowVal1	
	if(lowVal2>lowVal1){		//if lowVal2 is greater than lowVal1
		lowVal2=lowVal1;	//lowVal2 is equal to lowVal1 
	}//if
	if(STATE==1) startConversion();	//conversion continues till state ==0;			
		

	
}//

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
	
	//Enable the output compare interrupt enable
	//TIMSK1 = TIMSK1|0x02;						//Timer mask is disabled in order to not need an ISR routine for it.
	
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
}//timerCount
