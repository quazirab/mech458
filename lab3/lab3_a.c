/* Solution Set for the LinkedQueue.c */
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
#include "LinkedQueue.h" 	/* This is the attached header file, which cleans things up */
#include <util/delay_basic.h>
#include<avr/interrupt.h>

/* Make sure you read it!!! */
/* global variables */
/* Avoid using these */

/* main routine */
	void timerCount();

		
int main(){	

	link *newLink;			//Link pointer for the new node to be added on each input
	link *head;				//Link pointer for the head of the queue
	link *tail;				//Link pointer for the tail of the queue
	link *rtnLink;			//Link pointer for returning the value of the node that has been dequeued
	int shiftCount = 0;		//Integer counter for shifting the bit display on the LED bank
	char disp;				//char variable to temporarily store the value in e.itemCode before displaying it

	DDRA = 0x00;			//SET ALL OF THE PORT A TO INPUT BITS
  	DDRC = 0xFF;			//SET ALL OF THE PORT B TO OUTPUT BITS

	while(1){
		PORTC = 0;				//Set PORTC to all 0
		shiftCount = 0;			//Re-set the bit-shift counter to 
		
		setup(&head, &tail);	//Initialise the linked queue
		
		for(int i = 0; i<=3; i++){			//Loop through for 4 presses of the button
			
			 
			while((PINA & 0x04) == 0x04);	//Wait for the button to be pressed
			timerCount(20);					//Debounce by waiting for 20ms
			while((PINA & 0x04) == 0x00);	//Wait for the button to be released
			timerCount(20);
			
			initLink(&newLink);
			
			newLink->e.itemCode =  (PINA & 0x03); //ONLY THE INPUT BITS ARE READ
			enqueue(&head, &tail, &newLink);
			
		}//for - Input Loop
		dequeue(&head, &rtnLink);
		free(rtnLink);
		
		for(int j = 0; j<3; j++){
			dequeue(&head, &rtnLink);				//Extract the head of the list and set it to 'rtnLink'

			disp = rtnLink->e.itemCode;			//Set the value of the head of the lists 'itemCode' variable to the 'disp' element
			PORTC |= (disp << shiftCount);		//Set PORTC to display the value in 'disp' according to how many bits are being shifted, while keeping any previously displayed results
			timerCount(2000);
			shiftCount += 2;					//Increment shiftCount by 2, causing the display line above to shift the output by 2 bits to the left on the next loop.
			free(rtnLink);							//Free the memory occupied by 'rtnLink'

		}//for - Display loop
		
		while((PINA & 0x04) == 0x04);
		timerCount(20);
		while((PINA & 0x04) == 0x00);
		timerCount(20);
	
	}//while - Main loop
return(0);
}/* main */
		
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
	TIMSK1 = TIMSK1|0x02;
	
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
void dequeue(link **h, link **deQueuedLink){
	/* ENTER YOUR CODE HERE */
	*deQueuedLink = *h;	// Will set to NULL if Head points to NULL
	/* Ensure it is not an empty queue */
	if (*h != NULL){
		*h = (*h)->next;	//Set *h to be the next node in the linked list
	}/*if*/
	
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
