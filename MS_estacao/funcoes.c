/*
 * funcoes.c
 *
 *  Created on: 9 de dez de 2018
 *      Author: diesson
 */

#include "funcoes.h"
volatile timer_t controle_timer;

void timerOn(uint8_t t_ms){
	TIMER_IRQS->TC2.BITS.TOIE = 1;
	controle_timer.timer2_tempo = t_ms;
	controle_timer.timer2_status = OFF;
}

void timer01State(flag_t flag){
	TIMER_IRQS->TC1.BITS.OCIEA = flag;
}

void timerOff(void){
	TIMER_IRQS->TC2.BITS.OCIEA = 0;
}

void timerWait(void){
	while(!controle_timer.timer2_status);
}

void adcOn(uint8_t op){
	chg_nibl(ADCS->AD_MUX, op);
	set_bit(ADCS->ADC_SRA, ADSC);
	while(tst_bit(ADCS->ADC_SRA, ADSC));
}

void adcOff(){
	chg_nibl(ADCS->AD_MUX, 0);
	clr_bit(ADCS->ADC_SRA, ADSC);
}
