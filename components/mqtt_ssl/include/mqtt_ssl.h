/*
 * @Author: Breakfast759
 * @Date: 2022-05-19 16:32:29
 * @LastEditors: Breakfast759
 * @LastEditTime: 2022-05-22 16:12:00
 * @FilePath: \8_Version2.3.0.20210519_Base\components\mqtt_ssl\include\mqtt_ssl.h
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

#define ONENET_BROKER_URI_SSL               "mqtts://mqttstls.heclouds.com:8883"
#define ONENET_HOST_SSL                     "mqttstls.heclouds.com"
#define ONENET_PORT_SSL                     8883

#define ONENET_BROKER_URI                   "mqtt://mqtts.heclouds.com:1883"
#define ONENET_HOST                         "mqtts.heclouds.com"
#define ONENET_PORT                         1883

#define CONFIG_BROKER_BIN_SIZE_TO_SEND      20000
#define DEVICE_ID                           "502403"
#define DEVICE_NAME                         "HelloBug_DevBoard"
#define PASSWORD                            "version=2018-10-31&res=products%2F502403%2Fdevices%2FHelloBug_DevBoard&et=1684543751&method=md5&sign=m%2BdkupFZAdU9IcC3jIPQhw%3D%3D"

#define ONENET_TOPIC_DP_PUBLISH             "$sys/502403/HelloBug_DevBoard/dp/post/json"
#define ONENET_TOPIC_DP_SUB                 "$sys/502403/HelloBug_DevBoard/dp/post/json/+"
#define ONENET_TOPIC_CMD_SUB                "$sys/502403/HelloBug_DevBoard/cmd/#"

#define ONENET_POST_BODY_FORMAT             "{\"id\":123,\"dp\":%s}"

esp_mqtt_client_handle_t client;

static const char *TAG = "MQTTS_EXAMPLE";

void mqtt_app_start(void);

#endif
