/*
 * accelerometer.h
 *
 *  Created on: 7 de nov de 2018
 *      Author: diesson
 */

#ifndef ACCELEROMETER_H_
#define ACCELEROMETER_H_

#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>

//#include "lib/i2c_master.h"
#include "lib/avr_i2c.h"
#include "lib/avr_usart.h"

void adxl345_init();
uint16_t read_x_accelerometer();
uint16_t read_y_accelerometer();
uint16_t read_z_accelerometer();
void print_adxl345_values();

#endif /* ACCELEROMETER_H_ */
