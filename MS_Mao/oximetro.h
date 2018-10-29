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

uint8_t detectar_maior(uint16_t* vetor, uint8_t n);
uint8_t detectar_menor(uint16_t* vetor, uint8_t n);

#endif /* OXIMETRO_H_ */
