
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

// AVR
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/interrupt.h>
// LIB
#include "monitor.h"
#include "lib/avr_usart.h"

extern volatile state_t curr_state;
extern fsm_t myFSM[];

int main(){

	//FILE *usart = get_usart_stream();
	USART_Init(B9600);

	controleInit();
	sei();
	//fprintf(usart, "USART\n\r");
	i2c_init();
	adxl345_init();
	bmp085_init();
	//fprintf(usart, "I2C\n\r");

	_delay_ms(100);

	while(1){

		(*myFSM[curr_state].func)();
		//_delay_ms(20);

	}

	return 0;

}
