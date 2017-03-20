/*
 * usart.c
 *
 * Created: 3/18/2017 11:16:57 PM
 * Author : Quazi
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>


#define F_CPU 1000000
#define BAUD 4800
#define BRC ((F_CPU/16/BAUD)-1)

volatile int serialReadPos=0;

char serialBuffer[5] = {'1','2','3','4','\n\r'};



int main(void)
{
	UBRR1H = (BRC>>8);
	UBRR1L = BRC;
	
	UCSR1A = (1<<U2X1);						//doubling the Transfer from 4800 to 9600
	UCSR1B = (1<<TXEN1)|(1<<TXCIE1);		//TX,TX vector 
	UCSR1C = (1<<UCSZ11)|(1<<UCSZ10);		//8 bit
	
	sei();

	
    /* Replace with your application code */
    while (1) 
    {
			
    }
}


ISR(USART1_TX_vect){
		
	UDR1 = serialBuffer[serialReadPos];
	serialReadPos++;
	if (serialReadPos>=5) serialReadPos=0;
	
}


