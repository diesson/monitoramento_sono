/*
 * accelerometer.c
 *
 *  Created on: 7 de nov de 2018
 *      Author: diesson
 */

#include "accelerometer.h"

void adxl345_init()
{
	i2c_write(0x2E, 0x60, ADXL345_ADDR);//101110
	i2c_write(0x2F, 0x01, ADXL345_ADDR);
	i2c_write(0x1D, 50, ADXL345_ADDR);//011101
	i2c_write(0x2C, 0x0A, ADXL345_ADDR);//2C [58]
	i2c_write(0x2D, 0x08, ADXL345_ADDR);//2D [5A]
	i2c_write(0x31, 0x08, ADXL345_ADDR);//31 [62]
}

uint16_t read_x_accelerometer()
{
	uint16_t x_value;

	x_value = (i2c_read(0x33, ADXL345_ADDR)) << 8;//110010 0x32
	x_value |= i2c_read(0x32, ADXL345_ADDR);//110011 0x33

	return x_value;
}

uint16_t read_y_accelerometer()
{
	uint16_t y_value;

	y_value = (i2c_read(0x35, ADXL345_ADDR)) << 8;
	y_value |= i2c_read(0x34, ADXL345_ADDR);

	return y_value;
}

uint16_t read_z_accelerometer()
{
	uint16_t z_value;

	z_value = (i2c_read(0x37, ADXL345_ADDR)) << 8;
	z_value |= i2c_read(0x36, ADXL345_ADDR);

	return z_value;
}

void print_adxl345_values()
{

	fprintf(get_usart_stream(), "Eixo X:%03d \n\r", read_x_accelerometer());
	fprintf(get_usart_stream(), "Eixo Y:%03d \n\r", read_y_accelerometer());
	fprintf(get_usart_stream(), "Eixo Z:%03d \n\r", read_z_accelerometer());

}



