#include <stdlib.h>
#include <avr/io.h>

#include <util/delay_basic.h>
#include <avr/interrupt.h>

#define BAUD  9600
#define BRC ((1000000/16/BAUD)-1)

void timerCount(int tim);

//PD3 = TX

int main(){
	DDRC=0xFF; 
	UBRR1H = (BRC>>8);
	UBRR1L = BRC;
	
	UCSR1B = (1<<TXEN1);
	UCSR1C = (1<<UCSZ11)|(1<<UCSZ10);   //8bit
	
	while(1){
		while ( !( UCSR1A & (1<<UDRE1)) )
		PORTC = 8;
		UDR1 = '8';
		timerCount(2000);
	}
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
