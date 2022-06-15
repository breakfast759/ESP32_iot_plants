// 包含 ESP32 头文件
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
// ESP SYS Libs
#include "esp_freertos_hooks.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
// WIFI Libs
#include "nvs_flash.h"

// LVGL Libs
#include "lvgl/lvgl.h"								// LVGL头文件
#include "lvgl_helpers.h"							// LVGL硬件驱动相关头文件
#include "./lvgl/src/extra/libs/fsdrv/lv_fsdrv.h"	// LVGL文件系统驱动相关头文件

// MY Libs
#include "LED.h"
#include "pump.h"
#include "motor.h"
#include "light.h"
#include "ui_display.h"
#include "wifi_connection.h"
#include "mqtt_ssl.h"
#include "solid_humi_adc.h"
#include "illuminance_adc.h"
#include "dht11.h"

#define TAG "IoT Demo"

//LVGL 需要系统滴答声才能知道动画和其他任务的经过时间。
#define LV_TICK_PERIOD_MS				1	// LVGL 任务滴答周期 10MS
SemaphoreHandle_t xGuiSemaphore;			// 创建一个GUI信号量

bool wifi_connect_flag = 0;
bool mqtt_connect_flag = 0;

static void lv_tick_task(void *arg);		// LVGL 时钟任务
void guiTask(void *pvParameter);			// LVGL_GUI任务

// LVGL 时钟任务
// 即告诉LVGL每LV_TICK_PERIOD_MS（这里是1ms）调用1次
static void lv_tick_task(void *arg) {
	(void) arg;
	lv_tick_inc(LV_TICK_PERIOD_MS);
}

// LVGL_GUI任务
void guiTask(void *pvParameter)
{
	(void) pvParameter;
	xGuiSemaphore = xSemaphoreCreateMutex();	// 创建GUI信号量

	// 初始化缓存
	static lv_color_t buf1[DISP_BUF_SIZE];  // LVGL 用来绘制图像的缓冲区
	static lv_color_t buf2[DISP_BUF_SIZE];  // LVGL 在前一帧被渲染时将下一帧渲染到另一个缓冲区中发送
	static lv_disp_draw_buf_t disp_buf;  	// 显存定义
	uint32_t size_in_px = DISP_BUF_SIZE;  	// 320*64
	// 初始化显存(显存, buf1, buf2, `buf1` 和 `buf2` 的像素数大小)
	lv_disp_draw_buf_init(&disp_buf, buf1, buf2, size_in_px);

	// 添加并注册显示驱动
	lv_disp_drv_t disp_drv;  				// 定义一个显示驱动
	lv_disp_drv_init(&disp_drv);  			// 初始化显示驱动
	disp_drv.flush_cb = disp_driver_flush;  // 显示函数回调即，发送缓存到液晶函数
	disp_drv.draw_buf = &disp_buf;  		// 设置显存
	lv_disp_drv_register(&disp_drv);  		// 注册显示驱动

	// 添加并注册触摸驱动 
	lv_indev_drv_t indev_drv;               // 定义一个输入驱动
	lv_indev_drv_init(&indev_drv);          // 初始化输入驱动
	indev_drv.read_cb = touch_driver_read;  // 输入驱动读取回调，即触摸芯片读取坐标函数
	indev_drv.type = LV_INDEV_TYPE_POINTER; // 输入驱动为指针设备
	lv_indev_drv_register(&indev_drv);      // 注册输入驱动
	ESP_LOGI(TAG, "Add Register Touch Drv successfully!");

	ui_display();

	// 创建一个定时器处理LVGL，定期处理GUI回调
	const esp_timer_create_args_t periodic_timer_args = {
		.callback = &lv_tick_task,          // 超时回调函数
		.name = "periodic_gui"              // 定时器名称
	};
	esp_timer_handle_t periodic_timer;                                                      // 定义定时器
	ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));               // 创建定时器
	ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000));    // 开始定时器，每LV_TICK_PERIOD_MS * 1000微妙触发一次
	
	while (1) {
		vTaskDelay(1);
		// 尝试锁定信号量，如果成功，调用lvgl的东西
		// 参数 xSemaphore 正在获取的信号量的句柄 - 在创建信号量时获得。
		if (xSemaphoreTake(xGuiSemaphore, (TickType_t)10) == pdTRUE) {
			lv_timer_handler();
			xSemaphoreGive(xGuiSemaphore);	// 释放信号量
		}
	}
	// vTaskDelete(NULL);						// 删除任务
}

// 主函数
void app_main() 
{
	ESP_LOGI("\r\n\r\nIoT Demo", "APP Start!~");
	uint8_t MAC[6];
	ESP_LOGI(TAG, "Hello Breakfast759!");
	// 打印芯片信息
	esp_chip_info_t chip_info;
	esp_chip_info(&chip_info);
	ESP_LOGI(TAG, "ESP32 Chip Cores Count:  %d",chip_info.cores);
	if(chip_info.model == 1){
		ESP_LOGI(TAG, "ESP32 Chip Model is:  ESP32");
	}else if(chip_info.model == 2){
		ESP_LOGI(TAG, "ESP32 Chip Model is:  ESP32S2");
	}else{
		ESP_LOGI(TAG, "ESP32 Chip Model is:  Unknown Model");
	}
	ESP_LOGI(TAG, "ESP32 Chip Features is:  %d",chip_info.features);
	ESP_LOGI(TAG, "ESP32 Chip Revision is:  %d",chip_info.revision);

	ESP_LOGI(TAG, "ESP32 Chip, WiFi%s%s, ",
			(chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
			(chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

	ESP_LOGI(TAG, "SPI Flash Chip Size: %dMByte %s flash", spi_flash_get_chip_size() / (1024 * 1024),
			(chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "Embedded" : "External");

	ESP_LOGI(TAG, "Free Heap Size is:  %d Byte",esp_get_free_heap_size());
	ESP_LOGI(TAG, "Free Internal Heap Size is:  %d Byte",esp_get_free_internal_heap_size());
	ESP_LOGI(TAG, "Free minimum Heap Size is:  %d Byte",esp_get_minimum_free_heap_size());

	esp_base_mac_addr_get(MAC);
	ESP_LOGI(TAG, "Base MAC Addr :  %02X.%02X.%02X.%02X.%02X.%02X",MAC[0],MAC[1],MAC[2],MAC[3],MAC[4],MAC[5]);

	ESP_LOGI(TAG, "LVGL Version: %d.%d.%d \r\n",LVGL_VERSION_MAJOR,LVGL_VERSION_MINOR,LVGL_VERSION_PATCH);

	//Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

	// wifi_init_sta();
	lv_init();									// 初始化LittlevGL
	lvgl_driver_init();							// 初始化液晶SPI驱动 触摸芯片SPI/IIC驱动
	led_config();								// 初始化LED组件，方便调试
	pump_config();								// 初始化pump组件
	motor_config();								// 初始化motor组件
	light_config();								// 初始化light组件
	solidHumi_config();							// 初始化土壤湿度测量组件（主要是为了初始化adc）
	illuminance_config();						// 初始化光照度测量组件
	dht11_init(DHT11_GPIO);						// 初始化温湿度传感器dht11
	wifi_init_sta(wifi_connect_flag, EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);//依照默认设置连一次wifi
	wifi_connect_flag |= 1;
	esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
    esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_BASE", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);
    esp_event_loop_create_default();
    mqtt_app_start();
	mqtt_connect_flag |= 1; 
	
	// 如果要使用任务创建图形，则需要创建固定任务,否则可能会出现诸如内存损坏等问题
	xTaskCreatePinnedToCore(guiTask, "gui", 4096*2, NULL, 0, NULL, 1);
	
}




