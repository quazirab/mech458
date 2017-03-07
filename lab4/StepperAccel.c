/* 	
	Course		: UVic Mechatronics 458
	Milestone	: 4 (Stepper Motor)
	Title		: Stepper Motor
	Name 1:	Evan McMillan	Student ID: V00203520
	Name 2:	Quazi Rabbi		Student ID: V00778792
	
	Description: This code initializes the motor with a 50-step rotation to determine the location of the stepper,
	then rotates clockwise and anti-clockwise by 30 degrees, then 60 degrees, then 180 degrees.
*/

/* include libraries */
#include <stdlib.h>
#include <avr/io.h>

#include <util/delay_basic.h>
#include <avr/interrupt.h>

void timerCount();
void forward(int fCount);
void backward (int bCount);

/* Make sure you read it!!! */
/* global variables */
/* Avoid using these */
volatile int countStep=0;				//Global variable to track the location of the stepper
volatile int coilCount = 0;				//Global variable to track which coil was last used
//char step[4] = {0x30,0x06,0x28,0x05};	//Array containing the bits to send to the stepper driver for each of the four steps
char step[4] = {0x38,0x0E,0x2B,0x15};
//PB0 = L4
//PB1 = L4
//PB2 = E2
//PB3 = L2
//PB4 = L1
//PB5 = E1
		
int main(){	
	
		
	
  DDRA = 0xFF; //SET ALL OF THE PORT A TO Output BITS
				
	DDRC = 0xFF;		//Set PORTC to all output
	PORTC = 0xf0;		//Set LED bank to the leftmost 4 lights lit, guarantee the routine is running properly
	timerCount(1000);
	PORTC = 0;			//Set all LEDs to blank
			/*
			forward(50);		//initialize the stepper by rotating forwards 50 steps.
			timerCount(1000);
			
			forward(17);		//Rotate clockwise approximately 30 degrees
			timerCount(1000);
			backward(17);		//Rotate anti-clockwise approximately 30 degrees
			timerCount(1000);
			forward(33);		//Rotate clockwise by 60 degrees
			timerCount(1000);
			backward(33);		//Rotate anti-clockwise by 60 degrees
			timerCount(1000);
			forward(100);		//Rotate clockwise by 180 degrees
			timerCount(1000);
			backward(100);		//Rotate anti-clockwise by 180 degrees
*/
			forward(400);
			timerCount(1000);
			backward(400);
			timerCount(1000);
			forward(200);
			timerCount(50);
			backward(200);
			
			timerCount(3000);
	 
	
	return(0);
}/* main */
		


/**************************************************************************************/
/*****************************Stepper Motor forward************************************/
/**************************************************************************************/

void forward(int fCount){
	int delay = 20;
	
	for(int i=0;i<fCount;i++){
		coilCount++;					//powers the next coil (clockwise) from the last one that fired
		if (coilCount>3) coilCount=0;	//if the coil moves a full 4 steps, move it back to the first coil
		PORTA = step[coilCount];		//Send the signal to the motor driver on PORTA for each individual step
		timerCount(delay);
		if(i <= 10) delay -= 1;
		/*if(i > 5 && i < 10) delay -= 2;
		if(fCount - i <10 && fCount	- i >=5) delay +=2;*/
		if(fCount -i <10) delay +=1;
		countStep++;					//track where in the rotation the stepper is, referenced to when it was turned on (future will be reference to the hall sensor)
		if(countStep>200)countStep=0;	//if the stepper rotates a full 360 degrees, reset to the initial value and start again
	}//for
		
}

/**************************************************************************************/
/*****************************Stepper Motor Backward************************************/
/**************************************************************************************/

void backward (int bCount){
	int delay = 20;
	for(int i=0;i<bCount;i++){
		coilCount--;					//powers the previous coil (anti-clockwise) from the last one that fired
		if (coilCount<0) coilCount=3;	//if the coil moves a full 4 steps, move it back to the fourth coil in order to continue reversing (anti-clockwise)
		PORTA = step[coilCount];		//Send the signal to the motor driver on PORTA for each individual step
		timerCount(delay);
		if(i <= 10) delay -= 1;
		/*if(i > 5 && i < 10) delay -= 2;
		if(bCount - i <10 && bCount	- i >=5) delay +=2;*/
		if(bCount -i <10) delay +=1;
		countStep--;					//track where in the rotation the stepper is, referenced to when it was turned on (future will be reference to the hall sensor)
		if(countStep<0)countStep=200;	//if the stepper rotates a full 360 degrees, reset to the initial value and start again
	}//for
}


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
	
	//Enable the output compare interrupt enable
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
		
