//--------------------------------------------------------------------------------------------- //
//		AVR e Arduino: Tecnicas de Projeto, 2a ed. - 2012.										//
//--------------------------------------------------------------------------------------------- //
//=============================================================================================	//
//		Sub-rotinas para o trabalho com um LCD 16x2 com via de dados de 4 bits					//	
//		Controlador HD44780	- Pino R/W aterrado													//
//		A via de dados do LCD deve ser ligado aos 4 bits mais significativos ou					//
//		aos 4 bits menos significativos do PORT do uC											// 																
//=============================================================================================	//
#include "LCD_I2C.h"

uint8_t twiBuffer[2];


/* sinal de habilitacaoo para o LCD */
//#define PULSO_ENABLE() 	_delay_us(1); set_bit(data_i2c, E_I2C);
//	i2c_write(data_i2c, data_i2c, PCF_ADDR); _delay_us(1); clr_bit(data_i2c, E_I2C);
//	i2c_write(data_i2c, data_i2c, PCF_ADDR); _delay_us(45)

#define PULSO_ENABLE() set_bit(twiBuffer[1], E_I2C);  TWI_Start_Transceiver_With_Data( twiBuffer, 2 ); clr_bit(twiBuffer[1], E_I2C); TWI_Start_Transceiver_With_Data( twiBuffer, 2 );

/* Stream para utilizacao do fprintf */
FILE lcd_str_i2c = FDEV_SETUP_STREAM(lcd_putchar_i2c, NULL, _FDEV_SETUP_WRITE);
volatile uint8_t data_i2c = 0;

/* Retorna stream local para utilizacao de fprintf */
FILE *inic_stream_i2c(){
	return &lcd_str_i2c;
}


//---------------------------------------------------------------------------------------------
// Sub-rotina para enviar caracteres e comandos ao LCD com via de dados de 4 bits
//---------------------------------------------------------------------------------------------
void cmd_LCD_i2c(uint8_t c, uint8_t cd)				//c é o dado  e cd indica se é instrução ou caractere
{
	if(cd==0)
		clr_bit(twiBuffer[1], RS_I2C);
	else
		set_bit(twiBuffer[1], RS_I2C);

	//primeiro nibble de dados - 4 MSB
	#if (NIBBLE_DADOS_I2C)								//compila código para os pinos de dados do LCD nos 4 MSB do PORT
		twiBuffer[1] = (twiBuffer[1] & 0x0F)|(0xF0 & c);
	#else											//compila código para os pinos de dados do LCD nos 4 LSB do PORT
		twiBuffer[1] = (twiBuffer[1] & 0xF0)|(c>>4);
	#endif
	//set_bit(twiBuffer[1], BACKLIGHT);

	PULSO_ENABLE();

	//segundo nibble de dados - 4 LSB
	#if (NIBBLE_DADOS_I2C)								//compila código para os pinos de dados do LCD nos 4 MSB do PORT
		twiBuffer[1] = (twiBuffer[1] & 0x0F) | (0xF0 & (c<<4));
	#else											//compila código para os pinos de dados do LCD nos 4 LSB do PORT
		twiBuffer[1] = (twiBuffer[1] & 0xF0) | (0x0F & c);
	#endif
	//set_bit(twiBuffer[1], BACKLIGHT);
	
	PULSO_ENABLE();
	
	if((cd==0) && (c<4))				//se for instrução de retorno ou limpeza espera LCD estar pronto
		_delay_ms(2);
}

//---------------------------------------------------------------------------------------------
//Sub-rotina para inicialização do LCD com via de dados de 4 bits
//---------------------------------------------------------------------------------------------
void inic_LCD_4bits_i2c()		//sequência ditada pelo fabricando do circuito integrado HD44780
{							//o LCD será só escrito. Então, R/W é sempre zero.
	twiBuffer[0] = LCD_ADDR;
	twiBuffer[1] = 0;

	/* Configura pinos de controle */
	set_bit(twiBuffer[1], RS_I2C);
	set_bit(twiBuffer[1], E_I2C);

	/* Configure pinos de dados */
	#if (NIBBLE_DADOS_I2C)
		twiBuffer[1] |=  0xF0;
	#else
		twiBuffer[1] |=  0x0F;
	#endif

	TWI_Start_Transceiver_With_Data( twiBuffer, 2 );

	//i2c_write(data_i2c, data_i2c, PCF_ADDR);

	clr_bit(twiBuffer[1], RS_I2C);	//RS em zero indicando que o dado para o LCD será uma instrução
	clr_bit(twiBuffer[1], E_I2C);	//pino de habilitação em zero

	TWI_Start_Transceiver_With_Data( twiBuffer, 2 );

	_delay_ms(20);	 		//tempo para estabilizar a tensão do LCD, após VCC ultrapassar 4.5 V (na prática pode
						//ser maior).
	//interface de 8 bits						
	#if (NIBBLE_DADOS_I2C)
		twiBuffer[1] = (twiBuffer[1] & 0x0F) | 0x30;
	#else		
		twiBuffer[1] = (twiBuffer[1] & 0xF0) | 0x03;
	#endif
							
	PULSO_ENABLE();			//habilitação respeitando os tempos de resposta do LCD
	_delay_ms(5);		
	PULSO_ENABLE();
	_delay_us(200);
	PULSO_ENABLE();	/*até aqui ainda é uma interface de 8 bits.
					Muitos programadores desprezam os comandos acima, respeitando apenas o tempo de
					estabilização da tensão (geralmente funciona). Se o LCD não for inicializado primeiro no 
					modo de 8 bits, haverá problemas se o microcontrolador for inicializado e o display já o tiver sido.*/
	
	//interface de 4 bits, deve ser enviado duas vezes (a outra está abaixo)
	#if (NIBBLE_DADOS_I2C)
		twiBuffer[1] = (twiBuffer[1] & 0x0F) | 0x28;
	#else		
		twiBuffer[1] = (twiBuffer[1] & 0xF0) | 0x0A;
	#endif

	PULSO_ENABLE();
   	cmd_LCD_i2c(0x28,0); 		//interface de 4 bits 2 linhas (aqui se habilita as 2 linhas)
							//são enviados os 2 nibbles (0x2 e 0x8)
   	cmd_LCD_i2c(0x08,0);		//desliga o display
   	cmd_LCD_i2c(0x01,0);		//limpa todo o display
   	cmd_LCD_i2c(0x0C,0);		//mensagem aparente cursor inativo não piscando
   	cmd_LCD_i2c(0x80,0);		//inicializa cursor na primeira posição a esquerda - 1a linha
}

//---------------------------------------------------------------------------------------------
//Sub-rotina de escrita no LCD -  dados armazenados na RAM
//---------------------------------------------------------------------------------------------
void escreve_LCD_i2c(char *c)
{
   for (; *c!=0;c++) cmd_LCD_i2c(*c,1);
}

//---------------------------------------------------------------------------------------------
//Sub-rotina de escrita no LCD - dados armazenados na FLASH
//---------------------------------------------------------------------------------------------
void escreve_LCD_Flash_i2c(const char *c)
{
   for (;pgm_read_byte(&(*c))!=0;c++) cmd_LCD_i2c(pgm_read_byte(&(*c)),1);
}


int lcd_putchar_i2c(char c, FILE *fp){
	cmd_LCD_i2c(c,1);

	return 0;
}
