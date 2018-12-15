#ifndef MONITOR_H_
#define MONITOR_H_

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
#include "oximetro.h"
#include "accelerometer.h"
#include "temperatura.h"

// Define
#define P_VERM PB0
#define P_INFV PB1
#define PA_VERM PB2
#define PA_INFV PB3
#define P_ganhoA0 PD4
#define P_ganhoA1 PD5
#define P_ganhoA2 PD6
#define P_ganhoA3 PD7

#define CTRL_LED GPIO_B
#define CTRL_PGA GPIO_D
#define TEMPO_SLEEP  1000//1//1000//30000
#define N_AMOSTRAS 10
#define VALOR_COMPARACAO 100

/* Definição dos estados */
typedef enum {
	VERMELHO,
	INFRAV,
	TEMPERATURA,
	PROCESSAMENTO,
	ENVIO,
	SLEEP,
	MOVIMENTO,
	NUM_STATES
} state_t;

/* Definição da estrutura mantenedora do vetor de estados */
typedef struct {
	state_t myState;
	void (*func)(void);
}fsm_t;

typedef struct {
	uint16_t I_max;
	uint16_t I_min;
	uint16_t I_med;
	uint16_t R_max;
	uint16_t R_min;
	uint16_t R_med;
	uint8_t batimento;
}oximetro_t;

typedef struct {
	uint16_t pos_x;
	uint16_t pos_y;
	uint16_t pos_z;
	uint8_t movimento;
}acelerometro_t;

// SM
void f_vermelho();
void f_infrav();
void f_temperatura();
void f_processamento();
void f_envio();
void f_sleep();
void f_movimento();
// Geral
void controleInit();

#endif /* MONITOR_H_ */
