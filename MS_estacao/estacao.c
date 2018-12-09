#include "estacao.h"
#include "funcoes.h"
#include "dht22.h"
#include "LDR.h"

/* Mapeamento entre estado e funções */
fsm_t myFSM[] = {
	{ TEMPERATURA, f_temperatura },
	{ LUZ, f_luz },
	{ RUIDO, f_ruido },
	{ ENVIO, f_envio },
	{ SLEEP, f_sleep }
};

// Variaveis globais
volatile state_t curr_state;
volatile flag_t ciclo_ME = ON;
volatile uint16_t cont_ruido = 0;
estacao_t leitura;
dht22_t dht22;

extern volatile timer_t temporizador;

/* Inicializacoes */
void timerInit(){

	TIMER_0->TCCRA = UNSET(COM0A1) | UNSET(COM0A0) | UNSET(COM0B1) | UNSET(COM0B0) | UNSET(WGM01) | UNSET(WGM00);
	TIMER_0->TCCRB = SET(WGM02) | UNSET(CS02) | SET(CS01) | SET(CS00); // ~1.02ms presc = 64
	//TIMER_IRQS->TC0.BITS.TOIE = 1;

	TIMER_1->TCCRA = UNSET(COM1A1) | UNSET(COM1A0) | UNSET(COM1B1) | UNSET(COM1B0) | UNSET(WGM11) | UNSET(WGM10);
	TIMER_1->TCCRB = UNSET(WGM13) | SET(WGM12) | UNSET(CS12) | SET(CS11) | SET(CS10); // ~10ms presc = 64
	TIMER_IRQS->TC1.BITS.OCIEA = 1;
	TIMER_1->OCRA = 2500;

	temporizador.timer0_status = OFF;
	temporizador.timer0_tempo = 0;
	temporizador.timer1_tempo = TEMPO_SLEEP;

}

void controleInit(){

	//GPIO_B->DDR  |= SET(PB0) | SET(PB1) | SET(PB2) | SET(PB3) | SET(PB4);
	//GPIO_D->DDR  |= SET(PD6) | SET(PD7);
	//GPIO_D->PORT |= SET(PD0) | SET(PD1) | SET(PD2) | SET(PD3);

	GPIO_B->DDR  &= ~(1 << PB0);
	GPIO_B->PORT |= (1 << PB0);
	EXT_IRQ->PC_INT.BITS.PCIE_0 = 1;
	EXT_IRQ_PCINT_MASK->PCMASK0.BITS.PCINT_0 = 1;

	ldrInit();
	timerInit();
	dht_init(&dht22, PB1);
	curr_state = TEMPERATURA;

	set_sleep_mode(SLEEP_MODE_IDLE); //SLEEP_MODE_EXT_STANDBY

}

/* funcoes da maquina de estado */
void f_temperatura()
{
	while(!dht_read_data(&dht22, &leitura.temperatura, &leitura.umidade));
	fprintf(get_usart_stream(), "*");

	curr_state = LUZ;
}

void f_luz()
{
	leitura.luz = ldrRead();
	fprintf(get_usart_stream(), "*");

	curr_state = RUIDO;
}

void f_ruido()
{
	leitura.ruido = cont_ruido;
	cont_ruido = 0;
	fprintf(get_usart_stream(), "*");
	curr_state = ENVIO;
}

void f_envio()
{

	fprintf(get_usart_stream(), "%d.%d; %d.%d; %d; %d;\n\r", leitura.temperatura/10, leitura.temperatura%10,
															leitura.umidade/10, leitura.umidade%10,
															leitura.luz,
															leitura.ruido);

	ciclo_ME = OFF;
	timerOff();
	adcOff();
	sleep_enable();

	curr_state = SLEEP;
	_delay_ms(500);

}

void f_sleep()
{

	if(ciclo_ME == OFF){
		if(temporizador.timer1_tempo == 0){
			ciclo_ME = ON;
			sleep_disable();
			temporizador.timer1_tempo = TEMPO_SLEEP;
			curr_state = TEMPERATURA;
		}else{
			sleep_cpu();
		}
	}

}

/* Interrupcoes */
ISR(TIMER0_OVF_vect){ // 1ms

	if((temporizador.timer0_tempo) && (temporizador.timer0_status = OFF)){
		temporizador.timer0_tempo--;
	}else
		temporizador.timer0_status = ON;

}

ISR(TIMER1_COMPA_vect){ // 10ms

	if(ciclo_ME == OFF)
		temporizador.timer1_tempo--;

}

ISR(PCINT0_vect){

	if(!tst_bit(PINB,PB0))
		cont_ruido++;

}
