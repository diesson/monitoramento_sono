
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

// AVR
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/interrupt.h>
// LIB
#include "estacao.h"
#include "lib/avr_usart.h"
#include "dht22.h"
#include "lib/softuart.h"

extern volatile state_t curr_state;
extern fsm_t myFSM[];

int main(){

	FILE *usart = get_usart_stream();
	USART_Init(B9600);

	softuart_init();
	softuart_turn_rx_on(); /* redundant - on by default */
	controleInit();
	sei();

	fprintf(usart, "batimentos; temp. corporal; acelerometro; oximetro; temp. ambiente; umidade; luz; ruido; hora;\n");

	_delay_ms(100);

	while(1){

		(*myFSM[curr_state].func)();

		_delay_ms(1000);

	}

	return 0;

}
