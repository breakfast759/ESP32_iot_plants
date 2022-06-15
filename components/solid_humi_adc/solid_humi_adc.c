/*
 * @Author: Breakfast759
 * @Date: 2022-05-19 15:50:20
 * @LastEditors: Breakfast759
 * @LastEditTime: 2022-06-06 12:59:05
 * @FilePath: \11_Version1.0.1.20220531_RC\components\solid_humi_adc\solid_humi_adc.c
 * @Description: 
 * 
 * Copyright (c) 2022 by 514 Anti-Rolling Department, All Rights Reserved. 
 */
#include <stdio.h>
#include "solid_humi_adc.h"

// adc斜率曲线
static esp_adc_cal_characteristics_t *adc_chars;

static const adc_channel_t channel = ADC_CHANNEL_7;     // GPIO35 if ADC1, GPIO27 if ADC2
static const adc_bits_width_t width = ADC_WIDTH_BIT_12;  // 设置分辨率

static const adc_atten_t atten = ADC_ATTEN_DB_11;
static const adc_unit_t unit = ADC_UNIT_1;

static void print_char_val_type(esp_adc_cal_value_t val_type)
{
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        printf("Characterized using Two Point Value\n");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        printf("Characterized using eFuse Vref\n");
    } else {
        printf("Characterized using Default Vref\n");
    }
}

void solidHumi_config(void)
{
    //Check if TP is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
        printf("eFuse Two Point: Supported\n");
    } else {
        printf("eFuse Two Point: NOT supported\n");
    }
    //Check Vref is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK) {
        printf("eFuse Vref: Supported\n");
    } else {
        printf("eFuse Vref: NOT supported\n");
    }
    //Configure ADC
    if (unit == ADC_UNIT_1) {
        adc1_config_width(width);
        /* Vref 是 ESP32 ADC 内部用于测量输入电压的参考电压。 
	     * ESP32 ADC 可以测量从 0 V 到 Vref 的模拟电压。 
	     * 在不同的芯片中，Vref 不同，中位数为 1.1 V。为了转换大于 Vref 的电压，可以在输入 ADC 之前对输入电压进行衰减。 
	     * 有 4 种可用的衰减选项，衰减越高，可测量的输入电压就越高。
	     * 这里的ADC_ATTEN_DB_11代表可测量的电压范围为50 mV ~ 2450 mV                                          */
        adc1_config_channel_atten(channel, atten);
    } else {
        adc2_config_channel_atten((adc2_channel_t)channel, atten);
    }

    //Characterize ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, width, DEFAULT_VREF, adc_chars);
    print_char_val_type(val_type);
}

uint32_t solidHumi_read()
{
    uint32_t adc_reading = 0;
    //Multisampling
    for (int i = 0; i < NO_OF_SAMPLES; i++) {
        if (unit == ADC_UNIT_1) {
            adc_reading += adc1_get_raw((adc1_channel_t)channel);
        } else {
            int raw;
            adc2_get_raw((adc2_channel_t)channel, width, &raw);
            adc_reading += raw;
        }
    }
    adc_reading /= NO_OF_SAMPLES;
    //Convert adc_reading to voltage in mV
    uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
    // printf("Raw: %d\tVoltage: %dmV\n", adc_reading, voltage);
    // vTaskDelay(pdMS_TO_TICKS(1000));
    return voltage;
}
