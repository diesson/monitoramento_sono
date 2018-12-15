/*
 * temperatura.h
 *
 *  Created on: 11 de dez de 2018
 *      Author: diesson
 */

#ifndef TEMPERATURA_H_
#define TEMPERATURA_H_

#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>

#include "lib/avr_gpio.h"
#include "lib/avr_i2c.h"

#define BMP085_ADDR 0xEE
#define OSS			0	// Oversampling Setting
#define COLUNA_LCD 10
#define LINHA_T 2
#define LINHA_P 2

long read_temperature();
void bmp085_init();
long read_pressure();

#endif /* TEMPERATURA_H_ */
