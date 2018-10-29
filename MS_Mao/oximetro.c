#include "oximetro.h"

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
