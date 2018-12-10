/*
 * funcoes.h
 *
 *  Created on: 9 de dez de 2018
 *      Author: diesson
 */

#ifndef FUNCOES_H_
#define FUNCOES_H_

// AVR
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/sleep.h>
// LIB
#include "lib/bits.h"
#include "lib/avr_gpio.h"
#include "lib/avr_extirq.h"
#include "lib/avr_adc.h"
#include "lib/avr_timer.h"
#include "estacao.h"

typedef enum ctrl{OFF, ON, FALSE, TRUE, DESCENDO, SUBINDO} flag_t;

typedef struct {
	flag_t timer2_status;		 	//flag_t timer_amost = OFF;
	uint8_t timer2_tempo; 			//volatile uint8_t contTimer = 0;
	uint16_t timer1_tempo; 			//volatile uint16_t acordar = TEMPO_SLEEP;
}timer_t;

void timerOn(uint8_t t_ms);
void timerOff(void);
void timerWait(void);
void adcOn(uint8_t op);
void adcOff();
void timer01State(flag_t flag);

#endif /* FUNCOES_H_ */
