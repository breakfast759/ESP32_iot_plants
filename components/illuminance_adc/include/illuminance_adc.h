/*
 * @Author: Breakfast759
 * @Date: 2022-05-23 19:24:30
 * @LastEditors: Breakfast759
 * @LastEditTime: 2022-05-23 19:26:05
 * @FilePath: \8_Version2.3.0.20210519_Base\components\illuminance_adc\include\illuminance_adc.h
 * @Description: 
 * 
 * Copyright (c) 2022 by 514 Anti-Rolling Department, All Rights Reserved. 
 */
/**
 * @file illuminance_adc.h
 *
 */

#ifndef _ILLUMINANCE_ADC_H_
#define _ILLUMINANCE_ADC_H_

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "esp_log.h"

/*********************
 *      DEFINES
 *********************/
#define DEFAULT_VREF    3300        //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   64          //Multisampling

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void illuminance_config(void);
uint32_t illuminance_read(void);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*UI_DISPLAY_H*/
