/*
 * temt6000.h
 *
 *  Created on: 26 de nov de 2018
 *      Author: diesson
 */

#ifndef LDR_H_
#define LDR_H_

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
#include "funcoes.h"

// Defines
#define LDR_PIN PC0

void ldrInit();
uint16_t ldrRead();



#endif /* LDR_H_ */
