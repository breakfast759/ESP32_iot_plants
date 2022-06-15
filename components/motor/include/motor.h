/*
 * @Author: Breakfast759
 * @Date: 2022-05-27 16:49:46
 * @LastEditors: Breakfast759
 * @LastEditTime: 2022-05-27 16:53:14
 * @FilePath: \9_Version1.0.0.20220523_Alpha\components\motor\include\motor.h
 * @Description: 
 * 
 * Copyright (c) 2022 by 514 Anti-Rolling Department, All Rights Reserved. 
 */
#ifndef _MOTOR_H_
#define _MOTOR_H_

#define LEDC_HS_CH2_CHANNEL		LEDC_CHANNEL_2

#define LEDC_TEST_DUTY			8000

void motor_config(void);
void motor_set_pwm(uint16_t duty);

#endif
