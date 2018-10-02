#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "bits.h"
#include "avr_gpio.h"
#include "avr_spi.h"

volatile filaSPI_t fila;

static inline void enqueue(uint8_t dado);
static inline uint8_t dequeue();
static inline uint8_t filaVazia();

void SPI_MasterInit(void){

	GPIO_B->DDR = SET(PB1) | SET(PB2) | SET(MOSI) | SET(SCK);

	 // clock = F_CPU / 16
	SPI->SP_CR.MASK = SET(SPE) | SET(MSTR) | SET(SPR0) | SET(CPHA);
	SPI->SP_SR.MASK =  SET(SPIF);

	fila.flag = 0;
	fila.cauda = 0;
	fila.empty = 1;
}

void SPI_SlaveInit(void){

	GPIO_B->DDR = SET(MISO); // Set MISO output, all others input
	SPI->SP_CR.MASK = SET(SPE) | SET(CPHA) | SET(SPIE); // Enable SPI

	fila.flag = 0;
	fila.cauda = 0;
	fila.empty = 1;
}

uint8_t SPI_MasterReceive(uint8_t byte){

	SPI->SP_DR = byte;
	while (!SPI->SP_SR.SP_IF);

	return SPI->SP_DR;

}

uint8_t SPI_SlaveReceive(void){

	while (!SPI->SP_SR.SP_IF); // Wait for reception complete
	return SPI->SP_DR; // Return Data Register

}


uint8_t spiFlag(){
	return (fila.flag);
}

void spiCopy(uint8_t* vetor, uint8_t n){

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

ISR(SPI_STC_vect){

	uint8_t c = SPI->SP_DR;

	if(c != CHAR_SPI)
		enqueue(c);
	else
		fila.flag = 1;


}

/* FILA */
static inline void enqueue(uint8_t dado){

	fila.buffer[fila.cauda] = dado;

	if((fila.cabeca == fila.cauda) && !(fila.empty)){
		fila.cabeca++;
		if(fila.cabeca == TAM_SPI)
			fila.cabeca = 0;
	}
	fila.cauda++;
	fila.empty = 0;
	if(fila.cauda == TAM_SPI){
		fila.cauda = 0;
	}

}

static inline uint8_t dequeue(){

	if(!fila.empty){
		uint8_t valor = fila.buffer[fila.cabeca];
		fila.cabeca++;
		if(fila.cabeca == TAM_SPI)
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

static inline uint8_t filaVazia(){

	return fila.empty;
}
