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

const uint16_t Al_max = 128;
const uint16_t Steel_max = 800;
const uint16_t Steel_min = 600;
const uint16_t Black_min = 935;
const uint16_t white_max =930;
const uint16_t whie_min = 900;
const uint8_t pwm = 160;

typedef struct {
	uint8_t  pos;
	uint8_t number;
} part;

typedef struct cylinder{
	part		p;
	struct cylinder *next;
} cylinder;


void setupADC();
void setupPWM();
void timerCount(int tim);
void breakVcc(int tim);
void setupInterrupt();
void clockwise(int clCount);
void cclockwise (int cclCount);
void initcylinder( cylinder **newcylinder);
void enqueue(cylinder **h, cylinder **t,cylinder **nL);
void dequeue( cylinder **h, cylinder **t, cylinder **deQueuedcylinder);
void stepperpos(uint8_t pos);
void StepperHome();


/*Usage of pins in this program*/
//PORTC for LEDs
//PF1 = RL 
//PB7 = PWM 
//PB5 for Direction Switch
//PB0 = EB
//PB1 = EA
//PB2 = IB
//PB3 = IA
//PB0 = L4
//PB1 = L4
//PB2 = E2
//PB3 = L2
//PB4 = L1
//PB5 = E1
//PD0 = OI
//PD1 = IN
//PD2 = OR
//PD3 = EX

const char dcDrive[4] = {0x00,0x04,0x08,0x0c};				//Array containing the DC motor driver truth table
volatile unsigned short lowVal1;
volatile unsigned short lowVal2;							//short is 16 bit(2byte)
volatile int countStep=0;									//Global variable to track the location of the stepper
volatile int coilCount = 0;									//Global variable to track which coil was last used
volatile int Status_flag = 0;								//Stepper operation only when not 0;
volatile int Status_flag
char step[4] = {0x36,0x2e,0x2d,0x35};						//Drives step 1&2, 2&3,3&4,4&1; dual phase full stepping
cylinder *newcylinder =NULL;								//cylinder pointer for the new node to be added on each input
cylinder *tail = NULL;										//cylinder pointer for the tail of the queue
cylinder *head = NULL;										//cylinder pointer for the head of the queue
cylinder *rtncylinder = NULL;								//cylinder pointer for returning the value of the node that has been dequeued
volatile uint8_t BELT_STATUS = 1;							//1-BELT MOVING, 0 FOR BELT STOP


//volatile uint8_t pos = 0;								//value of the stepper position - 1 - white, 2 - black, 3 - steel , 4 - alu


int main(){

	DDRB = 0x0F; 	//SET PORT B LOWER 4 BITS TO OUTPUT, WITH PB4,PB5,PB6 TO INPUT, AND PB7 TO OUTPUT
	DDRC = 0xFF;	//SET ALL OF THE PORT C TO OUTPUT BITS
	DDRD = 0xF0;	//SET ALL OF THE PORT D TO OUTPUT BITS
	DDRF = 0x00;	//SET ALL OF PORT F TO INPUT BITS
	
	
	cli();				//Disable Global interrupts
	setupADC();			//Run the ADC Setup Function below
	setupPWM();			//Run the PWM setup Function below
	setupInterrupt();	//Sets up all the interrupts
	sei();				//Enable global interrupts
	StepperHome();		//brings the stepper to position

	while(1){
		
	if ((PINB & 0x10) == 0x10){
		if (BELT_STATUS) PORTB = dcDrive[1];
		else PORTB = 0;
		}//if
	else{
		PORTB = 0;
		}//if
	if(Status_flag){
			dequeue(&head,&tail, &rtncylinder);
			stepperpos(rtncylinder->p.pos);
			Status_flag--;
			if(Status_flag<0) Status_flag=0;
		}//if	 
	}//while
	return(0);
}/* main */


/**************************************************************************************/
/*****************************Stepper Motor Clockwise**********************************/
/**************************************************************************************/
void stepperpos(uint8_t pos){
		clockwise(50*pos);
}

/**************************************************************************************/
/*****************************Stepper Motor Clockwise**********************************/
/**************************************************************************************/
void clockwise(int clCount){
	int delay = 20;
	
	for(int i=0;i<clCount;i++){
		coilCount++;					//powers the next coil (clockwise) from the last one that fired
		if (coilCount>3) coilCount=0;	//if the coil moves a full 4 steps, move it back to the first coil
		PORTA = step[coilCount];		//Send the signal to the motor driver on PORTA for each individual step
		timerCount(delay);
		if(i <= 13) delay -= 1;
		if(clCount -i <= 13) delay +=1;
		countStep++;					//track where in the rotation the stepper is, referenced to when it was turned on (future will be reference to the hall sensor)
		if(countStep>200)countStep=0;	//if the stepper rotates a full 360 degrees, reset to the initial value and start again
	}//for
		
}

/**************************************************************************************/
/*****************************Stepper Motor Counter Clockwise**************************/
/**************************************************************************************/
void cclockwise (int cclCount){
	int delay = 20;
	for(int i=0;i<cclCount;i++){
		coilCount--;					//powers the previous coil (anti-clockwise) from the last one that fired
		if (coilCount<0) coilCount=3;	//if the coil moves a full 4 steps, move it back to the fourth coil in order to continue reversing (anti-clockwise)
		PORTA = step[coilCount];		//Send the signal to the motor driver on PORTA for each individual step
		timerCount(delay);
		if(i <= 13) delay -= 1;
		if(cclCount -i <= 13) delay +=1;
		countStep--;					//track where in the rotation the stepper is, referenced to when it was turned on (future will be reference to the hall sensor)
		if(countStep<0)countStep=200;	//if the stepper rotates a full 360 degrees, reset to the initial value and start again
	}//for
}


/**************************************************************************************/
/************************************Interrupt***********************************************/
/**************************************************************************************/
void setupInterrupt(){
	EICRA |= _BV(ISC00)|_BV(ISC01);		//OI - FOR INITIATING A LINK LIST - RISING EDGE
	EICRA |= _BV(ISC10);				//OR - FOR ADC VALUE - ANY EDGE
	EICRA |= _BV(ISC21);				//EX - STEPPER WAITER - FALLING EDGE
	EICRA |= _BV(ISC31);				//HE - RESESTS THE STEPPER COUNT - FALLING EDGE
	EIMSK |= 0x0F;						//needs to changed
}//IN - FOR METAL DETECTION

/*Interrupt 0: OI sensor on PD0*/
ISR(INT0_vect){											//OI 
	/*Initialize new list element when the first optical sensor is triggered.*/
// 		initcylinder(&newcylinder);
// 		newcylinder->p.number = number;				//puts the position number of the cylinder in the newcylinder
// 		enqueue(&head,&tail, &newcylinder);			//enqueues the new cylinder
// 		number++;
}//ISR

/*Interrupt 1: OR on PD1*/
ISR(INT1_vect){
	/*ADC VALUE*/
	lowVal2=0x03FF;							//intial highest vale to 1023
	ADCSRA |= (1<<ADSC);					//start conversion, goes to ADC	
}//ISR
/*Interrupt 1: OR on PD1*/
ISR (INT2_vect){
	/*WAIT AT THE END OF THE BELT*/
	if (BELT_STATUS==1)BELT_STATUS=0;		//stops the belt if the 
	else BELT_STATUS =1;
}
/*Interrupt 3: First optical sensor on PD3*/
ISR(INT3_vect){
	
}//ISR



ISR(BADISR_vect){
	/*Error handler?*/
}//BADISR
/**************************************************************************************/
/************************************PWM***********************************************/
/**************************************************************************************/
void setupPWM(){
	TCCR0A =(1<<COM0A1) | (1<<WGM01) | (1<<WGM00);	//Set timer counter compare register to Fast PWM
	TCCR0B = (1<<CS01);								//prescale to 9
	OCR0A = 160;		
}//setupPWM


/**************************************************************************************/
/*************************************ADC**********************************************/
/**************************************************************************************/
void setupADC(){
	ADMUX = (1<< REFS0)|(1<<MUX0);						// Left adjust, and use VCC as top reference
	ADCSRA=(1<<ADEN)|(1<<ADIE);							//Set the values of the ADC Enable and ADC Interrupt Enable bits to 1
	DIDR0 = (1<<ADC1D);									//Turns off the digital input buffer for ADC1 on PF1
}/*setupADC*/

ISR(ADC_vect){
	lowVal1 = ADC;										//puts the full 10 bit value to lowVal1
	if(lowVal2>lowVal1){								//if lowVal2 is greater than lowVal1
		lowVal2=lowVal1;								//lowVal2 is equal to lowVal1
	}//if
	if((PIND & 0x01) == 0x01) ADCSRA |= (1<<ADSC);			//conversion continues till state ==0;
	else{
		initcylinder(&newcylinder);							//Initialize the newcylinder
			/*1 - white, 2 - black, 3 - steel , 4 - alu*/
			if (lowVal2<Al_max){							//aluminum
					newcylinder->p.pos=4;	
				}//if
			else if (lowVal2>Steel_min && lowVal2<Steel_max){//STEEL
					newcylinder->p.pos=3;
				}//if
			else if (lowVal2>Black_min){					//BLACK
					newcylinder->p.pos=2;
				}//if
			
			else if (lowVal2<white_max && lowVal2>whie_min){//WHITE
					newcylinder->p.pos=1;
				}//if
			else PORTC = 0XFF;	
			enqueue(&head,&tail, &newcylinder);				//enqueues the new cylinder
			Status_flag ++;								// 1 - turns on stepper
		}//if

	}//else
}//ISR


/**************************************************************************************/
/*************************************LinkList*********************************************/
/**************************************************************************************/

void initcylinder( cylinder **newcylinder){
	//cylinder *l;
	*newcylinder = malloc(sizeof(cylinder));
	(*newcylinder)->next = NULL;
	return;
}/*initcylinder*/

void enqueue( cylinder **h, cylinder **t, cylinder **nL){

	if (*t != NULL){
		(*t)->next = *nL;
		*t = *nL; 
	}/*if*/
	else{
	
		*t = *nL;
	}/* else */
	return;
}/*enqueue*/

void dequeue( cylinder **h, cylinder **t, cylinder **deQueuedcylinder){
	/* ENTER YOUR CODE HERE */
	*deQueuedcylinder = *h;	// Will set to NULL if Head points to NULL
	/* Ensure it is not an empty queue */
	if (*h != NULL){
		*h = (*h)->next;	//Set *h to be the next node in the cylindered list
	}/*if*/
	if (*h==NULL)*t=NULL;
	
	return;
}/*dequeue*/

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
