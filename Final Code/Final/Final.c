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
#include "LinkedQueue.h" 	/* This is the attached header file, which cleans thi*/

const unsigned short Al_max = 350;
const unsigned short Steel_max = 800;
const unsigned short Steel_min = 400;
const unsigned short Black_min = 940;
const unsigned short white_max =930;
const unsigned short white_min = 900;
const uint8_t pwm = 128;


void setupADC();
void setupPWM();
void timerCount(int tim);
void breakVcc(int tim);
void setupInterrupt();
void clockwise(int clCount);
void cclockwise (int cclCount);
void stepperpos(int pos);
void StepperHome();
void timer2Setup();



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
volatile uint8_t countStep=0;									//Global variable to track the location of the stepper
volatile int8_t coilCount = 0;									//Global variable to track which coil was last used
volatile uint8_t Status_flag = 0;								//Stepper operation only when not 0;
volatile uint8_t stepper_flag=1;
char step[4] = {0x36,0x2e,0x2d,0x35};						//Drives step 1&2, 2&3,3&4,4&1; dual phase full stepping
link *newLink;								//cylinder pointer for the new node to be added on each input
link *tail;										//cylinder pointer for the tail of the queue
link *head;										//cylinder pointer for the head of the queue
link *rtnLink;								//cylinder pointer for returning the value of the node that has been dequeued
volatile uint8_t BELT_STATUS = 1;							//1-BELT MOVING, 0 FOR BELT STOP
volatile uint8_t stepperHome = 0;
volatile uint8_t num = 0;
volatile uint8_t stepperPOS = 1;			//value of the stepper position - /*1-black, 2-steel,3-white,4-alu*/
volatile uint8_t stopper = 0;				//for knowing how many parts have crossed EX
volatile uint8_t timer2overflow;							//Calculating overflow for displaying
volatile uint8_t alu;										//number of alu for displaying
volatile uint8_t steel;										//number of steel for displaying
volatile uint8_t black;										//number of black for displaying
volatile uint8_t white;										//number of white for displaying
volatile uint8_t displayCounter=1;							//counter for showing dispaying position -  /*1-black, 2-steel,3-white,4-alu*/
volatile int8_t posEX=1;											//for counting how many have crossed EX
						


/* main routine */
int main(){	
	DDRB = 0x8F; 	//SET PORT B LOWER 4 BITS TO OUTPUT, WITH PB4,PB5,PB6 TO INPUT, AND PB7 TO OUTPUT
	DDRC = 0xFF;	//SET ALL OF THE PORT C TO OUTPUT BITS
	DDRD = 0xF0;	//SET ALL OF THE PORT D TO OUTPUT BITS
	DDRF = 0x00;	//SET ALL OF PORT F TO INPUT BITS

	rtnLink = NULL;
	newLink = NULL;
	cli();				//Disable Global interrupts
	setupPWM();			//Run the PWM setup Function below
	setupADC();			//Run the ADC Setup Function below
	
	setupInterrupt();	//Sets up all the interrupts
	timer2Setup();		//Sets up timer 2 for EX and beltstop calibration
	sei();				//Enable global interrupts
	StepperHome();		//brings the stepper to position
	PORTB = dcDrive[1];
	
	setup(&head,&tail);
	
	while(1){
		
		
			
		if(Status_flag>0) {
					dequeue(&head,&tail,&rtnLink);
					stepper_flag=1;
					stepperpos(rtnLink->e.itemCode);
					stepper_flag=0;
					PORTB=dcDrive[1];
					timerCount(200);
					Status_flag--;
				}//if
				PORTC=num;
				//PORTB= dcDrive[1];
				
	}//while
	return(0);

}/* main */

/**************************************************************************************/
/*****************************Stepper Motor Home**********************************/
/**************************************************************************************/

void stepperpos(int pos){

	int movepos = pos - stepperPOS;
	if(movepos == 3) movepos = -1;
	if(movepos == -3) movepos = 1;
	if(movepos != 0){
		if(movepos > 0) cclockwise(movepos);
		if(movepos < 0) clockwise(abs(movepos));
	}
	stepperPOS = pos;
}


/**************************************************************************************/
/*****************************Stepper Motor Clockwise**********************************/
/**************************************************************************************/


void StepperHome(){
	int delay = 20;
	while (!stepperHome)
	{	
		coilCount++;					//powers the next coil (clockwise) from the last one that fired
		if (coilCount>3) coilCount=0;	//if the coil moves a full 4 steps, move it back to the first coil
		PORTA = step[coilCount];		//Send the signal to the motor driver on PORTA for each individual step
		timerCount(delay);
		if(coilCount<=13) delay=-1;
		if(delay < 13) delay =13;
		//countStep++;					//track where in the rotation the stepper is, referenced to when it was turned on (future will be reference to the hall sensor)
	}
}

/**************************************************************************************/
/*****************************Stepper Motor Clockwise**********************************/
/**************************************************************************************/
void clockwise(int pos){
	int delay = 20;
	int clCount = pos*50;
	for(int i=0;i<clCount;i++){
		coilCount++;					//powers the next coil (clockwise) from the last one that fired
		if (coilCount>3) coilCount=0;	//if the coil moves a full 4 steps, move it back to the first coil
		PORTA = step[coilCount];		//Send the signal to the motor driver on PORTA for each individual step
		timerCount(delay);
		if(i <= 12) delay -= 1;
		if(clCount -i <= 12) delay +=1;
		countStep++;					//track where in the rotation the stepper is, referenced to when it was turned on (future will be reference to the hall sensor)
		if(countStep>200)countStep=0;	//if the stepper rotates a full 360 degrees, reset to the initial value and start again
	}//for
	
}

/**************************************************************************************/
/*****************************Stepper Motor Counter Clockwise**************************/
/**************************************************************************************/
void cclockwise (int pos){
	int delay = 20;
	int cclCount = pos*50;
	for(int i=0;i<cclCount;i++){
		coilCount--;					//powers the previous coil (anti-clockwise) from the last one that fired
		if (coilCount<0) coilCount=3;	//if the coil moves a full 4 steps, move it back to the fourth coil in order to continue reversing (anti-clockwise)
		PORTA = step[coilCount];		//Send the signal to the motor driver on PORTA for each individual step
		timerCount(delay);
		if(i <= 12) delay -= 1;
		if(cclCount -i <= 12) delay +=1;
		countStep--;					//track where in the rotation the stepper is, referenced to when it was turned on (future will be reference to the hall sensor)
		if(countStep<0)countStep=200;	//if the stepper rotates a full 360 degrees, reset to the initial value and start again
	}//for
}


/**************************************************************************************/
/************************************Interrupt***********************************************/
/**************************************************************************************/
void setupInterrupt(){
	EICRA |= _BV(ISC01);				//OI - FOR INITIATING A LINK LIST - FALLING EDGE
	EICRA |= _BV(ISC10)|_BV(ISC11);		//OR - FOR ADC VALUE - Rising Edge
	EICRA |= _BV(ISC21);				//EX - STEPPER WAITER - FALLING EDGE
	EICRA |= _BV(ISC31);				//HE - RESESTS THE STEPPER COUNT - FALLING EDGE
	EIMSK |= 0x0F;						//needs to changed
}//IN - FOR METAL DETECTION

/*Interrupt 0: EX sensor on PD0*/
ISR(INT0_vect){		
										//OI
	

}//ISR

/*Interrupt 1: OR on PD1*/
ISR(INT1_vect){
	/*ADC VALUE*/
	OCR0A = 65;
	lowVal2=0x03FF;							//intial highest vale to 1023
	ADCSRA |= (1<<ADSC);					//start conversion, goes to ADC
	
}//ISR
/*Interrupt 1: EX on PD2*/
ISR (INT2_vect){
	/*WAIT AT THE END OF THE BELT*/
	//stops the belt if the
		if(stepper_flag==1) {
			PORTB=0;
		}
		posEX=1;
				
}
/*Interrupt 3: Hall Effect on PD3*/
ISR(INT3_vect){
	countStep=0;		//resest the counterstep to home;
	stepperHome=1;
	
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
	OCR0A = pwm;
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
		if((PIND & 0x02)==0x02) {
			ADCSRA |= (1<<ADSC);			//conversion continues till state ==0;
		}
		
			else{
				
				initLink(&newLink);							//Initialize the newLink
			
				/*1-black, 2-steel,3-white,4-alu*/
						if (lowVal2<Al_max){							//aluminum
								newLink->e.itemCode=4;
								alu++;
							}//if
		 				else if (lowVal2>Steel_min && lowVal2<Steel_max){//STEEL
							newLink->e.itemCode=2;
							steel++;
						}//if
						else if (lowVal2>Black_min){					//BLACK
							newLink->e.itemCode=1;
							black++;
							}//if
						else if (lowVal2<white_max && lowVal2>white_min  ){//WHITE
								newLink->e.itemCode=3;
								white++;
							}//if
						else {
							newLink->e.itemCode=3;
							white++;
						}
						
						//PORTC = lowVal2&0xFF;
						PORTD = (lowVal2>>8)<<5;
						enqueue(&head,&tail,&newLink);				//enqueues the new cylinder
						Status_flag++;
						
						//return;
		}//else
	}//ISR



/**************************************************************************************/
/***************************** SUBROUTINES ********************************************/
/**************************************************************************************/







/**************************************************************************************
* DESC: initializes the linked queue to 'NULL' status
* INPUT: the head and tail pointers by reference
*/

void setup(link **h,link **t){
	*h = NULL;		/* Point the head to NOTHING (NULL) */
	*t = NULL;		/* Point the tail to NOTHING (NULL) */
	return;
}/*setup*/




/**************************************************************************************
* DESC: This initializes a link and returns the pointer to the new link or NULL if error 
* INPUT: the head and tail pointers by reference
*/
void initLink(link **newLink){
	//link *l;
	*newLink = malloc(sizeof(link));
	(*newLink)->next = NULL;
	return;
}/*initLink*/




/****************************************************************************************
*  DESC: Accepts as input a new link by reference, and assigns the head and tail		
*  of the queue accordingly				
*  INPUT: the head and tail pointers, and a pointer to the new link that was created 
*/
/* will put an item at the tail of the queue */
void enqueue(link **h, link **t, link **nL){

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




/**************************************************************************************
* DESC : Removes the link from the head of the list and assigns it to deQueuedLink
* INPUT: The head and tail pointers, and a ptr 'deQueuedLink' 
* 		 which the removed link will be assigned to
*/
/* This will remove the link and element within the link from the head of the queue */
void dequeue(link **h,link **t, link **deQueuedLink){
	/* ENTER YOUR CODE HERE */
	*deQueuedLink = *h;	// Will set to NULL if Head points to NULL
	/* Ensure it is not an empty queue */
	if (*h != NULL){
		*h = (*h)->next;
	}/*if*/
	if (*h==NULL) *t=NULL;
	return;
}/*dequeue*/




/**************************************************************************************
* DESC: Peeks at the first element in the list
* INPUT: The head pointer
* RETURNS: The element contained within the queue
*/
/* This simply allows you to peek at the head element of the queue and returns a NULL pointer if empty */
element firstValue(link **h){
	return((*h)->e);
}/*firstValue*/





/**************************************************************************************
* DESC: deallocates (frees) all the memory consumed by the Queue
* INPUT: the pointers to the head and the tail
*/
/* This clears the queue */
void clearQueue(link **h, link **t){

	link *temp;

	while (*h != NULL){
		temp = *h;
		*h=(*h)->next;
		free(temp);
	}/*while*/
	
	/* Last but not least set the tail to NULL */
	*t = NULL;		

	return;
}/*clearQueue*/





/**************************************************************************************
* DESC: Checks to see whether the queue is empty or not
* INPUT: The head pointer
* RETURNS: 1:if the queue is empty, and 0:if the queue is NOT empty
*/
/* Check to see if the queue is empty */
char isEmpty(link **h){
	/* ENTER YOUR CODE HERE */
	return(*h == NULL);
}/*isEmpty*/





/**************************************************************************************
* DESC: Obtains the number of links in the queue
* INPUT: The head and tail pointer
* RETURNS: An integer with the number of links in the queue
*/
/* returns the size of the queue*/
int size(link **h, link **t){

	link 	*temp;			/* will store the link while traversing the queue */
	int 	numElements;

	numElements = 0;

	temp = *h;			/* point to the first item in the list */

	while(temp != NULL){
		numElements++;
		temp = temp->next;
	}/*while*/
	
	return(numElements);
}/*size*/


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


/**************************************************************************************/
/*************************Timer for EX and Belt Calibration****************************/
/**************************************************************************************/

void timer2Setup(){
	// set up timer with prescaler = 1024
	TCCR2B |= (1 << CS22)|(1 << CS20);
	
	// enable overflow interrupt
	TIMSK2 |= (1 << TOIE2);	
}

ISR(TIMER2_OVF_vect){
// 	timer2overflow++;		//overflows everytime it TCNT hits 256;
// 	if(timer2overflow >=4){		//every 1 secs
// 		
// 		if(displayCounter ==1){	//for black
// 			PORTC = black;
// 			PORTC|=(0b0001<<4);
// 			displayCounter++;
// 		}//if
// 		else if(displayCounter ==2){	//for steel
// 			PORTC = steel;
// 			PORTC|=(0b0010<<4);
// 			displayCounter++;
// 		}//if
// 		else if(displayCounter ==3){	//for white
// 			PORTC = white;
// 			PORTC|=(0b0100<<4);
// 			displayCounter++;
// 		}//if
// 		else if(displayCounter ==4){	//for alu
// 			PORTC = alu;
// 			PORTC|=(0b1000<<4);
// 			displayCounter=1;
// 		}//if
// 		
// 		TCNT2 = 46;		//start new timer
// 		timer2overflow=0;
// 	}//if
}//isr

