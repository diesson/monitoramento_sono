/*
 * temt6000.c
 *
 *  Created on: 26 de nov de 2018
 *      Author: diesson
 */
#include "temt6000.h"
volatile uint16_t temt6000;

void temtInit()
{

	// Entrada[1, 2] [ADC]
	GPIO_C->DDR |= UNSET(PC0);	// GPIO_C->DDR |= ~(0b00000111);

	// Configuracao
	ADCS->AD_MUX = UNSET(REFS1) | SET(REFS0) | UNSET(ADLAR) | UNSET(MUX3) | UNSET(MUX2) | UNSET(MUX1) | UNSET(MUX0); // AVCC
	ADCS->ADC_SRA = SET(ADEN) | SET(ADSC) | UNSET(ADATE) | SET(ADIE) | SET(ADPS2) | SET(ADPS1) | SET(ADPS0);

	// Desabilitar parte digital
	ADCS->DIDr0.BITS.ADC0 = 1;

	chg_nibl(ADCS->AD_MUX, 0x00);	// SET(MUX0)

}

uint16_t temtRead()
{
	chg_nibl(ADCS->AD_MUX, 0x00);
	set_bit(ADCS->ADC_SRA, ADSC);
	while(tst_bit(ADCS->ADC_SRA, ADSC));

	return temt6000;
}

ISR(ADC_vect)
{
	temt6000 = ADC;
}
