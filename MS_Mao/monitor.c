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
volatile flag_t timer_amost = OFF, batimento_ativo = OFF, ciclo_oximetro = ON;
volatile uint32_t temperatura;
volatile oximetro_t oximetro;
volatile acelerometro_t acelerometro;
volatile uint16_t oxim_atual[TAM_ADC];
volatile uint16_t acordar = TEMPO_SLEEP;

// Funcoes locais
static flag_t batimentos(flag_t status);
static inline void timerOn(void){TIMER_IRQS->TC0.BITS.TOIE = 1; timer_amost = OFF;}//cpl_bit(GPIO_B->PORT, PB4);
static inline void timerOff(void){ TIMER_IRQS->TC0.BITS.OCIEA = 0;}

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

flag_t batimentos(flag_t status){

	flag_t ret;
	if((batimento_ativo == OFF) && (status == ON)){
		oximetro.batimento = 0;
		batimento_ativo = ON;
		set_bit(GPIO_B->PORT, PB4);
		ret = FALSE;
	}else if(status == OFF){
		batimento_ativo = OFF;
		ciclo_oximetro = OFF;
		clr_bit(GPIO_B->PORT, PB4);
		ret = TRUE;
		//fprintf(get_usart_stream(), " {tempo: %d}\n\r", oximetro.batimento);
	}
	return ret;
}

void controleInit(){

	GPIO_B->DDR  |= SET(PB0) | SET(PB1) | SET(PB2) | SET(PB3) | SET(PB4);
	GPIO_D->DDR  |= SET(PD6) | SET(PD7);
	//GPIO_D->PORT |= SET(PD0) | SET(PD1) | SET(PD2) | SET(PD3);

	adcInit();
	timerInit();
	curr_state = VERMELHO;

	oximetro.I_max = 0;
	oximetro.I_min = 1024;
	oximetro.I_med = 0;

	oximetro.R_max = 0;
	oximetro.R_min = 1024;
	oximetro.R_med = 0;

	oximetro.batimento = 0;

	acelerometro.pos_x = 0;
	acelerometro.pos_y = 0;
	acelerometro.pos_z = 0;
	acelerometro.movimento = 0;

	set_sleep_mode(SLEEP_MODE_IDLE); //SLEEP_MODE_EXT_STANDBY

}

/* funcoes da maquina de estado */
void f_vermelho(){

	timerOn();

	set_bit(CTRL_LED->PORT, P_VERM);
	clr_bit(CTRL_LED->PORT, P_INFV);

	set_bit(CTRL_LED->PORT, PA_VERM);

	_delay_ms(5);

	// Leitura do Vermelho max
	leituraAtual = RAC;
	chg_nibl(ADCS->AD_MUX, RAC);	// SET(MUX0)
	set_bit(ADCS->ADC_SRA, ADSC);
	while(tst_bit(ADCS->ADC_SRA, ADSC));

	clr_bit(CTRL_LED->PORT, PA_VERM);

	// Leitura do Vermelho med
	leituraAtual = RDC;
	chg_nibl(ADCS->AD_MUX, RDC);	// SET(MUX1)
	set_bit(ADCS->ADC_SRA, ADSC);
	while(tst_bit(ADCS->ADC_SRA, ADSC));

	while(!timer_amost);

	curr_state = INFRAV;

}

void f_infrav(){

	timerOn();

	set_bit(CTRL_LED->PORT, P_INFV);
	clr_bit(CTRL_LED->PORT, P_VERM);

	set_bit(CTRL_LED->PORT, PA_INFV);

	_delay_ms(5);

	// Leitura do Infravermelho max
	leituraAtual = IAC;
	chg_nibl(ADCS->AD_MUX, IAC);	// SET(MUX0)
	set_bit(ADCS->ADC_SRA, ADSC);
	while(tst_bit(ADCS->ADC_SRA, ADSC));

	clr_bit(CTRL_LED->PORT, PA_INFV);

	// Leitura do Infravermelho med
	leituraAtual = IDC;
	chg_nibl(ADCS->AD_MUX, IDC);	// SET(MUX1)
	set_bit(ADCS->ADC_SRA, ADSC);
	while(tst_bit(ADCS->ADC_SRA, ADSC));

	while(!timer_amost);

	//curr_state = PROCESSAMENTO;
	curr_state = ENVIO;
}

void f_processamento(){

	clr_bit(CTRL_LED->PORT, P_INFV);
	clr_bit(CTRL_LED->PORT, P_VERM);

	static uint8_t atual = 0, anterior = 0, cont_subida = 0, cont_descida = 0, cont_amostras = 0, op_atual = 0;
	static uint16_t amostra[N_AMOSTRAS] = {0}, amostra_R[N_AMOSTRAS] = {0}, erro = 0, valor_max = 0;
	static flag_t flag_status = OFF, flag_status_anterior = OFF, flag_pronto = OFF;

	oximetro.I_med = (oximetro.I_med + oxim_atual[IDC])/2;
	oximetro.R_med = (oximetro.R_med + oxim_atual[RDC])/2;
	amostra[atual] = oxim_atual[IAC];
	amostra_R[atual] = oxim_atual[RAC];

	if(atual)
		anterior = atual - 1;
	else
		anterior = N_AMOSTRAS - 1;

	if(amostra[atual] > amostra[anterior]){
		cont_subida++;
		if(cont_subida == 5){
			flag_status = SUBINDO;
			cont_descida = 0;
			cont_subida = 0;
		}
	}else if(amostra[atual] < amostra[anterior]){
		cont_descida++;
		if(cont_descida == 5){
			flag_status = DESCENDO;
			cont_descida = 0;
			cont_subida = 0;
		}
	}

	if(amostra[atual] > valor_max){
		valor_max = amostra[atual] - amostra[atual]/50;
		erro = valor_max - valor_max/10;
	}
	cont_amostras++;

	if( (cont_amostras == 150) && (valor_max < 512) ){
		cont_amostras = 0;
		valor_max = 0;
		op_atual++;
		if(op_atual == 4)
			op_atual--;
		fprintf(get_usart_stream(), "OP: %d; max: %d;\n\r", (op_atual + 1), valor_max);
	}else if( (cont_amostras == 150) && (valor_max > 800) ){
		cont_amostras = 0;
		valor_max = 0;
		op_atual--;
		if(op_atual == 255)
			op_atual = 0;
		fprintf(get_usart_stream(), "OP: %d; max: %d;\n\r", (op_atual + 1), valor_max);
	}else if( cont_amostras == 250){
		fprintf(get_usart_stream(), "OP: %d; max: %d;\n\r", (op_atual + 1), valor_max);
		valor_max = 0;
	}

	switch(op_atual){
		case 0:
			clr_bit(CTRL_LED->PORT, P_ganhoA);
			clr_bit(CTRL_LED->PORT, P_ganhoB);
			break;
		case 1:
			set_bit(CTRL_LED->PORT, P_ganhoA);
			clr_bit(CTRL_LED->PORT, P_ganhoB);
			break;
		case 2:
			clr_bit(CTRL_LED->PORT, P_ganhoA);
			set_bit(CTRL_LED->PORT, P_ganhoB);
			break;
		case 3:
			set_bit(CTRL_LED->PORT, P_ganhoA);
			set_bit(CTRL_LED->PORT, P_ganhoB);
			break;
		default:
			op_atual = 0;
			break;
	}

	if(flag_status_anterior != flag_status){
		if(flag_status == DESCENDO){
			uint8_t m = detectar_maior(amostra, N_AMOSTRAS);
			if(amostra[m] > erro){
				oximetro.I_max = amostra[m];
				oximetro.R_max = amostra_R[m];

				if(flag_pronto == DESCENDO)
					flag_pronto = batimentos(ON);
				else if(flag_pronto == ON)
					flag_pronto = batimentos(OFF);
			}
		}else if(flag_status == SUBINDO){
			uint8_t m = detectar_menor(amostra, N_AMOSTRAS);
			oximetro.I_min = amostra[m];
			oximetro.R_min = amostra_R[m];
			if(flag_pronto == OFF)
				flag_pronto = DESCENDO;
			else if(flag_pronto == FALSE)
				flag_pronto = ON;
		}
	}

	flag_status_anterior = flag_status;

	atual++;
	if(atual == N_AMOSTRAS)
		atual = 0;

	if(flag_pronto == TRUE){

		for(atual = 0; atual < N_AMOSTRAS; atual++){
			amostra[atual] = 0;
			amostra_R[atual] = 0;
		}
		valor_max = 0;

		atual = 0;
		erro = 0;
		anterior = 0;
		cont_subida = 0;
		cont_descida = 0;
		cont_amostras = 0;

		// a vida eh bela

		flag_status = OFF;
		flag_status_anterior = OFF;
		flag_pronto = OFF;

		curr_state = ENVIO;
		//curr_state = VERMELHO;
	}else
		curr_state = VERMELHO;
}

void f_temperatura(){

	// Leitura do Infravermelho med
	leituraAtual = TEMP;
	chg_nibl(ADCS->AD_MUX, 2);	// SET(MUX1)
	set_bit(ADCS->ADC_SRA, ADSC);
	while(tst_bit(ADCS->ADC_SRA, ADSC));

	temperatura = oxim_atual[TEMP]*5;
	temperatura = temperatura*100/1024;

	curr_state = ENVIO;

}

void f_envio(){

	static uint8_t mov = 0;

	//fprintf(get_usart_stream(), "batimento: %d;\n\r", 6000/oximetro.batimento);
	//fprintf(get_usart_stream(), "ENVIO: %d[%d]\n\r", mov, acelerometro.movimento);
	if(mov < acelerometro.movimento)
	{
		fprintf(get_usart_stream(), "Acelerometro x: %d;\n\r", (signed)acelerometro.pos_x);
		fprintf(get_usart_stream(), "Acelerometro y: %d;\n\r", (signed)acelerometro.pos_y);
		fprintf(get_usart_stream(), "Acelerometro z: %d;\n\n\r", (signed)acelerometro.pos_z);
		mov = acelerometro.movimento;
	}
	_delay_ms(1000);
/*	fprintf(get_usart_stream(), "Medio [I/R]: [%d, %d];\n\r", oximetro.I_med, oximetro.R_med);
	fprintf(get_usart_stream(), "Maximo [I/R]: [%d, %d];\n\r", oximetro.I_max, oximetro.R_max);
	fprintf(get_usart_stream(), "Minimo [I/R]: [%d, %d];\n\r\n\r", oximetro.I_min, oximetro.R_min);

	uint32_t conta = ((uint32_t)(oximetro.R_max - oximetro.R_min)*oximetro.I_med*100)/((uint32_t)(oximetro.I_max - oximetro.I_min)*oximetro.R_med*5);

	fprintf(get_usart_stream(), "Temperatura: %ld;\n\r", temperatura);
	//fprintf(get_usart_stream(), "conta = [(%u - %u)*%u*100]/[(%u - %u)*%u*5];\n\r\n\r", oximetro.R_max,oximetro.R_min, oximetro.I_med, oximetro.I_max, oximetro.I_min, oximetro.R_med);
	//fprintf(get_usart_stream(), "conta = %ld/%ld;\n\r\n\r", (uint32_t)(oximetro.R_max - oximetro.R_min)*oximetro.I_med*100, (uint32_t)(oximetro.I_max - oximetro.I_min)*oximetro.R_med*5);
	fprintf(get_usart_stream(), "Oximetro : %ld;\n\r\n\r", conta);
*/
	timerOff();
	clr_bit(ADCS->ADC_SRA, ADSC);

	sleep_enable();
	//curr_state = VERMELHO;
	//curr_state = MOVIMENTO;
	//ciclo_oximetro = ON;

	ciclo_oximetro = OFF;
	curr_state = SLEEP;
	_delay_ms(500);

}

void f_sleep(){

	static uint8_t mov = 0;

	if(ciclo_oximetro == OFF){
		if(acordar == 0){
			ciclo_oximetro = ON;
			sleep_disable();
			acordar = TEMPO_SLEEP;
			//curr_state = VERMELHO;
			curr_state = ENVIO;
		}else if(mov < acelerometro.movimento){
			curr_state = ENVIO;
			mov = acelerometro.movimento;
		}else{
			sleep_cpu();
			curr_state = MOVIMENTO;
		}
	}

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

/* Interrupcoes */
ISR(ADC_vect){

	oxim_atual[leituraAtual] = ADC;

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

	if(batimento_ativo == ON)
		oximetro.batimento++;

	if(ciclo_oximetro == OFF)
		acordar--;

}
