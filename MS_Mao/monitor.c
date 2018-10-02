#include "monitor.h"
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
volatile flag_t timer_20m = OFF, batimento_ativo = OFF, ciclo_oximetro = OFF;
volatile uint32_t temperatura;
volatile oximetro_t oximetro;
volatile uint16_t oxim_atual[TAM_ADC];
volatile uint16_t acordar = TEMPO_SLEEP;

// Funcoes locais
static void batimentos(uint8_t flag_batimento);
static inline void timerOn(void){TIMER_IRQS->TC0.BITS.TOIE = 1; timer_20m = OFF;}//cpl_bit(GPIO_B->PORT, PB4);
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

void batimentos(uint8_t flag_batimento){

	if(flag_batimento == ON){
		if(batimento_ativo == OFF){//TIMER_IRQS->TC1.BITS.OCIEA = 1;
			oximetro.batimento = 0;
			batimento_ativo = ON;
			set_bit(GPIO_B->PORT, PB4);
		}else{//TIMER_IRQS->TC1.BITS.OCIEA = 0;
			batimento_ativo = OFF;
			ciclo_oximetro = OFF;
			clr_bit(GPIO_B->PORT, PB4);
		}
	}

}

void controleInit(){

	GPIO_B->DDR  |= SET(PB0) | SET(PB1) | SET(PB2) | SET(PB3) | SET(PB4);
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

	set_sleep_mode(SLEEP_MODE_IDLE); //SLEEP_MODE_EXT_STANDBY

}

/* funcoes da maquina de estado */
void f_vermelho(){

	timerOn();

	set_bit(CTRL_LED->PORT, P_VERM);
	clr_bit(CTRL_LED->PORT, P_INFV);

	set_bit(CTRL_LED->PORT, PA_VERM);

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

	while(!timer_20m);

	curr_state = INFRAV;

}

void f_infrav(){

	timerOn();

	set_bit(CTRL_LED->PORT, P_INFV);
	clr_bit(CTRL_LED->PORT, P_VERM);

	set_bit(CTRL_LED->PORT, PA_INFV);

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

	while(!timer_20m);

	curr_state = PROCESSAMENTO;

}

void f_processamento(){

	clr_bit(CTRL_LED->PORT, P_INFV);
	clr_bit(CTRL_LED->PORT, P_VERM);

	static uint16_t pulso_r[5], pulso_i[5] = {0, 0, 0, 0, 0};
	static uint16_t pulso_r2[5], pulso_i2[5] = {1023, 1023, 1023, 1023};
	static uint8_t i = 0, i_e1, i_e2, i_d1, i_d2, bat = ON;

	switch (i) {
		case 0:
			i_e1 = 4; i_e2 = 3; i_d1 = 1; i_d2 = 2; break;
		case 1:
			i_e1 = 0; i_e2 = 4; i_d1 = 2; i_d2 = 3; break;
		case 2:
			i_e1 = 1; i_e2 = 0; i_d1 = 3; i_d2 = 4; break;
		case 3:
			i_e1 = 2; i_e2 = 1; i_d1 = 4; i_d2 = 0; break;
		case 4:
			i_e1 = 3; i_e2 = 2; i_d1 = 0; i_d2 = 1; break;
		case 5:
			i = 0; break;
		default:
			break;
	}

	pulso_i[i_d2] = oxim_atual[IAC];
	pulso_i2[i_d2] = oxim_atual[IAC];
	pulso_r[i_d2] = oxim_atual[RAC];
	pulso_r2[i_d2] = oxim_atual[RAC];
	oximetro.I_med = (oximetro.I_med + oxim_atual[IDC])/2;
	oximetro.R_med = (oximetro.R_med + oxim_atual[RDC])/2;

	if(pulso_i[i] >= (oximetro.I_max - oximetro.I_max/100)){
		if( (pulso_i[i_e1] <= pulso_i[i]) && (pulso_i[i_e2] <= pulso_i[i]) && (pulso_i[i_d1] <= pulso_i[i]) && (pulso_i[i_d2] <= pulso_i[i]) ){
			oximetro.I_max = pulso_i[i];
			oximetro.R_max = pulso_r[i];
			//cpl_bit(GPIO_B->PORT, PB4); fprintf(get_usart_stream(), " {I %d, R %d}\n\r", oximetro.I_max, oximetro.R_max);
			batimentos(bat);
			bat = OFF;
		}
	}

	if(pulso_i2[i] <= (oximetro.I_min + oximetro.I_min/100)){
		if( (pulso_i2[i_e1] >= pulso_i2[i]) && (pulso_i2[i_e2] >= pulso_i2[i]) && (pulso_i2[i_d1] >= pulso_i2[i]) && (pulso_i2[i_d2] >= pulso_i2[i]) ){
			oximetro.I_min = pulso_i2[i];
			oximetro.R_min = pulso_r2[i];
			//cpl_bit(GPIO_B->PORT, PB4);
			bat = ON;
		}
	}

	i++;

	if(ciclo_oximetro == ON)
		curr_state = VERMELHO;
	else
		curr_state = ENVIO;//curr_state = TEMPERATURA;

	//curr_state = VERMELHO;

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

	fprintf(get_usart_stream(), "batimento: %d;\n\r", 6000/oximetro.batimento);
	fprintf(get_usart_stream(), "Medio [I/R]: [%d, %d];\n\r", oximetro.I_med, oximetro.R_med);
	fprintf(get_usart_stream(), "Maximo [I/R]: [%d, %d];\n\r", oximetro.I_max, oximetro.R_max);
	fprintf(get_usart_stream(), "Minimo [I/R]: [%d, %d];\n\r\n\r", oximetro.I_min, oximetro.R_min);

	uint32_t conta = ((uint32_t)(oximetro.R_max - oximetro.R_min)*oximetro.I_med*100)/((uint32_t)(oximetro.I_max - oximetro.I_min)*oximetro.R_med*5);

	fprintf(get_usart_stream(), "Temperatura: %ld;\n\r", temperatura);
	//fprintf(get_usart_stream(), "conta = [(%u - %u)*%u*100]/[(%u - %u)*%u*5];\n\r\n\r", oximetro.R_max,oximetro.R_min, oximetro.I_med, oximetro.I_max, oximetro.I_min, oximetro.R_med);
	//fprintf(get_usart_stream(), "conta = %ld/%ld;\n\r\n\r", (uint32_t)(oximetro.R_max - oximetro.R_min)*oximetro.I_med*100, (uint32_t)(oximetro.I_max - oximetro.I_min)*oximetro.R_med*5);
	fprintf(get_usart_stream(), "Oximetro : %ld;\n\r\n\r", conta);

	timerOff();
	clr_bit(ADCS->ADC_SRA, ADSC);

	sleep_enable();
	curr_state = SLEEP;

}

void f_sleep(){

	if(ciclo_oximetro == OFF){
		if(acordar == 0){
			ciclo_oximetro = ON;
			sleep_disable();
			acordar = TEMPO_SLEEP;
			curr_state = VERMELHO;
		}else{
			sleep_cpu();
		}
	}

}

void f_movimento(){



}

/* Interrupcoes */
ISR(ADC_vect){

	oxim_atual[leituraAtual] = ADC;

}

ISR(TIMER0_OVF_vect){ // 2ms

	static uint8_t contTimer = 0;

	if(contTimer > 1){
		timer_20m = ON;
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
