/*
 * @Author: Breakfast759
 * @Date: 2022-05-23 21:09:24
 * @LastEditors: Breakfast759
 * @LastEditTime: 2022-05-28 13:34:38
 * @FilePath: \9_Version1.0.0.20220523_Alpha\components\wifi_connection\include\wifi_connection.h
 * @Description: 
 * 
 * Copyright (c) 2022 by 514 Anti-Rolling Department, All Rights Reserved. 
 */
/**
 * @file wifi_connection.h
 *
 */

#ifndef _WIFI_CONNECTION_H_
#define _WIFI_CONNECTION_H_

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"

#include "lwip/err.h"
#include "lwip/sys.h"

/*********************
 *      DEFINES
 *********************/
/* The examples use WiFi configuration that you can set via project configuration menu

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define EXAMPLE_ESP_WIFI_SSID      "12306"
#define EXAMPLE_ESP_WIFI_PASS      "zc20010623"
#define EXAMPLE_ESP_MAXIMUM_RETRY  3
/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void wifi_init_sta(bool flag, const char * wifi_ssid, const char * wifi_password);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*UI_DISPLAY_H*/
