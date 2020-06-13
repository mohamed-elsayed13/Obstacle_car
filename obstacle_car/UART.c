/*
 * uart.c
 *
 * Created: 12/7/2014 5:10:39 PM
 *  Author: safifi
 */ 
#include "UART.h"
void Uart_init(){
	// baud rate
	UBRRL = 103;// baud rate 9600, F_CPU = 16Mhz

	/* Enable receiver and transmitter */
	UCSRB = (1<<RXEN) | (1<<TXEN)|(1<<RXCIE); 
	UCSRC = (1<<URSEL)| (1<<UCSZ1)|(1<<UCSZ0); // 8 bit data transfer
}

void Uart_Write(uint8_t data){
	/* Wait for empty transmit buffer */
	while ( !( UCSRA & (1<<UDRE)) );  // check trans
	UDR = data;
	
}

uint8_t Uart_Read(){
	while ( !(UCSRA & (1<<RXC)) );
	return UDR;
}
void Uart_Write_string(uint8_t* str){
	uint8_t i=0;
	while (str[i]!=0){
		Uart_Write(str[i]);
		i++;
	}
}