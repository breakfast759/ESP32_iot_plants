/*
 * @Author: Breakfast759
 * @Date: 2022-05-19 16:32:29
 * @LastEditors: Breakfast759
 * @LastEditTime: 2022-05-31 23:39:52
 * @FilePath: \11_Version1.0.1.20220531_RC\components\mqtt_ssl\mqtt_ssl.c
 * @Description: 
 * 
 * Copyright (c) 2022 by 514 Anti-Rolling Department, All Rights Reserved. 
 */
#include <stdio.h>
#include "mqtt_ssl.h"

#if CONFIG_BROKER_CERTIFICATE_OVERRIDDEN == 1
static const uint8_t mqtt_eclipseprojects_io_pem_start[]  = "-----BEGIN CERTIFICATE-----\n" CONFIG_BROKER_CERTIFICATE_OVERRIDE "\n-----END CERTIFICATE-----";
#else
extern const uint8_t mqtt_eclipseprojects_io_pem_start[]   asm("_binary_mqtt_eclipseprojects_io_pem_start");
#endif
extern const uint8_t mqtt_eclipseprojects_io_pem_end[]   asm("_binary_mqtt_eclipseprojects_io_pem_end");

extern const uint8_t MQTTS_certificate_pem_start[]   asm("_binary_MQTTS_certificate_pem_start");
extern const uint8_t MQTTS_certificate_pem_end[]   asm("_binary_MQTTS_certificate_pem_end");

bool cloud_control_flag = 0;
bool motor_status = 0;
bool motor_mannual_flag = 0;
bool light_status = 0;
bool light_mannual_flag = 0;
bool pump_status = 0;
bool pump_mannual_flag = 0;
int temp_low_threshold_get = 50;
int light_high_threshold_get = 50;
int soil_humi_low_threshold_get = 0;
int soil_humi_high_threshold_get = 50;

//
// Note: this function is for testing purposes only publishing part of the active partition
//       (to be checked against the original binary)
//
static void cmd_response(esp_mqtt_client_handle_t client)
{

}

/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    client = event->client;
    int msg_id;
    char cmdbuf[20];
    char topicbuf[100];
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        // msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
        // ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        /* ?????????????????????????????? */
        // ??????????????????????????????????????????????????????????????????????????????????????? topic ?????????????????????????????????
        msg_id = esp_mqtt_client_subscribe(client, ONENET_TOPIC_DP_SUB, 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        /* ??????????????????topic */
        msg_id = esp_mqtt_client_subscribe(client, ONENET_TOPIC_CMD_SUB, 1);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");

        // ?????????.??????*??????????????????????????????????????????????????????event->topic_len???
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);

        sprintf(cmdbuf,"%.*s", event->data_len, event->data);
        if(strstr(cmdbuf, ":motor_1")){
            cloud_control_flag = 1;
            motor_mannual_flag = !motor_mannual_flag;
            motor_status = 1;

            // ??????topic
            sprintf(topicbuf, "%.*s", event->topic_len, event->topic);
            // ???request topic???response topic
            int i;
            for(i = 99; i > 42; i--)
            {
                topicbuf[i] = topicbuf[i-1];
            }
            topicbuf[34] = 'r';
            topicbuf[35] = 'e';
            topicbuf[36] = 's';
            topicbuf[37] = 'p';
            topicbuf[38] = 'o';
            topicbuf[39] = 'n';
            topicbuf[40] = 's';
            topicbuf[41] = 'e';
            topicbuf[42] = '/';
            // ??????topic??????????????????
            printf("DemoRes=%s\r\n", topicbuf);
            // ????????????????????????
            ESP_LOGI(TAG, "Motor ON!");
            // ???????????????????????????????????????
            msg_id = esp_mqtt_client_publish(client, topicbuf, "Li Xincong Superman" , 0, 1, 0);
            // ??????????????????
            ESP_LOGI(TAG, "sent cmd response successful, msg_id=%d", msg_id);
        }
        else if(strstr(cmdbuf, ":motor_0")){
            cloud_control_flag = 1;
            motor_mannual_flag = !motor_mannual_flag;
            motor_status = 0;

            sprintf(topicbuf, "%.*s", event->topic_len, event->topic);
            int i;
            for(i = 99; i > 42; i--)
            {
                topicbuf[i] = topicbuf[i-1];
            }
            topicbuf[34] = 'r';
            topicbuf[35] = 'e';
            topicbuf[36] = 's';
            topicbuf[37] = 'p';
            topicbuf[38] = 'o';
            topicbuf[39] = 'n';
            topicbuf[40] = 's';
            topicbuf[41] = 'e';
            topicbuf[42] = '/';
            printf("DemoRes=%s\r\n", topicbuf);
            ESP_LOGI(TAG, "Motor OFF!");
            msg_id = esp_mqtt_client_publish(client, topicbuf, "Li Xincong Superman" , 0, 1, 0);
            ESP_LOGI(TAG, "sent cmd response successful, msg_id=%d", msg_id);
        }
        else if(strstr(cmdbuf, ":light_1")){
            cloud_control_flag = 1;
            light_mannual_flag = !light_mannual_flag;
            light_status = 1;

            sprintf(topicbuf, "%.*s", event->topic_len, event->topic);
            int i;
            for(i = 99; i > 42; i--)
            {
                topicbuf[i] = topicbuf[i-1];
            }
            topicbuf[34] = 'r';
            topicbuf[35] = 'e';
            topicbuf[36] = 's';
            topicbuf[37] = 'p';
            topicbuf[38] = 'o';
            topicbuf[39] = 'n';
            topicbuf[40] = 's';
            topicbuf[41] = 'e';
            topicbuf[42] = '/';
            printf("DemoRes=%s\r\n", topicbuf);
            ESP_LOGI(TAG, "Light ON!");
            msg_id = esp_mqtt_client_publish(client, topicbuf, "Li Xincong Superman" , 0, 1, 0);
            ESP_LOGI(TAG, "sent cmd response successful, msg_id=%d", msg_id);
        }
        else if(strstr(cmdbuf, ":light_0")){
            cloud_control_flag = 1;
            light_mannual_flag = !light_mannual_flag;
            light_status = 1;

            sprintf(topicbuf, "%.*s", event->topic_len, event->topic);
            int i;
            for(i = 99; i > 42; i--)
            {
                topicbuf[i] = topicbuf[i-1];
            }
            topicbuf[34] = 'r';
            topicbuf[35] = 'e';
            topicbuf[36] = 's';
            topicbuf[37] = 'p';
            topicbuf[38] = 'o';
            topicbuf[39] = 'n';
            topicbuf[40] = 's';
            topicbuf[41] = 'e';
            topicbuf[42] = '/';
            printf("DemoRes=%s\r\n", topicbuf);
            ESP_LOGI(TAG, "Light OFF!");
            msg_id = esp_mqtt_client_publish(client, topicbuf, "Li Xincong Superman" , 0, 1, 0);
            ESP_LOGI(TAG, "sent cmd response successful, msg_id=%d", msg_id);
        }
        else if(strstr(cmdbuf, ":pump_1")){
            cloud_control_flag = 1;
            pump_mannual_flag = !pump_mannual_flag;
            pump_status = 1;

            sprintf(topicbuf, "%.*s", event->topic_len, event->topic);
            int i;
            for(i = 99; i > 42; i--)
            {
                topicbuf[i] = topicbuf[i-1];
            }
            topicbuf[34] = 'r';
            topicbuf[35] = 'e';
            topicbuf[36] = 's';
            topicbuf[37] = 'p';
            topicbuf[38] = 'o';
            topicbuf[39] = 'n';
            topicbuf[40] = 's';
            topicbuf[41] = 'e';
            topicbuf[42] = '/';
            printf("DemoRes=%s\r\n", topicbuf);
            ESP_LOGI(TAG, "Pump ON!");
            msg_id = esp_mqtt_client_publish(client, topicbuf, "Li Xincong Superman" , 0, 1, 0);
            ESP_LOGI(TAG, "sent cmd response successful, msg_id=%d", msg_id);
        }
        else if(strstr(cmdbuf, ":pump_0")){
            cloud_control_flag = 1;
            pump_mannual_flag = !pump_mannual_flag;
            pump_status = 0;

            sprintf(topicbuf, "%.*s", event->topic_len, event->topic);
            int i;
            for(i = 99; i > 42; i--)
            {
                topicbuf[i] = topicbuf[i-1];
            }
            topicbuf[34] = 'r';
            topicbuf[35] = 'e';
            topicbuf[36] = 's';
            topicbuf[37] = 'p';
            topicbuf[38] = 'o';
            topicbuf[39] = 'n';
            topicbuf[40] = 's';
            topicbuf[41] = 'e';
            topicbuf[42] = '/';
            printf("DemoRes=%s\r\n", topicbuf);
            ESP_LOGI(TAG, "Pump OFF!");
            msg_id = esp_mqtt_client_publish(client, topicbuf, "Li Xincong Superman" , 0, 1, 0);
            ESP_LOGI(TAG, "sent cmd response successful, msg_id=%d", msg_id);
        }
        else if(strstr(cmdbuf, "temp_low_threshold")){
            cloud_control_flag = 1;
            // ?????????????????????????????????????????????????????????????????????emmm...
            // ???????????????????????????
            printf("CMDDATA=%s\r\n", cmdbuf);
            // ???????????????????????????????????????valuebuf???
            char valuebuf[5];
            valuebuf[0] = cmdbuf[19];
            valuebuf[1] = cmdbuf[20];
            valuebuf[2] = cmdbuf[21];
            // char?????????int
            temp_low_threshold_get = atoi(valuebuf);
            
            sprintf(topicbuf, "%.*s", event->topic_len, event->topic);
            int i;
            for(i = 99; i > 42; i--)
            {
                topicbuf[i] = topicbuf[i-1];
            }
            topicbuf[34] = 'r';
            topicbuf[35] = 'e';
            topicbuf[36] = 's';
            topicbuf[37] = 'p';
            topicbuf[38] = 'o';
            topicbuf[39] = 'n';
            topicbuf[40] = 's';
            topicbuf[41] = 'e';
            topicbuf[42] = '/';
            
            // ?????????????????????????????????
            // printf("temp_low_threshold_get=%d\r\n", temp_low_threshold_get);
            msg_id = esp_mqtt_client_publish(client, topicbuf, "Li Xincong Superman" , 0, 1, 0);
        }
        else if(strstr(cmdbuf, "light_high_threshold")){
            cloud_control_flag = 1;
            // ???????????????????????????
            printf("CMDDATA=%s\r\n", cmdbuf);
            // ???????????????????????????????????????valuebuf???
            char valuebuf[5];
            valuebuf[0] = cmdbuf[21];
            valuebuf[1] = cmdbuf[22];
            valuebuf[2] = cmdbuf[23];
            // char?????????int
            light_high_threshold_get = atoi(valuebuf);

            sprintf(topicbuf, "%.*s", event->topic_len, event->topic);
            int i;
            for(i = 99; i > 42; i--)
            {
                topicbuf[i] = topicbuf[i-1];
            }
            topicbuf[34] = 'r';
            topicbuf[35] = 'e';
            topicbuf[36] = 's';
            topicbuf[37] = 'p';
            topicbuf[38] = 'o';
            topicbuf[39] = 'n';
            topicbuf[40] = 's';
            topicbuf[41] = 'e';
            topicbuf[42] = '/';
            
            // ?????????????????????????????????
            printf("light_high_threshold_get=%d\r\n", light_high_threshold_get);
            msg_id = esp_mqtt_client_publish(client, topicbuf, "Li Xincong Superman" , 0, 1, 0);
        }
        else if(strstr(cmdbuf, "soil_humi_low_threshold")){
            cloud_control_flag = 1;
            // ???????????????????????????
            printf("CMDDATA=%s\r\n", cmdbuf);
            // ???????????????????????????????????????valuebuf???
            char valuebuf[5];
            valuebuf[0] = cmdbuf[24];
            valuebuf[1] = cmdbuf[25];
            valuebuf[2] = cmdbuf[26];
            // char?????????int
            soil_humi_low_threshold_get = atoi(valuebuf);
            
            sprintf(topicbuf, "%.*s", event->topic_len, event->topic);
            int i;
            for(i = 99; i > 42; i--)
            {
                topicbuf[i] = topicbuf[i-1];
            }
            topicbuf[34] = 'r';
            topicbuf[35] = 'e';
            topicbuf[36] = 's';
            topicbuf[37] = 'p';
            topicbuf[38] = 'o';
            topicbuf[39] = 'n';
            topicbuf[40] = 's';
            topicbuf[41] = 'e';
            topicbuf[42] = '/';
            
            // ?????????????????????????????????
            printf("soil_humi_low_threshold_get=%d\r\n", soil_humi_low_threshold_get);
            msg_id = esp_mqtt_client_publish(client, topicbuf, "Li Xincong Superman" , 0, 1, 0);
        }
        else if(strstr(cmdbuf, "soil_humi_high_threshold")){
            cloud_control_flag = 1;
            // ???????????????????????????
            printf("CMDDATA=%s\r\n", cmdbuf);
            // ???????????????????????????????????????valuebuf???
            char valuebuf[5];
            valuebuf[0] = cmdbuf[25];
            valuebuf[1] = cmdbuf[26];
            valuebuf[2] = cmdbuf[27];
            // char?????????int
            soil_humi_high_threshold_get = atoi(valuebuf);

            sprintf(topicbuf, "%.*s", event->topic_len, event->topic);
            int i;
            for(i = 99; i > 42; i--)
            {
                topicbuf[i] = topicbuf[i-1];
            }
            topicbuf[34] = 'r';
            topicbuf[35] = 'e';
            topicbuf[36] = 's';
            topicbuf[37] = 'p';
            topicbuf[38] = 'o';
            topicbuf[39] = 'n';
            topicbuf[40] = 's';
            topicbuf[41] = 'e';
            topicbuf[42] = '/';
            
            // ?????????????????????????????????
            printf("soil_humi_high_threshold_get=%d\r\n", soil_humi_high_threshold_get);
            msg_id = esp_mqtt_client_publish(client, topicbuf, "Li Xincong Superman" , 0, 1, 0);
        }
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            ESP_LOGI(TAG, "Last error code reported from esp-tls: 0x%x", event->error_handle->esp_tls_last_esp_err);
            ESP_LOGI(TAG, "Last tls stack error number: 0x%x", event->error_handle->esp_tls_stack_err);
            ESP_LOGI(TAG, "Last captured errno : %d (%s)",  event->error_handle->esp_transport_sock_errno,
                     strerror(event->error_handle->esp_transport_sock_errno));
        } else if (event->error_handle->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED) {
            ESP_LOGI(TAG, "Connection refused error: 0x%x", event->error_handle->connect_return_code);
        } else {
            ESP_LOGW(TAG, "Unknown error type: 0x%x", event->error_handle->error_type);
        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

void mqtt_app_start(void)
{
    // ?????????mqtt_client
    const esp_mqtt_client_config_t mqtt_cfg = {
        // .uri = ONENET_BROKER_URI,
        // .cert_pem = (const char *)mqtt_eclipseprojects_io_pem_start,
        .host = ONENET_HOST,
        .port = ONENET_PORT,
        // .cert_pem = (const char *)MQTTS_certificate_pem_start,
        .username = DEVICE_ID,
        .password = PASSWORD,
        .client_id = DEVICE_NAME,
        // .transport = MQTT_TRANSPORT_OVER_SSL,
        .keepalive = 1500,
        .protocol_ver = MQTT_PROTOCOL_V_3_1_1,
        // .uri = TEST_URI,
        // .cert_pem = (const char *)mqtt_eclipseprojects_io_pem_start,

    };

    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}
