#include "estacao.h"
#include "lib/avr_usart.h"
#include "dht22.h"
#include "temt6000.h"

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
volatile flag_t timer_amost = OFF, ciclo_oximetro = ON;
volatile uint16_t acordar = TEMPO_SLEEP;
estacao_t leitura;
dht22_t dht22;

/* Inicializacoes */
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

	//GPIO_B->DDR  |= SET(PB0) | SET(PB1) | SET(PB2) | SET(PB3) | SET(PB4);
	//GPIO_D->DDR  |= SET(PD6) | SET(PD7);
	//GPIO_D->PORT |= SET(PD0) | SET(PD1) | SET(PD2) | SET(PD3);

	temtInit();
	timerInit();
	dht_init(&dht22, PB1);
	curr_state = TEMPERATURA;

	set_sleep_mode(SLEEP_MODE_IDLE); //SLEEP_MODE_EXT_STANDBY

}

/* funcoes da maquina de estado */
void f_temperatura()
{

	if(dht_read_data(&dht22, &leitura.temperatura, &leitura.umidade))
	{
		fprintf(get_usart_stream(), "---------------------------------\ntemperatura: %d.%d C\n\r", leitura.temperatura/10, leitura.temperatura%10);
		fprintf(get_usart_stream(), "umidade: %d.%d%%\n---------------------------------\n\r", leitura.umidade/10, leitura.umidade%10);
	}

	curr_state = LUZ;

}

void f_luz()
{
	leitura.luz = temtRead();
	fprintf(get_usart_stream(), "Luminosidade: %d\n\r", leitura.luz);

	curr_state = TEMPERATURA;
}

void f_ruido()
{

	curr_state = PROCESSAMENTO;

}

void f_processamento()
{

	curr_state = ENVIO;
}

void f_envio()
{

	curr_state = SLEEP;
	_delay_ms(500);

}

void f_sleep()
{

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
