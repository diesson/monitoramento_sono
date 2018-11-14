#ifndef _LCD_H
#define _LCD_H

#ifndef F_CPU
	#define F_CPU 14745600UL
#endif

#include <util/delay.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdio.h>

#include "../lib/bits.h"
#include "../lib/avr_gpio.h"
//#include "../lib/avr_i2c.h"
#include "../lib/TWI_Master.h"

//Definicoes para facilitar a troca dos pinos do hardware e facilitar a re-programacao

#define LCD_ADDR 0x7E
#define NIBBLE_DADOS_I2C	1		//0 para via de dados do LCD nos 4 LSBs do PORT empregado (Px0-D4, Px1-D5, Px2-D6, Px3-D7)
								//1 para via de dados do LCD nos 4 MSBs do PORT empregado (Px4-D4, Px5-D5, Px6-D6, Px7-D7) 
#define E_I2C    		2     //pino de habilitacao do LCD (enable)
#define RW_I2C   		1     //pino para informar se o dado e uma instrucao ou caracter
#define RS_I2C   		0     //pino para informar se o dado e uma instrucao ou caracter
#define BACKLIGHT 		4

/* Inicializacao de hardware e stream */
void inic_LCD_4bits_i2c();
FILE * inic_stream_i2c();

/* Comandos e escrita estatica */
void cmd_LCD_i2c(uint8_t c, uint8_t cd);
void escreve_LCD_i2c(char *c);
void escreve_LCD_Flash_i2c(const char *c);

int lcd_putchar_i2c(char c, FILE *fp);


#endif
