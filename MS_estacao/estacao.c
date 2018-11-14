#include "estacao.h"
#include "lib/avr_usart.h"

/* Mapeamento entre estado e funções */
fsm_t myFSM[] = {
	{ TEMPERATURA, f_temperatura },
	{ LUZ, f_luz },
	{ RUIDO, f_ruido },
	{ PROCESSAMENTO, f_processamento },
	{ ENVIO, f_envio }
};

// Variaveis globais
volatile state_t curr_state;
volatile flag_t timer_amost = OFF, batimento_ativo = OFF, ciclo_oximetro = ON;
volatile uint16_t acordar = TEMPO_SLEEP;

/* Inicializacoes */
void adcInit(){

	// Entrada[1, 2] [ADC]
	GPIO_C->DDR |= UNSET(PC0) | UNSET(PC1) | UNSET(PC2) | UNSET(PC3) | UNSET(PC4);	// GPIO_C->DDR |= ~(0b00000111);

	// Configuracao
	ADCS->AD_MUX = UNSET(REFS1) | SET(REFS0) | UNSET(ADLAR) | UNSET(MUX3) | UNSET(MUX2) | UNSET(MUX1) | UNSET(MUX0); // AVCC
	ADCS->ADC_SRA = SET(ADEN) | SET(ADSC) | UNSET(ADATE) | SET(ADIE) | SET(ADPS2) | SET(ADPS1) | SET(ADPS0);

	// Desabilitar parte digital
	ADCS->DIDr0.BITS.ADC0 = 1;
	ADCS->DIDr0.BITS.ADC1 = 1;
	ADCS->DIDr0.BITS.ADC2 = 1;
	ADCS->DIDr0.BITS.ADC3 = 1;
	ADCS->DIDr0.BITS.ADC4 = 1;

	chg_nibl(ADCS->AD_MUX, 0x01);	// SET(MUX0)

}

void timerInit(){

	TIMER_0->TCCRA = UNSET(COM0A1) | UNSET(COM0A0) | UNSET(COM0B1) | UNSET(COM0B0) | UNSET(WGM01) | UNSET(WGM00);
	TIMER_0->TCCRB = SET(WGM02) | UNSET(CS02) | SET(CS01) | SET(CS00); // ~1.02ms presc = 64
	//TIMER_IRQS->TC0.BITS.TOIE = 1;

	TIMER_1->TCCRA = UNSET(COM1A1) | UNSET(COM1A0) | UNSET(COM1B1) | UNSET(COM1B0) | UNSET(WGM11) | UNSET(WGM10);
	TIMER_1->TCCRB = UNSET(WGM13) | SET(WGM12) | UNSET(CS12) | SET(CS11) | SET(CS10); // ~10ms presc = 64
	TIMER_IRQS->TC1.BITS.OCIEA = 1;
	TIMER_1->OCRA = 2500;

}

void controleInit(){

	GPIO_B->DDR  |= SET(PB0) | SET(PB1) | SET(PB2) | SET(PB3) | SET(PB4);
	GPIO_D->DDR  |= SET(PD6) | SET(PD7);
	//GPIO_D->PORT |= SET(PD0) | SET(PD1) | SET(PD2) | SET(PD3);

	adcInit();
	timerInit();
	curr_state = TEMPERATURA;


	set_sleep_mode(SLEEP_MODE_IDLE); //SLEEP_MODE_EXT_STANDBY

}

/* funcoes da maquina de estado */
void f_temperatura(){

	curr_state = LUZ;

}

void f_luz(){

	curr_state = RUIDO;
}

void f_ruido(){

	curr_state = PROCESSAMENTO;

}

void f_processamento(){

	curr_state = ENVIO;
}

void f_envio(){

	curr_state = SLEEP;
	_delay_ms(500);

}

void f_sleep(){

	if(ciclo_oximetro == OFF){
		if(acordar == 0){
			ciclo_oximetro = ON;
			sleep_disable();
			acordar = TEMPO_SLEEP;
			//curr_state = VERMELHO;
			curr_state = ENVIO;
		}else{
			sleep_cpu();
			curr_state = TEMPERATURA;
		}
	}

}

/* Interrupcoes */
ISR(ADC_vect){



}

ISR(TIMER0_OVF_vect){ // 2ms

	static uint8_t contTimer = 0;

	if(contTimer > 1){
		timer_amost = ON;
		contTimer = 0;
	}
	contTimer++;

}

ISR(TIMER1_COMPA_vect){ // 10ms

	if(ciclo_oximetro == OFF)
		acordar--;

}
