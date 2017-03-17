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

typedef struct cylinder{
	uint16_t  ADC_value; 	/* Stores for ADC Value */
	uint8_t  metal_value; 	/* 0: plastic, 1: metal */
	uint8_t  number;
	struct cylinder *next;
} cylinder;

typedef struct stepper{
	uint8_t  pos;		/*position number*/
	struct stepper *next;
} stepper;

void setupADC();
void setupPWM();
void timerCount(int tim);
void breakVcc(int tim);
void setupInterrupt();
void clockwise(int clCount);
void cclockwise (int cclCount);
void initcylinder(volatile cylinder **newcylinder);
void initstepper(volatile stepper **newstepper);
void enqueue(volatile cylinder **h,volatile cylinder **t,volatile cylinder **nL);
void stepperenqueue(volatile stepper **h,volatile stepper **t,volatile stepper **nL);
void dequeue(volatile cylinder **h,volatile cylinder **t,volatile cylinder **deQueuedcylinder);
void stepperdequeue(volatile stepper **h,volatile stepper **t,volatile stepper **deQueuedstepper);

/*Usage of pins in this program*/
//PORTC for LEDs
//PF1 for ADC input 
//PB7 for PWM output 
//PB5 for Direction Switch
//PB0 TO EB
//PB1 TO EA
//PB2 TO IB
//PB3 TO IA
//PB0 = L4
//PB1 = L4
//PB2 = E2
//PB3 = L2
//PB4 = L1
//PB5 = E1


volatile unsigned char ADClowerbit;
volatile unsigned char ADChigherbit;						//Global variable for storing the current ADC conversion value
volatile uint8_t Status_flag = 0;							//Global variable for the Status flag for newcylinder results found or not
const char dcDrive[4] = {0x00,0x04,0x08,0x0c};				//Array containing the DC motor driver truth table
volatile uint8_t STATE=0;
volatile unsigned short lowVal1;
volatile unsigned short lowVal2;							//short is 16 bit(2byte)
volatile int countStep=0;									//Global variable to track the location of the stepper
volatile int coilCount = 0;									//Global variable to track which coil was last used
char step[4] = {0x36,0x2e,0x2d,0x35};						//Drives step 1&2, 2&3,3&4,4&1; dual phase full stepping
cylinder *newcylinder =NULL;								//cylinder pointer for the new node to be added on each input
cylinder *tail = NULL;										//cylinder pointer for the tail of the queue
cylinder *head = NULL;										//cylinder pointer for the head of the queue
cylinder *rtncylinder = NULL;								//cylinder pointer for returning the value of the node that has been dequeued
stepper *newstepper = NULL;
stepper *Stail = NULL;
stepper *Shead = NULL;
stepper *rtnstepper = NULL;
volatile uint8_t metalState = 0;							//sees if the cylinder is about to enter the LE or not
volatile uint8_t metal=0;
volatile uint8_t number = 0;							//number in the positon
volatile uint8_t pos = 0;								//value of the stepper position - 1 - white, 2 - black, 3 - steel , 4 - alu


int main(){

	DDRB = 0b10001111; 	//SET PORT B LOWER 4 BITS TO OUTPUT, WITH PB4,PB5,PB6 TO INPUT, AND PB7 TO OUTPUT
	DDRC = 0xFF;;	//SET ALL OF THE PORT C TO OUTPUT BITS
	DDRD = 0xF0;	//SET ALL OF THE PORT D TO OUTPUT BITS
	DDRF = 0x00;	//SET ALL OF PORT F TO INPUT BITS
	
	
	cli();				//Disable Global interrupts
	setupADC();			//Run the ADC Setup Subroutine below
	setupPWM();			//Run the PWM setup Subroutine below
	setupInterrupt();	//Sets up all the interrupts
	
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
	EICRA |= _BV(ISC00);				//sets to any edges
	EICRA |= _BV(ISC11);				//IN sensor triggers on falling edge
	EICRA |= _BV(ISC31);				//First optical sensor triggers on 
	EIMSK |= 0x01;						//needs to changed
}

/*Interrupt 0: RL sensor on PD0*/
ISR(INT0_vect){
			
	if(STATE==0){
		STATE=1;						//changes the STATE to 1
		lowVal2=0x03FF;					//initial highest vale to 1023
		ADCSRA |= (1<<ADSC);			//start conversion, goes to ADC			
		}//if
	
	else {
		STATE=0;						//next edge
	}//else
	
}//ISR

/*Interrupt 1: inductive sensor on PD1*/
ISR(INT1_vect){
	/*Record if it is a metal. TRUE/FALSE*/
	metal = 1;
}//ISR

/*Interrupt 3: First optical sensor on PD3*/
ISR(INT3_vect){
	/*Initialize new list element when the first optical sensor is triggered.*/
	initcylinder(&newcylinder);
	newcylinder->number = number;				//puts the position number of the cylinder in the newcylinder
	enqueue(&head,&tail, &newcylinder);			//enqueues the new cylinder
	number++;
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

ISR(ADC_vect){
	lowVal1 = ADC;			//puts the full 10 bit value to lowVal1	
	if(lowVal2>lowVal1){		//if lowVal2 is greater than lowVal1
		lowVal2=lowVal1;	//lowVal2 is equal to lowVal1 
	}//if
	if(STATE==1) ADCSRA |= (1<<ADSC);        //Set the ADC Start Conversion bit on ADCSRA to 1.;	//conversion continues till state ==0;
	else {
		if (head->ADC_value=NULL) {		//checks if the head has any ADC value or not
			head->ADC_value=lowVal2;	//assigns the ADC value
			dequeue(&head,&tail,&rtncylinder);
			switch (rtncylinder->metal_value){
				case '1' : 
					if (rtncylinder->ADC_value>100 && rtncylinder->ADC_value<150) pos = 1;
					else pos =3;
					break;
					
					free(rtncylinder); 
				}//switch
			newstepper->pos=pos;	
			stepperenqueue(&Shead, &Stail, &newstepper);
			}//if
		else if (head->next->ADC_value=NULL){	//if the head has any ADC value, checks the next one
			head->next->next->ADC_value=lowVal2; //assigns the ADC value to the next null one
			
		}//if
		else if (head->next->next->ADC_value=NULL){	//if the head has any ADC value, checks the next one
			head->next->next->ADC_value=lowVal2;	//assigns the ADC value to the next null one
			
		}//if
		else if (head->next->next->next->ADC_value=NULL){	//if the head has any ADC value, checks the next one
			head->next->next->next->next->ADC_value=lowVal2;	//assigns the ADC value to the next null one
		}//if
	}//else
}//ISR


/**************************************************************************************/
/*************************************LinkList*********************************************/
/**************************************************************************************/

/**************************************************************************************
* DESC: This initializes a cylinder and returns the pointer to the new cylinder or NULL if error 
* INPUT: the head and tail pointers by reference
*/
void initcylinder(volatile cylinder **newcylinder){
	//cylinder *l;
	*newcylinder = malloc(sizeof(cylinder));
	(*newcylinder)->next = NULL;
	return;
}/*initcylinder*/

void initstepper(volatile stepper **newstepper){
	//cylinder *l;
	*newstepper = malloc(sizeof(stepper));
	(*newstepper)->next = NULL;
	return;
	}/*initstepper*/



/****************************************************************************************
*  DESC: Accepts as input a new cylinder by reference, and assigns the head and tail		
*  of the queue accordingly				
*  INPUT: the head and tail pointers, and a pointer to the new cylinder that was created 
*/
/* will put an item at the tail of the queue */
void enqueue(volatile cylinder **h,volatile cylinder **t,volatile cylinder **nL){

	if (*t != NULL){
		/* Not an empty queue */
		(*t)->next = *nL;
		*t = *nL; //(*t)->next;
	}/*if*/
	else{
		/* It's an empty Queue */
		//(*h)->next = *nL;
		//should be this
		*h = *nL;
		*t = *nL;
	}/* else */
	return;
}/*enqueue*/


void stepperenqueue(volatile stepper **h,volatile stepper **t,volatile stepper **nL){

	if (*t != NULL){
		/* Not an empty queue */
		(*t)->next = *nL;
		*t = *nL; //(*t)->next;
		}/*if*/
		else{
			/* It's an empty Queue */
			//(*h)->next = *nL;
			//should be this
			*h = *nL;
			*t = *nL;
			}/* else */
			return;
}/*stepperenqueue*/





/**************************************************************************************
* DESC : Removes the cylinder from the head of the list and assigns it to deQueuedcylinder
* INPUT: The head and tail pointers, and a ptr 'deQueuedcylinder' 
* 		 which the removed cylinder will be assigned to
*/
/* This will remove the cylinder and element within the cylinder from the head of the queue */
void dequeue(volatile cylinder **h,volatile cylinder **t,volatile cylinder **deQueuedcylinder){
	/* ENTER YOUR CODE HERE */
	*deQueuedcylinder = *h;	// Will set to NULL if Head points to NULL
	/* Ensure it is not an empty queue */
	if (*h != NULL){
		*h = (*h)->next;	//Set *h to be the next node in the cylindered list
	}/*if*/
	if (*h==NULL)*t=NULL;
	
	return;
}/*dequeue*/


void stepperdequeue(volatile stepper **h,volatile  stepper **t,volatile stepper **deQueuedstepper){
	/* ENTER YOUR CODE HERE */
	*deQueuedstepper = *h;	// Will set to NULL if Head points to NULL
	/* Ensure it is not an empty queue */
	if (*h != NULL){
		*h = (*h)->next;	//Set *h to be the next node in the cylindered list
		}/*if*/
		if (*h==NULL)*t=NULL;
		
		return;
}/*stepperdequeue*/
