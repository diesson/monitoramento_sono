#include "oximetro.h"

volatile uint8_t ganho_pga_R = 0;
volatile uint8_t ganho_pga_IR = 0;
volatile timer_t controle_timer;

uint8_t detectar_maior(uint16_t* vetor, uint8_t n)
{
	uint8_t maior = 0;
	for(uint8_t j = 1; j < n; j++){
		if(vetor[maior] < vetor[j])
			maior = j;
	}
	return maior;
}

uint8_t detectar_menor(uint16_t* vetor, uint8_t n)
{
	uint8_t menor = 0;
	for(uint8_t j = 1; j < n; j++){
		if(vetor[menor] > vetor[j])
			menor = j;
	}
	return menor;
}

void timerOn(uint8_t t_ms){
	TIMER_IRQS->TC0.BITS.TOIE = 1;
	controle_timer.timer0_tempo = t_ms;
	controle_timer.timer0_status = OFF;
}

void timerOff(void){
	TIMER_IRQS->TC0.BITS.OCIEA = 0;
}

void timerWait(void){
	while(!controle_timer.timer0_status);
}

void adcOn(leitura_t op){
	chg_nibl(ADCS->AD_MUX, op);
	set_bit(ADCS->ADC_SRA, ADSC);
	while(tst_bit(ADCS->ADC_SRA, ADSC));
}

void adcOff(){
	chg_nibl(ADCS->AD_MUX, 0);
	clr_bit(ADCS->ADC_SRA, ADSC);
}

void controleLed(uint8_t led, flag_t status){
	if(status)
		set_bit(CTRL_LED->PORT, led);
	else
		clr_bit(CTRL_LED->PORT, led);
}

void controleSH(uint8_t op, flag_t status){
	if(status)
		set_bit(CTRL_LED->PORT, op);
	else
		clr_bit(CTRL_LED->PORT, op);
}

flag_t oximetroInclinacao(uint16_t atual, uint16_t anterior, flag_t status){

	static uint8_t cont_subida = 0, cont_descida = 0;

	if(atual > anterior){
		cont_subida++;
		if(cont_subida == N_TESTE){
			status = SUBINDO;
			cont_descida = 0;
			cont_subida = 0;
		}
	}else if(atual < anterior){
		cont_descida++;
		if(cont_descida == N_TESTE){
			status = DESCENDO;
			cont_descida = 0;
			cont_subida = 0;
		}
	}

	return status;

}

uint8_t setControlePGA(pga_t pga, uint8_t op){

	switch(op){
	case 0:
		if(pga == PGA_IRED){
			clr_bit(CTRL_PGA->PORT, P_ganhoA0);
			clr_bit(CTRL_PGA->PORT, P_ganhoA1);
			ganho_pga_IR = 1;
		}else{
			clr_bit(CTRL_PGA->PORT, P_ganhoA2);
			clr_bit(CTRL_PGA->PORT, P_ganhoA3);
			ganho_pga_R = 1;
		}
		break;
	case 1:
		if(pga == PGA_IRED){
			set_bit(CTRL_PGA->PORT, P_ganhoA0);
			clr_bit(CTRL_PGA->PORT, P_ganhoA1);
			ganho_pga_IR = 2;
		}else{
			set_bit(CTRL_PGA->PORT, P_ganhoA2);
			clr_bit(CTRL_PGA->PORT, P_ganhoA3);
			ganho_pga_R = 2;
		}
		break;
	case 2:
		if(pga == PGA_IRED){
			clr_bit(CTRL_PGA->PORT, P_ganhoA0);
			set_bit(CTRL_PGA->PORT, P_ganhoA1);
			ganho_pga_IR = 4;
		}else{
			clr_bit(CTRL_PGA->PORT, P_ganhoA2);
			set_bit(CTRL_PGA->PORT, P_ganhoA3);
			ganho_pga_R = 4;
		}
		break;
	case 3:
		if(pga == PGA_IRED){
			set_bit(CTRL_PGA->PORT, P_ganhoA0);
			set_bit(CTRL_PGA->PORT, P_ganhoA1);
			ganho_pga_IR = 8;
		}else{
			set_bit(CTRL_PGA->PORT, P_ganhoA2);
			set_bit(CTRL_PGA->PORT, P_ganhoA3);
			ganho_pga_R = 8;
		}
		break;
	default:
		op = 0;
		ganho_pga_IR = 0;
		ganho_pga_R = 0;
		break;
	}

	return op;
}

uint16_t getControlePga(uint16_t maximo, pga_t pga){

	static uint8_t cont_IR = 0, cont_R = 0, op_pga_R = 0, op_pga_IR = 0;

	if(pga == PGA_IRED){

		cont_IR++;
		if( (cont_IR == 150) && (maximo < 128) ){
			cont_IR = 0;
			maximo = 0;
			op_pga_IR++;
			if(op_pga_IR == 4)
				op_pga_IR = 3;
		}else if( (cont_IR == 150) && (maximo > 200) ){
			cont_IR = 0;
			maximo = 0;
			op_pga_IR--;
			if(op_pga_IR == 255)
				op_pga_IR = 0;
		}else if( cont_IR == 250){
			maximo = 0;
		}
		setControlePGA(PGA_IRED, op_pga_IR);

	}else if(pga == PGA_RED){

		cont_R++;
		if( (cont_R == 150) && (maximo < 512) ){
			cont_R = 0;
			maximo = 0;
			op_pga_R++;
			if(op_pga_R == 4)
				op_pga_R = 3;
		}else if( (cont_R == 150) && (maximo > 800) ){
			cont_R = 0;
			maximo = 0;
			op_pga_R--;
			if(op_pga_R == 255)
				op_pga_R = 0;
		}else if( cont_R == 250){
			maximo = 0;
		}
		setControlePGA(PGA_RED, op_pga_R);

	}else{
		cont_IR = 0;
		cont_R = 0;
	}

	return maximo;

}

uint8_t getGanhoPga(pga_t pga){

	if(pga == PGA_IRED)
		return ganho_pga_IR;
	else if(pga == PGA_RED)
		return ganho_pga_R;
	else{
		ganho_pga_IR = 0;
		ganho_pga_R = 0;
		return 0;
	}

}
