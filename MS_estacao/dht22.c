/* Fast DHT Lirary
 *
 * Copyright (C) 2015 Sergey Denisov.
 * Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com)
 *
 */

#include "dht22.h"
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define DHT_COUNT 6
#define DHT_MAXTIMINGS 85


void dht_init(dht22_t* dht, uint8_t pin)
{
    dht->pin |= pin;
    /* Setup the pins! */
    DDR_DHT &= ~(1 << dht->pin);
    PORT_DHT |= (1 << dht->pin);
}

static uint8_t dht_read(dht22_t* dht)
{
    uint8_t tmp;
    uint8_t sum = 0;
    uint8_t j = 0, i;
    uint8_t last_state = 1;
    uint16_t counter = 0;
    /*
     * Pull the pin 1 and wait 250 milliseconds
     */
    PORT_DHT |= (1 << dht->pin);
    _delay_ms(250);

    dht->data[0] = dht->data[1] = dht->data[2] = dht->data[3] = dht->data[4] = 0;

    /* Now pull it low for ~20 milliseconds */
    DDR_DHT |= (1 << dht->pin);
    PORT_DHT &= ~(1 << dht->pin);
    _delay_ms(20);
    cli();
    PORT_DHT |= (1 << dht->pin);
    _delay_us(40);
    DDR_DHT &= ~(1 << dht->pin);

    /* Read the timings */
    for (i = 0; i < DHT_MAXTIMINGS; i++) {
        counter = 0;
        while (1) {
            tmp = ((PIN_DHT & (1 << dht->pin)) >> 1);
            _delay_us(3);

            if (tmp != last_state)
                break;

            counter++;
            _delay_us(1);

            if (counter == 255)
                break;
        }

        last_state = ((PIN_DHT & (1 << dht->pin)) >> 1);

        if (counter == 255)
            break;

        /* Ignore first 3 transitions */
        if ((i >= 4) && (i % 2 == 0)) {
            /* Shove each bit into the storage bytes */
            dht->data[j/8] <<= 1;
            if (counter > DHT_COUNT)
                dht->data[j/8] |= 1;
            j++;
        }
    }

    sei();
    sum = dht->data[0] + dht->data[1] + dht->data[2] + dht->data[3];

    if ((j >= 40) && (dht->data[4] == (sum & 0xFF)))
        return 1;
    return 0;
}

uint8_t dht_read_temp(dht22_t* dht, uint16_t* temp)
{
    if (dht_read(dht))
    {
    	*temp = dht->data[2];
    	*temp <<= 8;
    	*temp += dht->data[3];

        return 1;
    }
    return 0;
}

uint8_t dht_read_hum(dht22_t* dht, uint16_t* hum)
{
    if (dht_read(dht))
    {
    	*hum = dht->data[0];
		*hum <<= 8;
		*hum += dht->data[1];

		if (*hum == 0)
			return 0;

        return 1;
    }
    return 0;
}

uint8_t dht_read_data(dht22_t* dht, uint16_t* temp, uint16_t* hum)
{
    if (dht_read(dht)) {
        /* Reading temperature */
    	*temp = dht->data[2];
		*temp <<= 8;
		*temp += dht->data[3];

        /* Reading humidity */
        *hum = dht->data[0];
		*hum <<= 8;
		*hum += dht->data[1];

        if (*hum == 0)
            return 0;

        return 1;
    }
    return 0;
}


