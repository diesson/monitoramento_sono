#ifndef SPI_H_
#define SPI_H_

#include <stdint.h>
#include <avr/io.h>

#ifndef _IO
#define _IO volatile uint8_t
#endif

#ifndef _IO16
#define _IO16 volatile uint16_t
#endif

#define SS PB2
#define MOSI PB3
#define MISO PB4
#define SCK PB5
#define TAM_SPI 16
#define CHAR_SPI '.'

typedef struct{
	uint8_t buffer[TAM_SPI];
	uint8_t cabeca;
	uint8_t cauda;
	uint8_t empty;
	uint8_t flag;
}filaSPI_t;

/* Registers structure definition */
typedef struct
{
	/*!< SPI Control Register */
	union {
		_IO MASK;
		struct {
			_IO SPR_0:1;	/*!< SPR1, SPR0: SPI Clock Rate Select 1 and 0 */
			_IO SPR_1:1;
			/*!<  SPI2X	 SPR1	 SPR0	SCK freq  (SPI2X @ SP_SR register)
			 *     0	  0	       0	 fosc/4
			 *     0	  0        1	 fosc/16
			 *     0	  1        0	 fosc/64
			 *     0	  1        1	 fosc/128
			 *     1	  0    	   0	 fosc/2
			 *     1	  0        1	 fosc/8
			 *     1      1        0	 fosc/32
			 *     1      1 	   1	 fosc/64  */
			_IO C_PHA:1;	/*!< Clock phase */
			_IO C_POL:1;	/*!< Clock polarity */
			_IO MS_TR:1;	/*!< Master/Slave select */
			_IO D_ORD:1;	/*!< Data order */
			_IO SP_E:1;		/*!< SPI Enable */
			_IO SP_IE:1;	/*!< SPI IRQ  Enable */
		} BITS;
	} SP_CR;

	/*!< SPI Status Register  */
	union {
		_IO MASK;
		struct {
			_IO SP_I2X:1;	/*!< SPI Double speed */
			_IO unused:5;
			_IO W_COL:1;	/*!< Write colision flag */
			_IO SP_IF:1;	/*!< SPI IRQ flag */
		};
	} SP_SR;


	_IO SP_DR;		/*!< SPI Data Register    */

} SPI_Type;

#define SPI ((SPI_Type *) &SPCR)

void SPI_MasterInit(void);
void SPI_SlaveInit(void);
uint8_t SPI_MasterReceive(uint8_t byte);
uint8_t SPI_SlaveReceive(void);

uint8_t spiFlag();
void spiCopy(uint8_t* vetor, uint8_t n);

#endif /* SPI_H_ */
