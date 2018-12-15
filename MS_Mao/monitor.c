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
volatile uint8_t periodo = 0;
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
			//set_bit(GPIO_B->PORT, PB4);
	}else if(status == OFF){
		ciclo_oximetro = OFF;
		batimento_ativo = OFF;
		status = TRUE;
			//clr_bit(GPIO_B->PORT, PB4);
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
					anterior = 0,
					c = 0,
					k = 0;
	static uint16_t amostra_IR[N_AMOSTRAS] = {0},
					amostra_R[N_AMOSTRAS] = {0},
					maximo_R = 0,
					maximo_IR = 0,
					erro = 0,
					taxa = 0;
	static flag_t 	flag_status = OFF,
					flag_status_anterior = OFF,
					flag_pronto = OFF;

		// Aquisicao dos valores medios
	oximetro.I_med = (oximetro.I_med + oxim_atual[IDC])/2;
	oximetro.R_med = (oximetro.R_med + oxim_atual[RDC])/2;

	// Armazenar os valores AC em um vetor
	amostra_IR[atual] = oxim_atual[IAC]>>2;
	amostra_R[atual] = oxim_atual[RAC]>>2;

	// Posicao anterior do vetor
	if(atual)
		anterior = atual - 1;
	else
		anterior = N_AMOSTRAS - 1;


	// Armazenar o maior valor
	if(amostra_IR[atual] > erro){
		maximo_IR = amostra_IR[atual] - amostra_IR[atual]/50;
		//erro = amostra_IR[atual] - amostra_IR[atual]/10;
		erro = amostra_IR[atual] - 5;
	}

	if(amostra_R[atual] > maximo_R){
		maximo_R = amostra_R[atual] - amostra_R[atual]/50;
	}

	k++;
	if(k == 0){
		erro = erro/2;
	}

	// Verificar se precisa aumentar o ganho dos PGAs
	maximo_IR = getControlePga(maximo_IR, PGA_IRED);
	maximo_R = getControlePga(maximo_R, PGA_RED);

	switch(getGanhoPga(PGA_IRED)){
		case 1: taxa = (erro*95)/100; break;
		case 2: taxa = (erro*9)/10; break;
		case 4: taxa = (erro*8)/10; break;
		case 8: taxa = (erro*7)/10; break;
		default: break;
	}


	if(c == 200)
	{
		// Verificacao da inclinacao
		flag_status = oximetroInclinacao(amostra_IR[atual], amostra_IR[anterior], flag_status);

		// Deteccao de pico ou vale
		if(flag_status_anterior != flag_status){

			if(amostra_IR[atual] > taxa){
				//cpl_bit(GPIO_B->PORT, PB4);
				if(flag_pronto == OFF)
				{
					batimentos(ON);
					flag_pronto = ON;
					set_bit(GPIO_B->PORT, PB4);
				}
				else if(flag_pronto == ON)
				{
					batimentos(OFF);
					flag_pronto = TRUE;
					clr_bit(GPIO_B->PORT, PB4);

				}
			}

		}
		//	fprintf(get_usart_stream(), "atual: %d;\n", amostra_IR[atual]);

		flag_status_anterior = flag_status;

		// Atualizacao da posicao do indice do vetor
		atual++;
		if(atual == N_AMOSTRAS)
			atual = 0;
	}
	if(c < 200){
		c++;
	}

	// Reset das variaveis ou retorno para a proxima amostragem
	if(flag_pronto == TRUE){

		for(atual = 0; atual < N_AMOSTRAS; atual++){
			amostra_IR[atual] = 0;
			amostra_R[atual] = 0;
		}
		batimentos(OFF);
		clr_bit(GPIO_B->PORT, PB4);

		getControlePga(maximo_IR, PGA_RESET);

		maximo_IR = 0;
		maximo_R = 0;
		erro = 0;
		c = 0;

		atual = 0;
		anterior = 0;

		flag_status = OFF;
		flag_status_anterior = OFF;
		flag_pronto = OFF;

		uint8_t v = 6000/oximetro.batimento;

		if((v < 120) || (v > 50))
			//curr_state = VERMELHO;
			curr_state = TEMPERATURA;
		else
			curr_state = VERMELHO;
		//curr_state = ENVIO;
	}else
		curr_state = VERMELHO;

}

void f_temperatura(){

	// Leitura da temperatura
	temperatura = read_temperature();

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

	curr_state = SLEEP;

}

void f_envio(){

	uint8_t ganho_R, ganho_IR, i, j, teste;
	uint8_t buffer[50] = {0};

	// Calculo da oxigenacao
	ganho_R = getGanhoPga(PGA_RED);
	ganho_IR = getGanhoPga(PGA_IRED);
	uint32_t conta = 	((uint32_t)(oximetro.R_max - oximetro.R_min)*oximetro.I_med*ganho_IR*100)/
						((uint32_t)(oximetro.I_max - oximetro.I_min)*oximetro.R_med*ganho_R);

	//fprintf(get_usart_stream(), "%d; %d; %d; %d; %d, %d [%ld]\n\r", oximetro.I_max, oximetro.I_min, oximetro.I_med, oximetro.R_max, oximetro.R_min, oximetro.R_med, conta);

	i = 0;
	teste = 1;
	while(usartFlag()){
		if(teste != 0)
			timerOn(255);
		teste = 0;

		if(controle_timer.timer0_status){
			i++;
			teste = 1;
		}

		if(i >= 8){
			usartCopy(buffer, 40);
			for(j = 0; j < 50; j++)
				buffer[j] = 0;
			teste = 2;
			break;
		}
	}
	if(teste != 2){
		usartCopy(buffer, 40);

		teste = 0;
		for(i = 0; i < 40; i++){
			if(buffer[i] == '*'){
				teste = 1;
				break;
			}
		}
	}


	if(teste == 1){
		fprintf(get_usart_stream(), "%d; %ld.%ld; %d; %ld\r", 6000/oximetro.batimento,
															temperatura/10,
															temperatura%10,
															acelerometro.movimento,
															conta);
		acelerometro.movimento = 0;
	}
	usartCopy(buffer, 10);

	// Preparo para entrar no modo Sleep
	timerOff();
	adcOff();
	sleep_enable();

	ciclo_oximetro = OFF;
	curr_state = SLEEP;

	//_delay_ms(1000);

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
		if(controle_timer.timer1_tempo != 0)
			controle_timer.timer1_tempo--;

}
