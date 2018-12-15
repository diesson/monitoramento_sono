#ifndef ESTACAO_H_
#define ESTACAO_H_

// AVR
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include "lib/avr_usart.h"
#include "lib/avr_extirq.h"
// LIB
#include "lib/bits.h"
#include "lib/avr_gpio.h"
#include "lib/avr_extirq.h"
#include "lib/avr_adc.h"
#include "lib/avr_timer.h"
#include "lib/softuart.h"

// Define
#define TEMPO_SLEEP  1000//30000
#define N_AMOSTRAS 10
#define VALOR_COMPARACAO 100

//typedef enum ctrl{OFF, ON, FALSE, TRUE, DESCENDO, SUBINDO} flag_t;

/* Definição dos estados */
typedef enum {
	TEMPERATURA,
	LUZ,
	RUIDO,
	ENVIO,
	SLEEP,
	NUM_STATES
} state_t;

/* Definição da estrutura mantenedora do vetor de estados */
typedef struct {
	state_t myState;
	void (*func)(void);
}fsm_t;

typedef struct{
	uint16_t temperatura;
	uint16_t umidade;
	uint16_t luz;
	uint32_t ruido;
}estacao_t;

// SM
void f_temperatura();
void f_luz();
void f_ruido();
void f_envio();
void f_sleep();
// Geral
void controleInit();

#endif /* ESTACAO_H_ */

