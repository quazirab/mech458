#include <stdlib.h>
#include <avr/io.h>

#include <util/delay_basic.h>
#include <avr/interrupt.h>

#define BAUD  9600
#define BRC ((1000000/16/BAUD)-1)

//PD3 = TX

int main(){	

  UBRROH = (BRC>>8);
  UBRROL = BRC;
  
  UCSR0B = (1<<TXEN0);
  UCSROC = (1<<UCSZ01)|(1<<UCSZ00);   //8bit
  
  while(1){
    UDR0 = '8';
  }
  

