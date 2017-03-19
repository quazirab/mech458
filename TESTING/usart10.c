/*
 * usart.c
 *
 * Created: 3/18/2017 11:16:57 PM
 * Author : Quazi
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>


#define F_CPU 1000000
#define BAUD 9600
#define BRC ((F_CPU/16/BAUD)-1)
#define  TX_BUFFER_SIZE 128

char serialBuffer[TX_BUFFER_SIZE];
uint8_t serialReadPos = 0;
uint8_t serialWritePos = 0;

void appendSerial (char c);
void serialWrite(char c[]);

int main(void)
{
	UBRR1H = (BRC>>8);
	UBRR1L = BRC;
	
	UCSR1B = (1<<TXEN1)|(1<<TXCIE1);		//TX,TX vector 
	UCSR1C = (1<<UCSZ11)|(1<<UCSZ10);		//8 bit
	
	sei();
	
	serialWrite("Hello\n\r");
	
    /* Replace with your application code */
    while (1) 
    {
			
    }
}

void appendSerial(char c){
	serialBuffer[serialWritePos] = c;							//add the character to the buffer
	serialWritePos++;
	if(serialWritePos>=TX_BUFFER_SIZE) serialWritePos=0;		//if end move to the begining 
}

ISR(USART1_TX_vect){
	if(serialReadPos!=serialWritePos){							//if no data to read then move data 
		UDR1 = serialBuffer[serialReadPos];
		serialReadPos++;
		
		if(serialReadPos>=TX_BUFFER_SIZE){						//end of the array
			serialReadPos=0;
		}
	}
}

void serialWrite(char c[]){
	for(uint8_t i=0;i<strlen(c);i++){							//send messages
		appendSerial(c[i]);
	}
	if(UCSR1A&(1<<UDRE1)){										//make the tx ready to transfer data again
		UDR1 =0;
	}
}
