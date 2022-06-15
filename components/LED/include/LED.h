#ifndef _LED_H_
#define _LED_H_

#define LEDC_HS_CH0_CHANNEL		LEDC_CHANNEL_0

#define LEDC_TEST_DUTY			8000

void led_config(void);
void blink_led(void);
void led_set(uint8_t state);
void led_set_lux(uint16_t duty);

#endif
