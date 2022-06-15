#include <stdio.h>
#include "LED.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "led_strip.h"
#include "driver/ledc.h"

static const char *TAG = "LED";

/* Use project configuration menu (idf.py menuconfig) to choose the GPIO to blink,
   or you can edit the following line and set a number here.
*/
#define BLINK_GPIO CONFIG_BLINK_GPIO

static uint8_t s_led_state = 1;
// 定时器的配置
static ledc_timer_config_t ledc_timer = { 
		duty_resolution : LEDC_TIMER_13_BIT,	// PWM 占空比分辨率
		freq_hz : 5000,						    // PWM 信号频率
		speed_mode : LEDC_HIGH_SPEED_MODE,		// 高速模式
		timer_num : LEDC_TIMER_0				// 使用定时器0
	};
// LEDC通道配置
static ledc_channel_config_t ledc_channel = {
			.channel =	LEDC_HS_CH0_CHANNEL,
			.duty = 0, 
			.gpio_num = BLINK_GPIO,
			.speed_mode = LEDC_HIGH_SPEED_MODE,
			.timer_sel = LEDC_TIMER_0 
	};
#ifdef CONFIG_BLINK_LED_RMT
static led_strip_t *pStrip_a;

void led_config(void)
{
    ESP_LOGI(TAG, "Example configured to blink addressable LED!");
    /* LED strip initialization with the GPIO and pixels number*/
    pStrip_a = led_strip_init(CONFIG_BLINK_LED_RMT_CHANNEL, BLINK_GPIO, 1);
    /* Set all LED off to clear all pixels */
    pStrip_a->clear(pStrip_a, 50);
}

void blink_led(void)
{
    /* If the addressable LED is enabled */
    if (s_led_state) {
        /* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
        pStrip_a->set_pixel(pStrip_a, 0, 16, 16, 16);
        /* Refresh the strip to send data */
        pStrip_a->refresh(pStrip_a, 100);
    } else {
        /* Set all LED off to clear all pixels */
        pStrip_a->clear(pStrip_a, 50);
    }
}

#elif CONFIG_BLINK_LED_GPIO
void led_config(void)
{
    // 初始化LEDC定时器配置
	ledc_timer_config(&ledc_timer);
    // 将前面的的配置设置到 LED 控制器
	ledc_channel_config(&ledc_channel);
    // 初始化淡入淡出服务
	ledc_fade_func_install(0);
    // reset GPIO配置会使之前的LEDC配置失效
    // gpio_reset_pin(BLINK_GPIO);
    /* Set the GPIO as a push/pull output */
    // gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
    ESP_LOGI(TAG, "Configure LED Component successfully!");
    led_set_lux(0);
    // 如上初始化，gpio_set_level好像就没用了，要用led_set_lux
}

void blink_led(void)
{
    /* Set the GPIO level according to the state (LOW or HIGH)*/
    gpio_set_level(BLINK_GPIO, s_led_state);

    while (1) {
        ESP_LOGI(TAG, "Turning the LED %s!", s_led_state == true ? "ON" : "OFF");
        /* Set the GPIO level according to the state (LOW or HIGH)*/
        gpio_set_level(BLINK_GPIO, s_led_state);
        /* Toggle the LED state */
        s_led_state = !s_led_state;
        vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
    }

}

void led_set(uint8_t state)
{
    gpio_set_level(BLINK_GPIO, state);
}

void led_set_lux(uint16_t duty)
{
    duty = (100-duty)*8191/100;
    // ESP_LOGI(TAG, "Current duty is: %d", duty);
    ledc_set_duty_and_update(ledc_channel.speed_mode, ledc_channel.channel, duty, 0xFFFFF);
}
#endif
