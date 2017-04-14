/*
 * Stepper_test.c
 *
 * Created: 2017-04-13 1:16:22 PM
 *  Author: mech458
 */ 
/* include libraries */
#include <stdlib.h>
#include <avr/io.h>
#include <util/delay_basic.h>
#include <avr/interrupt.h>

char step[4] = {0x36,0x2e,0x2d,0x35};
volatile int8_t coilCount = 0;
volatile int ocr3value=2500;
volatile int timer3comcount;
volatile int8_t stepperworking=0; 
volatile int stepperPOS=0;
volatile uint8_t stepperdir;			//1- clockwise,2-cclockwise
volatile uint8_t countStep=0;
volatile uint8_t posStepCount =0;		//for desired position 
volatile uint8_t stepperdifference=0;
volatile uint8_t stepAccel;
volatile uint8_t stepDecel;
volatile uint8_t stepperHome = 0;
void timer3Setup();
void stepperPos(int pos);
void timerCount(int tim);
void StepperHome();
void setupInterrupt();

int main(void)
{
	DDRA = 0xff;
	DDRC = 0xff;
	DDRD = 0xf0;
	cli();
	timer3Setup();
	setupInterrupt();
	sei();
	//int loop = 0;
	StepperHome();
	timerCount(500);
	while(1){
		if (stepperworking==0){

			stepperPos(3);
			
			}
			
	}
    
}
void setupInterrupt(){
	EICRA |= _BV(ISC31);				//HE - RESESTS THE STEPPER COUNT - FALLING EDGE
	
	EIMSK |= 0x8;						//needs to changed
}//IN - FOR METAL DETECTION

void StepperHome(){
	int delay = 20;
	while (!stepperHome)
	{
		coilCount++;					//powers the next coil (clockwise) from the last one that fired
		if (coilCount>3) coilCount=0;	//if the coil moves a full 4 steps, move it back to the first coil
		PORTA = step[coilCount];		//Send the signal to the motor driver on PORTA for each individual step
		timerCount(delay);
		if(countStep<=10) delay -= 1;
		countStep++;					//track where in the rotation the stepper is, referenced to when it was turned on (future will be reference to the hall sensor)
	}
}

void stepperPos(int pos){
	
	if(pos!=stepperPOS){
		PORTC=1;
		int movepos = stepperPOS - pos;
		stepperPOS = pos;
		PORTC|=2;
		stepperdifference = 0;
		if(movepos == 3) movepos = -1;
		if(movepos == -3) movepos = 1;

		if(movepos != 0){
			if(movepos > 0) {
				stepperdir=1;				//cclockwise
				posStepCount=50*movepos;
				stepperdifference=posStepCount;
			}
		
			if(movepos < 0){
				stepperdir=2;			//clockwise
				posStepCount=50*(abs(movepos));
				stepperdifference=posStepCount;
				PORTC = 0xff;
			}
			ocr3value = 2500;
			OCR3A=ocr3value;//initial speed
			stepperworking=1;
		}
	}
	
}
void timer3Setup(){
	 // set up timer with prescaler = 8 and CTC mode
	 TCCR3B |= (1 << WGM32)|(1 << CS31);
	 
	 // initialize counter
	 TCNT3 = 0;
	 
	 // initialize compare value
	 OCR3A = ocr3value;
	 
	 // enable compare interrupt
	 TIMSK3 |= (1 << OCIE3A);
}

ISR (TIMER3_COMPA_vect)
{
	if (stepperworking){
		
		if (stepperdir==1){		//clockwise
			coilCount++;
			if (coilCount>3) coilCount=0;
			PORTA = step[coilCount];
			countStep++;
		
			stepAccel = stepperdifference-posStepCount;
			stepDecel = stepperdifference - stepAccel;
			if(stepAccel<=3) ocr3value-=250;
			if(stepAccel>3 && stepAccel <= 9) ocr3value -=125;
			if(stepDecel>3 && stepDecel<=9)ocr3value +=125;
			if(stepDecel<=3)ocr3value +=250;
			posStepCount--;
			OCR3A=ocr3value;
	
		}//if - stepperdir-clockwise
		
		if (stepperdir==2){
			coilCount--;
			if (coilCount<0) coilCount=3;		
			PORTA = step[coilCount];
			countStep--;	
	
			stepAccel = stepperdifference-posStepCount;
			stepDecel = stepperdifference - stepAccel;
			if(stepAccel<=3) ocr3value-=250;
			if(stepAccel>3 && stepAccel <= 9) ocr3value -=125;
			if(stepDecel>3 && stepDecel<=9)ocr3value +=125;
			if(stepDecel<=3)ocr3value +=250;
			posStepCount--;
			OCR3A=ocr3value;

		}//if - stepper-cclockwise
		
		if (posStepCount<=0){
			stepperworking=0;//done working
			timerCount(2000);
			} 
			
		/*OCR3A=ocr3value;*/
		
	}//stepperworking

}

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

/*Interrupt 3: Hall Effect on PD3*/
ISR(INT3_vect){
	countStep=0;		//resest the counterstep to home;
	stepperHome=1;
}//ISR

ISR(BADISR_vect){
	while(1){
		PORTC=0x0F;
		timerCount(500);
		PORTC=0xF5;
		timerCount(500);
	}
}
