/*
 * temperatura.c
 *
 *  Created on: 11 de dez de 2018
 *      Author: diesson
 */


#include "temperatura.h"

volatile int16_t ac1, ac2, ac3, ac4, ac5, ac6, b1, b2, mb, mc, md;
volatile long b5;

void bmp085_init()
{
	ac1 = i2c_read(0xAA, BMP085_ADDR) << 8;
	ac1 |= i2c_read(0xAB, BMP085_ADDR);
	ac2 = i2c_read(0xAC, BMP085_ADDR) << 8;
	ac2 |= i2c_read(0xAD, BMP085_ADDR);
	ac3 = i2c_read(0xAE, BMP085_ADDR) << 8;
	ac3 |= i2c_read(0xAF,BMP085_ADDR);
	ac4 = i2c_read(0xB0, BMP085_ADDR) << 8;
	ac4 |= i2c_read(0xB1, BMP085_ADDR);
	ac5 = i2c_read(0xB2, BMP085_ADDR) << 8;
	ac5 |= i2c_read(0xB3, BMP085_ADDR);
	ac6 = i2c_read(0xB4, BMP085_ADDR) << 8;
	ac6 |= i2c_read(0xB5, BMP085_ADDR);
	b1 = i2c_read(0xB6, BMP085_ADDR) << 8;
	b1 |= i2c_read(0xB7, BMP085_ADDR);
	b2 = i2c_read(0xB8, BMP085_ADDR) << 8;
	b2 |= i2c_read(0xB9, BMP085_ADDR);
	mb = i2c_read(0xBA, BMP085_ADDR) << 8;
	mb |= i2c_read(0xBB, BMP085_ADDR);
	mc = i2c_read(0xBC, BMP085_ADDR) << 8;
	mc |= i2c_read(0xBD, BMP085_ADDR);
	md = i2c_read(0xBE, BMP085_ADDR) << 8;
	md |= i2c_read(0xBF, BMP085_ADDR);

}

long read_temperature(){

	long x1, x2, temperatura;

	i2c_write(0xF4, 0x2E, BMP085_ADDR);
	_delay_ms(5);

	x1 = i2c_read(0xF6, BMP085_ADDR);
	x1 <<= 8;
	x1 |= i2c_read(0xF7, BMP085_ADDR);


	x1 = (((long)x1 - (long)ac6)*(long)ac5) >> 15;
	x2 = ((long)mc << 11)/((long)x1 + (long)md);
	b5 = x1 + x2;

	temperatura = ((b5 + 8)>>4);
	//temperatura = temperatura/10;

	return temperatura;
}

long read_pressure(){
  long x1, x2, x3, b3, b6, p;
  unsigned long b4, b7;

  i2c_write(0xF4, 0x34, BMP085_ADDR);
  _delay_ms(5);

  b7 = i2c_read(0xF6, BMP085_ADDR);
  b7 <<= 8;
  b7 |= i2c_read(0xF7, BMP085_ADDR);

  b6 = b5 - 4000;
  // Calcula B3
  x1 = (b2 * (b6 * b6)>>12)>>11;
  x2 = (ac2 * b6)>>11;
  x3 = x1 + x2;
  b3 = (((((long)ac1)*4 + x3)<<OSS) + 2)>>2;

  // Calcula B4
  x1 = (ac3 * b6)>>13;
  x2 = (b1 * ((b6 * b6)>>12))>>16;
  x3 = ((x1 + x2) + 2)>>2;
  b4 = -(ac4 * (unsigned long)(x3 + 32768))>>15;

  b7 = ((unsigned long)(b7 - b3) * (50000>>OSS));
  if (b7 < 0x80000000)
    p = (b7<<1)/b4;
  else
    p = (b7/b4)<<1;

  x1 = (p>>8) * (p>>8);
  x1 = (x1 * 3038)>>16;
  x2 = (-7357 * p)>>16;
  p += (x1 + x2 + 3791)>>4;

  return p;
}

