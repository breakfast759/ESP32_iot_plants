#ifndef _DHT11_H_
#define _DHT11_H_

#include <driver/rmt.h>
#include <soc/rmt_reg.h>
#include "driver/gpio.h" 

#define DHT11_GPIO 26

void dht11_init(uint8_t dht11_pin);
int dht11_start_get(int *temperature, int *humidity);

#endif