#ifndef _ONE_WIRE_H
#define _ONE_WIRE_H

#include <avr/io.h>
#include "bits.h"

//Pinos de hardware
#define DDRx	DDRB	//define o DDR do pino DQ
#define PORTx	PORTB	//define o PORT do pino DQ
#define PINx	PINB	//define o PIN do pino DQ
#define DQ		PB0		//pino para a comunicacao

//funcoes publicas
uint8_t reset_1w();
void power_1w();
uint8_t read_byte_1w();
void write_byte_1w(uint8_t dado);

#endif
