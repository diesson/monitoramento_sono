/* struct dht22 AVR Lirary
 *
 * Copyright (C) 2015 Sergey Denisov.
 * Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com)
 *
 */

#ifndef __DHT22_H__
#define __DHT22_H__

#include <stdint.h>

/*
 * Sensor's port
 */
#define DDR_DHT DDRB
#define PORT_DHT PORTB
#define PIN_DHT PINB

typedef struct dht22 {
    uint8_t data[6];    /* data from sensor store here */
    uint8_t pin;        /* DDR & PORT pin */
} dht22_t;

/**
 * Init dht sensor
 * @dht: sensor struct
 * @pin: PORT & DDR pin
 */
void dht_init(dht22_t* dht, uint8_t pin);

/**
 * Reading temperature from sensor
 * @dht: sensor struct
 * @temp: out temperature pointer
 *
 * Returns 1 if succeful reading
 * Returns 0 if fail reading
 */
uint8_t dht_read_temp(dht22_t* dht, uint16_t *temp);

/**
 * Reading humidity from sensor
 * @dht: sensor struct
 * @hum: out humidity pointer
 *
 * Returns 1 if succeful reading
 * Returns 0 if fail reading
 */
uint8_t dht_read_hum(dht22_t* dht, uint16_t *hum);

/**
 * Reading temperature and humidity from sensor
 * @dht: sensor struct
 * @temp: out temperature pointer
 * @hum: out humidity pointer
 *
 * Returns 1 if succeful reading
 * Returns 0 if fail reading
 *
 * The fastest function for getting temperature + humidity.
 */
uint8_t dht_read_data(dht22_t* dht, uint16_t* temp, uint16_t* hum);


#endif
