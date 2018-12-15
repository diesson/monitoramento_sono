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
volatile uint32_t cont_ruido = 0;
estacao_t leitura;
dht22_t dht22;

extern volatile timer_t controle_timer;

/* Inicializacoes */
void timerInit(){

	TIMER_2->TCCRA = UNSET(COM2A1) | UNSET(COM2A0) | UNSET(COM2B1) | UNSET(COM2B0) | UNSET(WGM21) | UNSET(WGM20);
	TIMER_2->TCCRB = SET(WGM22) | UNSET(CS22) | SET(CS21) | SET(CS20); // ~1.02ms presc = 64
	//TIMER_IRQS->TC0.BITS.TOIE = 1;

	TIMER_1->TCCRA = UNSET(COM1A1) | UNSET(COM1A0) | UNSET(COM1B1) | UNSET(COM1B0) | UNSET(WGM11) | UNSET(WGM10);
	TIMER_1->TCCRB = UNSET(WGM13) | SET(WGM12) | UNSET(CS12) | SET(CS11) | SET(CS10); // ~10ms presc = 64
	TIMER_IRQS->TC1.BITS.OCIEA = 1;
	TIMER_1->OCRA = 2500;

	controle_timer.timer2_status = OFF;
	controle_timer.timer2_tempo = 0;
	controle_timer.timer1_tempo = TEMPO_SLEEP;

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

	curr_state = LUZ;
}

void f_luz()
{
	leitura.luz = ldrRead();

	curr_state = RUIDO;
}

void f_ruido()
{
	leitura.ruido = cont_ruido;
	cont_ruido = 0;
	curr_state = ENVIO;
}

void f_envio()
{
	uint8_t c, i = 0, j = 0, k = 0;
	uint8_t buffer[50] = {0};

	timerOff();
	adcOff();
	timer01State(OFF);

	j = 0;

	do
	{
		if(j == 0){
			softuart_print("*\r");
			k++;
			//fprintf(get_usart_stream(), "*");
		}
		j++;

		c = softuart_getchar();
		//fprintf(get_usart_stream(), "%c", c);
		if(c != '\n')
		{
			if(c != '\r')
				buffer[i] = c;
			i++;
		}
		if(k == 100)
			break;

	}while(c != '\r');

	fprintf(get_usart_stream(), "%s; %d.%d; %d.%d; %d; %lu;\n\r", buffer,
																	leitura.temperatura/10, leitura.temperatura%10,
																	leitura.umidade/10, leitura.umidade%10,
																	leitura.luz,
																	leitura.ruido);


	for(j = 0; j < 50; j++)
		buffer[j] = 0;

	ciclo_ME = OFF;
	sleep_enable();
	timer01State(ON);

	//curr_state = TEMPERATURA;
	_delay_ms(500);
	curr_state = SLEEP;

}

void f_sleep()
{

	if(ciclo_ME == OFF){
		if(controle_timer.timer1_tempo == 0){
			ciclo_ME = ON;
			sleep_disable();
			controle_timer.timer1_tempo = TEMPO_SLEEP;
			curr_state = TEMPERATURA;
		}else{
			sleep_cpu();
		}
	}
}

/* Interrupcoes */
ISR(TIMER2_OVF_vect){ // 1ms

	if((controle_timer.timer2_tempo) && (controle_timer.timer2_status = OFF)){
		controle_timer.timer2_tempo--;
	}else
		controle_timer.timer2_status = ON;

}

ISR(TIMER1_COMPA_vect){ // 10ms

	if(ciclo_ME == OFF)
		if(controle_timer.timer1_tempo > 0)
		controle_timer.timer1_tempo--;

}

ISR(PCINT0_vect){

	if(!tst_bit(PINB,PB0))
		if(cont_ruido < 4200000000)
			cont_ruido++;

}
