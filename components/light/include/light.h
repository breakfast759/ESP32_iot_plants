/*
 * @Author: Breakfast759
 * @Date: 2022-05-27 16:59:14
 * @LastEditors: Breakfast759
 * @LastEditTime: 2022-05-27 16:59:33
 * @FilePath: \9_Version1.0.0.20220523_Alpha\components\light\include\light.h
 * @Description: 
 * 
 * Copyright (c) 2022 by 514 Anti-Rolling Department, All Rights Reserved. 
 */
#ifndef _LIGHT_H_
#define _LIGHT_H_

#define LEDC_HS_CH3_CHANNEL		LEDC_CHANNEL_3

#define LEDC_TEST_DUTY			8000

void light_config(void);
void light_set_pwm(uint16_t duty);

#endif
