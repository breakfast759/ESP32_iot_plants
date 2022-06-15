/*
 * @Author: Breakfast759
 * @Date: 2022-05-19 16:32:29
 * @LastEditors: Breakfast759
 * @LastEditTime: 2022-06-15 23:14:25
 * @FilePath: \ESP32_iot_plants\components\mqtt_ssl\include\mqtt_ssl.h
 * @Description: 
 * 
 * Copyright (c) 2022 by 514 Anti-Rolling Department, All Rights Reserved. 
 */
/**
 * @file solid_humi_adc.h
 *
 */

#ifndef _MQTT_SSL_H_
#define _MQTT_SSL_H_

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"

#include "esp_log.h"
#include "mqtt_client.h"
#include "esp_tls.h"
#include "esp_ota_ops.h"
#include <sys/param.h>

#define TEST_URI                            "mqtts://mqtt.eclipseprojects.io:8883"

/* ONENET MQTTS 加密连接，此程序中没有用（CA认证未处理），请使用非加密连接 */
#define ONENET_BROKER_URI_SSL               "mqtts://mqttstls.heclouds.com:8883"
#define ONENET_HOST_SSL                     "mqttstls.heclouds.com"
#define ONENET_PORT_SSL                     8883

/* ONENET MQTTS 非加密连接 */
#define ONENET_BROKER_URI                   "mqtt://mqtts.heclouds.com:1883"
#define ONENET_HOST                         "mqtts.heclouds.com"
#define ONENET_PORT                         1883

#define CONFIG_BROKER_BIN_SIZE_TO_SEND      20000
#define DEVICE_ID                           "your device id"
#define DEVICE_NAME                         "your debice name"
#define PASSWORD                            "your password"

#define ONENET_TOPIC_DP_PUBLISH             "$sys/your device id/your debice name/dp/post/json"
#define ONENET_TOPIC_DP_SUB                 "$sys/your device id/your debice name/dp/post/json/+"
#define ONENET_TOPIC_CMD_SUB                "$sys/your device id/your debice name/cmd/#"

#define ONENET_POST_BODY_FORMAT             "{\"id\":123,\"dp\":%s}"

esp_mqtt_client_handle_t client;

static const char *TAG = "MQTTS_EXAMPLE";

void mqtt_app_start(void);

#endif
