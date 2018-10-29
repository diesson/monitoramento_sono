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

// Define
#define P_VERM PB0
#define P_INFV PB1
#define PA_VERM PB2
#define PA_INFV PB3
#define P_ganhoA PD6
#define P_ganhoB PD7

#define CTRL_LED GPIO_B
#define TEMPO_SLEEP  1000//30000
#define N_AMOSTRAS 10

typedef enum ctrl{OFF, ON, FALSE, TRUE, DESCENDO, SUBINDO} flag_t;
typedef enum {
	RAC,
	IAC,
	RDC,
	IDC,
	TEMP,
	TAM_ADC
} leitura_t;

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
/*
 * ADMUX
 *
 * REFS [Reference Selection Bits] - Seleciona a tensão de referencia do ADC
 * ADLAR [ADC Left Adjust Result] - Configuracao do ADCH e ADCL (Registradores com as informacoes)
 * MUX [Analog Channel Selection Bits] - Selecionar a entrada analogica conctada ao ADC
 *
 *
 * ADCSRA
 *
 * ADEN [ADC Enable] - Habilitar ADC
 * ADSC [ADC Start Conversion] - Ativar esse bit para comecar conversao
 * ADATE [ADC Auto Trigger Enable] -
 * ADIF [ADC Interrupt Flag] - Flag de saida
 * ADIE [ADC Interrupt Enable] - Ativa a interrupcao de conversao AD
 * ADPS [ADC Prescaler Select Bits] - Prescaler
 */
