/*
 * LDR.c
 *
 *  Created on: 26 de nov de 2018
 *      Author: diesson
 */
#include "LDR.h"
volatile uint16_t ldr;

void ldrInit()
{

	// Entrada[1, 2] [ADC]
	GPIO_C->DDR |= UNSET(LDR_PIN);	// GPIO_C->DDR |= ~(0b00000111);

	// Configuracao
	ADCS->AD_MUX = UNSET(REFS1) | SET(REFS0) | UNSET(ADLAR) | UNSET(MUX3) | UNSET(MUX2) | UNSET(MUX1) | UNSET(MUX0); // AVCC
	ADCS->ADC_SRA = SET(ADEN) | SET(ADSC) | UNSET(ADATE) | SET(ADIE) | SET(ADPS2) | SET(ADPS1) | SET(ADPS0);

	// Desabilitar parte digital
	ADCS->DIDr0.BITS.ADC0 = 1;

	chg_nibl(ADCS->AD_MUX, LDR_PIN);	// SET(MUX0)

}

uint16_t ldrRead()
{

	adcOn(LDR_PIN);

	return ldr;
}

ISR(ADC_vect)
{
	ldr = ADC;
}
