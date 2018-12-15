#ifndef _USART_H
#define _USART_H

#include <stdint.h>
#include <avr/io.h>

#ifndef _IO
#define _IO volatile uint8_t
#endif

#ifndef _IO16
#define _IO16 volatile uint16_t
#endif

/* Baudrates */
#define B9600  (F_CPU/16/9600-1)
#define B57600  (F_CPU/16/57600-1)
#define B115200  (F_CPU/16/115200-1)

#define TAM_USART 16
#define CHAR_USART '\r'

/* Registers structure definition */
typedef struct{

	_IO UCSR_A;   	/*!< USART Control and Status Register A */
	_IO UCSR_B;		/*!< USART Control and Status Register B */
	_IO UCSR_C;		/*!< USART Control and Status Register C */
	_IO unused;
	_IO UBRR_L;   	/*!< USART Baud Rate Low Register */
	_IO UBRR_H;		/*!< USART Baud Rate High Register */
	_IO UDR_;		/*!< USART I/O Data Register  */

}UART_Type;

typedef struct{
	uint8_t buffer[TAM_USART];
	uint8_t cabeca;
	uint8_t cauda;
	uint8_t empty;
	uint8_t flag;
}fila_t;

#define USART_0 ((UART_Type *) &UCSR0A)

void USART_Init(uint16_t bauds);
FILE * get_usart_stream();

void USART_tx(uint8_t data);
uint8_t USART_rx();

/* Adicionei */
uint8_t dequeue();
//void enqueue(uint8_t dado);
uint8_t usartFlag();
uint8_t filaVazia();
void usartCopy(uint8_t* vetor, uint8_t n);

#endif
