#include <avr/io.h>
#include <util/delay.h>

#include "avr_onewire.h"

// macros privadas
#define DQ_OUTPUT()		set_bit(DDRx,DQ)
#define DQ_INPUT()		clr_bit(DDRx,DQ)
#define CLR_DQ()		clr_bit(PORTx,DQ)
#define SET_DQ()		set_bit(PORTx,DQ)
#define TST_DQ()		tst_bit(PINx,DQ)

//funcoes privadas
uint8_t read_bit_1w();
void write_bit_1w(uint8_t bit_1w);

uint8_t reset_1w()	//inicializa os dispositivos no barramento
{
	DQ_OUTPUT();				//DQ como saída
	CLR_DQ();				//DQ em nível zero por 480us
	_delay_us(480);

	DQ_INPUT();			//DQ como entrada, o resistor de pull-up mantém DQ em nível alto
	_delay_us(60);

	if(TST_DQ())			//se nao detectou a presença já retorna 1
		return 1;
		
	_delay_us(420);			//o pulso de presença pode ter 240 us
	return 0;				//retorna 0 para indicar sucesso
}

void power_1w()			//força o barramento em nível alto.
{							//utilizado com dispositivos alimentados no modo parasita
	DQ_OUTPUT();
	SET_DQ();
	_delay_ms(750);
	DQ_INPUT();			//pull-up externo
}

uint8_t read_bit_1w() 	//lê um bit do barramento
{
	uint8_t dado;
	
	DQ_OUTPUT();
	CLR_DQ();	
	_delay_us(2);
	DQ_INPUT();
	_delay_us (15);			//aguarda o dispositivo colocar o dado na saída
	dado = TST_DQ();
	_delay_us(50);
	return (dado);			//retorna o dado
}

void write_bit_1w(uint8_t bit_1w)//escreve um bit no barramento
{
	DQ_OUTPUT();
	CLR_DQ();
	_delay_us(2);
		
	if(bit_1w) 
		SET_DQ(); 
		
	_delay_us(120);
	DQ_INPUT();
}

uint8_t read_byte_1w()	//lê um byte do barramento
{
	uint8_t i, dado = 0;

	for (i=0;i<8;i++)		//lê oito bits iniciando pelo bit menos significativo
	{
		if (read_bit_1w()) 
			set_bit(dado,i);
	}
	return (dado);
}

void write_byte_1w(uint8_t dado)	//escreve um byte no barramento
{
	uint8_t i;

	for (i=0; i<8; i++)		//envia o byte iniciando do bit menos significativo
		write_bit_1w(tst_bit(dado,i));//escreve o bit no barramento
}
