#include <stdio.h>
#include <avr/interrupt.h>
#include "avr_usart.h"
#include "bits.h"
#include "avr_gpio.h"

static int usart_putchar(char c, FILE *fp);

static inline void enqueue(uint8_t dado);

volatile fila_t fila;

/* Stream init for printf  */
FILE usart_str = FDEV_SETUP_STREAM(usart_putchar, NULL, _FDEV_SETUP_WRITE);

/* Return stream pointer  */
FILE * get_usart_stream(){
	return &usart_str;
}

void USART_Init(uint16_t bauds){

	USART_0->UBRR_H = (uint8_t) (bauds >> 8);
	USART_0->UBRR_L = bauds;

	/* Disable double speed  */
	USART_0->UCSR_A = 0;
	/* Enable TX and RX */
	USART_0->UCSR_B = SET(RXEN0) | SET(TXEN0) | SET(RXCIE0);// | SET(TXCIE0);
	/* Asynchronous mode:
	 * - 8 data bits
	 * - 1 stop bit
	 * - no parity 	 */
	USART_0->UCSR_C = SET(UCSZ01) | SET(UCSZ00);

	fila.flag = 0;
	fila.cauda = 0;
	fila.empty = 1;
}

/* Send one byte: busy waiting */
void USART_tx(uint8_t data) {

	/* Wait until hardware is ready */
	while (!(USART_0->UCSR_A & (1 << UDRE0)));

	USART_0->UDR_ = data;
}

/* Receive one byte: busy waiting */
/*uint8_t USART_rx() {

	// Wait until something arrive
	while (!(USART_0->UCSR_A & (1 << RXC0)));

	return USART_0->UDR_;
}
*/

static int usart_putchar(char c, FILE *fp){
	USART_tx(c);

	return 0;
}

uint8_t usartFlag(){
	return !(fila.flag);
}

void usartCopy(uint8_t* vetor, uint8_t n){

	uint8_t i = 0;

	for(;!filaVazia();){
		if(i >= n-1){
			break;
		}
		vetor[i] = dequeue();

		i++;
	}
	vetor[i] = '\0';
	dequeue();

}

ISR(USART_RX_vect){ // Recebe

	uint8_t c = USART_0->UDR_;
	if(c != CHAR_USART)
		enqueue(c);
	else
		fila.flag = 1;

}

//ISR(USART_TX_vect){ // Transmitir
//
//
//
//
//}

/* FILA */
static inline void enqueue(uint8_t dado){

	fila.buffer[fila.cauda] = dado;

	if((fila.cabeca == fila.cauda) && !(fila.empty)){
		fila.cabeca++;
		if(fila.cabeca == TAM_USART)
			fila.cabeca = 0;
	}
	fila.cauda++;
	fila.empty = 0;
	if(fila.cauda == TAM_USART){
		fila.cauda = 0;
	}

}

uint8_t dequeue(){

	if(!fila.empty){
		uint8_t valor = fila.buffer[fila.cabeca];
		fila.cabeca++;
		if(fila.cabeca == TAM_USART)
			fila.cabeca = 0;

		if(fila.cabeca == fila.cauda){
			fila.flag = 0;
			fila.cabeca = 0;
			fila.cauda = 0;
			fila.empty = 1;
		}
		return valor;
	}else
		return 0;

}

uint8_t filaVazia(){

	return fila.empty;
}
