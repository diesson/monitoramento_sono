#ifndef OXIMETRO_H_
#define OXIMETRO_H_

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
#include "monitor.h"

#define N_TESTE 5

typedef enum ctrl{OFF, ON, FALSE, TRUE, DESCENDO, SUBINDO} flag_t;
typedef enum pga{PGA_IRED, PGA_RED, PGA_RESET} pga_t;

typedef enum {
	RAC,
	IAC,
	RDC,
	IDC,
	TEMP,
	TAM_ADC
} leitura_t;

typedef struct {
	flag_t timer0_status;		 	//flag_t timer_amost = OFF;
	uint8_t timer0_tempo; 			//volatile uint8_t contTimer = 0;
	uint16_t timer1_tempo; 			//volatile uint16_t acordar = TEMPO_SLEEP;
}timer_t;

uint8_t detectar_maior(uint16_t* vetor, uint8_t n);
uint8_t detectar_menor(uint16_t* vetor, uint8_t n);
flag_t batimentos(flag_t status);
flag_t oximetroInclinacao(uint16_t atual, uint16_t anterior, flag_t status);
uint8_t setControlePGA(pga_t pga, uint8_t op);
uint16_t getControlePga(uint16_t maximo, pga_t pga);
uint8_t getGanhoPga(pga_t pga);

void timerOn(uint8_t t_ms);
void timerOff(void);
void timerWait(void);
void adcOn(leitura_t op);
void adcOff();
void controleLed(uint8_t led, flag_t status);
void controleSH(uint8_t op, flag_t status);

#endif /* OXIMETRO_H_ */
