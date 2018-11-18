#include "monitor.h"
#include "oximetro.h"
#include "lib/avr_usart.h"

/* Mapeamento entre estado e funções */
fsm_t myFSM[] = {
	{ VERMELHO, f_vermelho },
	{ INFRAV, f_infrav },
	{ TEMPERATURA, f_temperatura },
	{ PROCESSAMENTO, f_processamento },
	{ ENVIO, f_envio },
	{ SLEEP, f_sleep },
	{ MOVIMENTO, f_movimento }
};

// Variaveis globais
volatile state_t curr_state;
volatile leitura_t leituraAtual;
volatile oximetro_t oximetro;
volatile uint16_t oxim_atual[TAM_ADC];
volatile uint32_t temperatura;
volatile acelerometro_t acelerometro;
volatile flag_t batimento_ativo = OFF, ciclo_oximetro = ON;
extern volatile timer_t controle_timer;

flag_t batimentos(flag_t status){

	if((batimento_ativo == OFF) && (status == ON)){
		oximetro.batimento = 0;
		batimento_ativo = ON;
		status = FALSE;
			set_bit(GPIO_B->PORT, PB4);
	}else if(status == OFF){
		ciclo_oximetro = OFF;
		batimento_ativo = OFF;
		status = TRUE;
			clr_bit(GPIO_B->PORT, PB4);
	}
	return status;
}

/* Inicializacoes */
void adcInit(){

	GPIO_C->DDR |= UNSET(PC0) | UNSET(PC1) | UNSET(PC2) | UNSET(PC3) | UNSET(PC4);	// GPIO_C->DDR |= ~(0b00000111);

	// Configuracao dos ADC
	ADCS->AD_MUX = UNSET(REFS1) | SET(REFS0) | UNSET(ADLAR) | UNSET(MUX3) | UNSET(MUX2) | UNSET(MUX1) | UNSET(MUX0); // AVCC
	ADCS->ADC_SRA = SET(ADEN) | SET(ADSC) | UNSET(ADATE) | SET(ADIE) | SET(ADPS2) | SET(ADPS1) | SET(ADPS0);

	// Desabilitar a parte digital
	ADCS->DIDr0.BITS.ADC0 = 1;
	ADCS->DIDr0.BITS.ADC1 = 1;
	ADCS->DIDr0.BITS.ADC2 = 1;
	ADCS->DIDr0.BITS.ADC3 = 1;
	ADCS->DIDr0.BITS.ADC4 = 1;

	// Inicializa no ADC 01
	chg_nibl(ADCS->AD_MUX, 0x01);

}

void timerInit(){

	// Configuracao do Timer 0 [~1.02ms, presc = 64]
	TIMER_0->TCCRA = UNSET(COM0A1) | UNSET(COM0A0) | UNSET(COM0B1) | UNSET(COM0B0) | UNSET(WGM01) | UNSET(WGM00);
	TIMER_0->TCCRB = SET(WGM02) | UNSET(CS02) | SET(CS01) | SET(CS00);

	// Configuracao do Timer 1 [~10ms, presc = 64]
	TIMER_1->TCCRA = UNSET(COM1A1) | UNSET(COM1A0) | UNSET(COM1B1) | UNSET(COM1B0) | UNSET(WGM11) | UNSET(WGM10);
	TIMER_1->TCCRB = UNSET(WGM13) | SET(WGM12) | UNSET(CS12) | SET(CS11) | SET(CS10);
	TIMER_IRQS->TC1.BITS.OCIEA = 1;
	TIMER_1->OCRA = 2500;

	controle_timer.timer0_status = OFF;
	controle_timer.timer0_tempo = 0;
	controle_timer.timer1_tempo = TEMPO_SLEEP;

}

void controleInit(){

	// Inicializacao das pinos
	GPIO_B->DDR  |= SET(PB0) | SET(PB1) | SET(PB2) | SET(PB3) | SET(PB4);
	GPIO_D->DDR  |= SET(PD6) | SET(PD7);

	// Inicializacao do ADC e Timer
	adcInit();
	timerInit();

	// Inicializacao da estrutura de dados do oximetro
	oximetro.I_max = 0;
	oximetro.I_min = 1024;
	oximetro.I_med = 0;

	oximetro.R_max = 0;
	oximetro.R_min = 1024;
	oximetro.R_med = 0;

	oximetro.batimento = 0;

	// Inicializacao da estrutura de dados do acelerometro
	acelerometro.pos_x = 0;
	acelerometro.pos_y = 0;
	acelerometro.pos_z = 0;
	acelerometro.movimento = 0;

	// Inicializacao da maquina de estados
	curr_state = VERMELHO;

	// Configuracao do modo sleep
	set_sleep_mode(SLEEP_MODE_IDLE);			// opcional: SLEEP_MODE_EXT_STANDBY

}

/* funcoes da maquina de estado */
void f_vermelho(){

	// Acionar o LED Vermelho
	controleLed(P_VERM, ON);

	// Acionamento do S/H
	timerOn(3);
	timerWait();
	controleSH(PA_VERM, ON);
	timerOn(1);
	timerWait();
	controleSH(PA_VERM, OFF);
	timerOn(1);
	timerWait();

	// Apagar o LED Vermelho
	controleLed(P_VERM, OFF);

	// Leitura do Vermelho max
	leituraAtual = RAC;
	adcOn(RAC);

	// Leitura do Vermelho med
	leituraAtual = RDC;
	adcOn(RDC);

	curr_state = INFRAV;

}

void f_infrav(){

	// Acionar o LED Infravermelho
	controleLed(P_INFV, ON);

	// Acionamento do S/H
	timerOn(3);
	timerWait();
	controleSH(PA_INFV, ON);
	timerOn(1);
	timerWait();
	controleSH(PA_INFV, OFF);
	timerOn(1);
	timerWait();

	// Apagar o LED Infravermelho
	controleLed(PA_INFV, OFF);

	// Leitura do Infravermelho max
	leituraAtual = IAC;
	adcOn(IAC);

	// Leitura do Infravermelho med
	leituraAtual = IDC;
	adcOn(IDC);

	curr_state = PROCESSAMENTO;

}

void f_processamento(){

	// Garantir que os LED's estao apagados
	controleLed(P_VERM, OFF);
	controleLed(P_INFV, OFF);

	// Variaveis
	static uint8_t 	atual = 0,
					anterior = 0;
	static uint16_t amostra_IR[N_AMOSTRAS] = {0},
					amostra_R[N_AMOSTRAS] = {0},
					maximo_R = 0,
					maximo_IR = 0,
					erro = 0;
	static flag_t 	flag_status = OFF,
					flag_status_anterior = OFF,
					flag_pronto = OFF;

	// Aquisicao dos valores medios
	oximetro.I_med = (oximetro.I_med + oxim_atual[IDC])/2;
	oximetro.R_med = (oximetro.R_med + oxim_atual[RDC])/2;

	// Armazenar os valores AC em um vetor
	amostra_IR[atual] = oxim_atual[IAC];
	amostra_R[atual] = oxim_atual[RAC];

	// Posicao anterior do vetor
	if(atual)
		anterior = atual - 1;
	else
		anterior = N_AMOSTRAS - 1;

	// Verificacao da inclinacao
	flag_status = oximetroInclinacao(amostra_IR[atual], amostra_IR[anterior], flag_status);

	// Armazenar o maior valor
	if(amostra_IR[atual] > maximo_IR){
		maximo_IR = amostra_IR[atual] - amostra_IR[atual]/50;
		erro = maximo_IR - maximo_IR/10;
	}

	if(amostra_R[atual] > maximo_R){
		maximo_R = amostra_R[atual] - amostra_R[atual]/50;
	}

	// Verificar se precisa aumentar o ganho dos PGAs
	maximo_IR = getControlePga(maximo_IR, PGA_IRED);
	maximo_R = getControlePga(maximo_R, PGA_RED);

	// Deteccao de pico ou vale
	if(flag_status_anterior != flag_status){
		if(flag_status == DESCENDO){
			uint8_t m = detectar_maior(amostra_IR, N_AMOSTRAS);
			if(amostra_IR[m] > erro){
				oximetro.I_max = amostra_IR[m];
				oximetro.R_max = amostra_R[m];

				if(flag_pronto == DESCENDO)
					flag_pronto = batimentos(ON);
				else if(flag_pronto == ON)
					flag_pronto = batimentos(OFF);
			}
		}else if(flag_status == SUBINDO){
			uint8_t m = detectar_menor(amostra_IR, N_AMOSTRAS);
			oximetro.I_min = amostra_IR[m];
			oximetro.R_min = amostra_R[m];

			if(flag_pronto == OFF)
				flag_pronto = DESCENDO;
			else if(flag_pronto == FALSE)
				flag_pronto = ON;
		}
	}

	flag_status_anterior = flag_status;

	// Atualizacao da posicao do indice do vetor
	atual++;
	if(atual == N_AMOSTRAS)
		atual = 0;

	// Reset das variaveis ou retorno para a proxima amostragem
	if(flag_pronto == TRUE){

		for(atual = 0; atual < N_AMOSTRAS; atual++){
			amostra_IR[atual] = 0;
			amostra_R[atual] = 0;
		}

		getControlePga(maximo_IR, PGA_RESET);

		maximo_IR = 0;
		maximo_R = 0;
		erro = 0;

		atual = 0;
		anterior = 0;

		flag_status = OFF;
		flag_status_anterior = OFF;
		flag_pronto = OFF;

		curr_state = TEMPERATURA;
	}else
		curr_state = VERMELHO;
}

void f_temperatura(){

	// Leitura da temperatura
	leituraAtual = TEMP;
	adcOn(TEMP);

	// Calculo da temperatura
	temperatura = oxim_atual[TEMP]*5;
	temperatura = temperatura*100/1024;

	curr_state = ENVIO;

}

void f_movimento(){

	uint16_t pos_x, pos_y, pos_z, delta = 0, delta_temp = 0;

	pos_x = read_x_accelerometer();
	pos_y = read_y_accelerometer();
	pos_z = read_z_accelerometer();

	delta_temp = acelerometro.pos_x - pos_x;
	if((signed)delta_temp < 0)
		delta = delta_temp*-1;

	delta_temp = acelerometro.pos_y - pos_y;
	if((signed)delta_temp < 0)
		delta += delta_temp*-1;

	delta_temp = acelerometro.pos_z - pos_z;
	if((signed)delta_temp < 0)
		delta += delta_temp*-1;

	if(delta > VALOR_COMPARACAO)
		acelerometro.movimento++;

	acelerometro.pos_x = pos_x;
	acelerometro.pos_y = pos_y;
	acelerometro.pos_z = pos_z;

	//fprintf(get_usart_stream(), "delta: %d\n\r", delta);
	curr_state = SLEEP;

}

void f_envio(){

	static uint8_t mov = 0;
	uint8_t ganho_R, ganho_IR;

	// Calculo da oxigenacao
	ganho_R = getGanhoPga(PGA_RED);
	ganho_IR = getGanhoPga(PGA_IRED);
	uint32_t conta = 	((uint32_t)(oximetro.R_max - oximetro.R_min)*oximetro.I_med*ganho_IR*100)/
						((uint32_t)(oximetro.I_max - oximetro.I_min)*oximetro.R_med*ganho_R);

	// Envio da informacao via USART para depuracao
	fprintf(get_usart_stream(), "batimento: %d;\n\r", 6000/oximetro.batimento);
	fprintf(get_usart_stream(), "[Ganho R: %d; Ganho IR: %d]\n\r", ganho_R, ganho_IR);
	fprintf(get_usart_stream(), "Oximetro : %ld;\n\r\n\r", conta);

	if(mov < acelerometro.movimento){
		fprintf(get_usart_stream(), "Acelerometro: [%d, %d, %d]; %d movs\n\r", 	(signed)acelerometro.pos_x,
																				(signed)acelerometro.pos_y,
																				(signed)acelerometro.pos_z,
																				acelerometro.movimento);
		mov = acelerometro.movimento;
	}

	fprintf(get_usart_stream(), "Temperatura: %ld;\n\r", temperatura);

/*	_delay_ms(1000);
	fprintf(get_usart_stream(), "Medio [I/R]: [%d, %d];\n\r", oximetro.I_med, oximetro.R_med);
	fprintf(get_usart_stream(), "Maximo [I/R]: [%d, %d];\n\r", oximetro.I_max, oximetro.R_max);
	fprintf(get_usart_stream(), "Minimo [I/R]: [%d, %d];\n\r\n\r", oximetro.I_min, oximetro.R_min);

	uint32_t conta = ((uint32_t)(oximetro.R_max - oximetro.R_min)*oximetro.I_med*100)/((uint32_t)(oximetro.I_max - oximetro.I_min)*oximetro.R_med*5);

	fprintf(get_usart_stream(), "Temperatura: %ld;\n\r", temperatura);
	fprintf(get_usart_stream(), "conta = [(%u - %u)*%u*100]/[(%u - %u)*%u*5];\n\r\n\r", oximetro.R_max,oximetro.R_min, oximetro.I_med, oximetro.I_max, oximetro.I_min, oximetro.R_med);
	fprintf(get_usart_stream(), "conta = %ld/%ld;\n\r\n\r", (uint32_t)(oximetro.R_max - oximetro.R_min)*oximetro.I_med*100, (uint32_t)(oximetro.I_max - oximetro.I_min)*oximetro.R_med*5);
	fprintf(get_usart_stream(), "Oximetro : %ld;\n\r\n\r", conta);*/

	// Preparo para entrar no modo Sleep
	timerOff();
	adcOff();
	sleep_enable();

	ciclo_oximetro = OFF;
	curr_state = SLEEP;

	_delay_ms(1000);

}

void f_sleep(){

	static uint8_t mov = 0;

	if(ciclo_oximetro == OFF){
		if(controle_timer.timer1_tempo == 0){
			ciclo_oximetro = ON;
			sleep_disable();
			controle_timer.timer1_tempo = TEMPO_SLEEP;
			curr_state = VERMELHO;
		}else if(mov < acelerometro.movimento){
			//curr_state = ENVIO;
			mov = acelerometro.movimento;
		}else{
			sleep_cpu();
			curr_state = MOVIMENTO;
		}
	}

}

/* Interrupcoes */
ISR(ADC_vect){

	oxim_atual[leituraAtual] = ADC;

}

ISR(TIMER0_OVF_vect){ // 1ms

	if((controle_timer.timer0_tempo) && (controle_timer.timer0_status = OFF)){
		controle_timer.timer0_tempo--;
	}else
		controle_timer.timer0_status = ON;

}

ISR(TIMER1_COMPA_vect){ // 10ms

	if(batimento_ativo == ON)
		oximetro.batimento++;

	if(ciclo_oximetro == OFF)
		controle_timer.timer1_tempo--;

}
