/*
 * temt6000.h
 *
 *  Created on: 26 de nov de 2018
 *      Author: diesson
 */

#ifndef TEMT6000_H_
#define TEMT6000_H_

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

// Defines
#define TEMT6000_PIN PC0

void temtInit();
uint16_t temtRead();



#endif /* TEMT6000_H_ */
