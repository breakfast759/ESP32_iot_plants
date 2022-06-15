/*
 * @Author: Breakfast759
 * @Date: 2022-05-23 19:54:55
 * @LastEditors: Breakfast759
 * @LastEditTime: 2022-05-31 23:35:26
 * @FilePath: \11_Version1.0.1.20220531_RC\components\pump\pump.c
 * @Description: 
 * 
 * Copyright (c) 2022 by 514 Anti-Rolling Department, All Rights Reserved. 
 */
#include <stdio.h>
#include "pump.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/ledc.h"

static const char *TAG = "pump";

/* Use project configuration menu (idf.py menuconfig) to choose the GPIO to blink,
   or you can edit the following line and set a number here.
*/
#define PUMP_GPIO 16

// static uint8_t s_pump_state = 1;
// 定时器的配置
static ledc_timer_config_t ledc_timer = { 
		duty_resolution : LEDC_TIMER_13_BIT,	// PWM 占空比分辨率
		freq_hz : 5000,						    // PWM 信号频率
		speed_mode : LEDC_HIGH_SPEED_MODE,		// 高速模式
		timer_num : LEDC_TIMER_1				// 使用定时器0
	};
// LEDC通道配置
static ledc_channel_config_t ledc_channel = {
			.channel =	LEDC_HS_CH1_CHANNEL,
			.duty = 0, 
			.gpio_num = PUMP_GPIO,
			.speed_mode = LEDC_HIGH_SPEED_MODE,
			.timer_sel = LEDC_TIMER_1 
	};

void pump_config(void)
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
    ESP_LOGI(TAG, "Configure PUMP Component successfully!");
    pump_set_pwm(100);
    // 如上初始化，gpio_set_level好像就没用了，要用led_set_lux
}

void pump_set_pwm(uint16_t duty)
{
    duty = (100 - duty) * 8191 / 100;
    // ESP_LOGI(TAG, "Current pump duty is: %d", duty);
    ledc_set_duty_and_update(ledc_channel.speed_mode, ledc_channel.channel, duty, 0xFFFFF);
}
