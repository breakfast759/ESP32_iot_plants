/*
 * @Author: Breakfast759
 * @Date: 2022-05-23 19:54:55
 * @LastEditors: Breakfast759
 * @LastEditTime: 2022-05-23 19:56:39
 * @FilePath: \8_Version2.3.0.20210519_Base\components\pump\include\pump.h
 * @Description: 
 * 
 * Copyright (c) 2022 by 514 Anti-Rolling Department, All Rights Reserved. 
 */
#ifndef _PUMP_H_
#define _PUMP_H_

#define LEDC_HS_CH1_CHANNEL		LEDC_CHANNEL_1

#define LEDC_TEST_DUTY			8000

void pump_config(void);
void pump_set_pwm(uint16_t duty);

#endif
