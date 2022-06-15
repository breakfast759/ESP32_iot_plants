/*
 * @Author: Breakfast759
 * @Date: 2022-05-19 15:50:20
 * @LastEditors: Breakfast759
 * @LastEditTime: 2022-06-06 13:41:31
 * @FilePath: \11_Version1.0.1.20220531_RC\components\ui_display\ui_display.c
 * @Description: 
 * 
 * Copyright (c) 2022 by 514 Anti-Rolling Department, All Rights Reserved. 
 */

/*********************
 *      INCLUDES
 *********************/
#include "ui_display.h"
#include "LED.h"
#include "pump.h"
#include "motor.h"
#include "light.h"
#include "wifi_connection.h"
#include "solid_humi_adc.h"
#include "illuminance_adc.h"
#include "dht11.h"
#include "esp_log.h"
#include "mqtt_ssl.h"

#define TAG "ui_display"

#if LV_MEM_CUSTOM == 0 && LV_MEM_SIZE < (38ul * 1024ul)
    #error Insufficient memory for lv_demo_widgets. Please set LV_MEM_SIZE to at least 38KB (38ul * 1024ul).  48KB is recommended. 
#endif

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef enum {
    DISP_SMALL,
    DISP_MEDIUM,
    DISP_LARGE,
}disp_size_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void info_create(lv_obj_t * parent);
static void measuring_create(lv_obj_t * parent);    /**/
static void control_create(lv_obj_t * parent);
static void color_changer_create(lv_obj_t * parent);

// 声明回调函数
static void color_changer_event_cb(lv_event_t * e);
static void color_event_cb(lv_event_t * e);
static void ta_event_cb(lv_event_t * e);
static void connect_btn_cb(lv_event_t * e);
static void chart_event_cb(lv_event_t * e);
static void motor_sw_cb(lv_event_t * e);
static void light_sw_cb(lv_event_t * e);
static void pump_sw_cb(lv_event_t * e);
static void temp_arc_cb(lv_event_t * e);
static void light_arc_cb(lv_event_t *e);
static void humi_slider_cb(lv_event_t *e);

/**********************
 *  STATIC VARIABLES
 **********************/
static disp_size_t disp_size;

static lv_obj_t * tv;
static lv_style_t style_text_muted;
static lv_style_t style_title;
static lv_style_t style_icon;
static lv_style_t style_bullet;
static lv_style_t style_control;

static lv_obj_t * user_name;
static lv_obj_t * password;

static lv_obj_t * chart1;
static lv_obj_t * chart2;
static lv_obj_t * chart3;
static lv_obj_t * chart4;

static lv_chart_series_t * ser1;
static lv_chart_series_t * ser2;
static lv_chart_series_t * ser3;

static lv_obj_t * arc1;
static lv_obj_t * arc2;
static lv_obj_t * slider1;
static lv_obj_t * sw1;
static lv_obj_t * sw2;
static lv_obj_t * sw3;

static const lv_font_t * font_large;
static const lv_font_t * font_normal;

// 定时器运行次数，通过这个来计时，定时器周期2s，半小时（也就是30 * 60 * 60 = 108000）上传一次数据
static uint32_t timer_count;
// about wifi
static const char * wifi_ssid;
static const char * wifi_password;
extern bool wifi_connect_flag;
extern bool mqtt_connect_flag;
// about sensor
static int temp_low_threshold = 50;
extern int temp_low_threshold_get;

static int light_high_threshold = 50;
extern int light_high_threshold_get;

static int soil_humi_low_threshold = 0;
extern int soil_humi_low_threshold_get;

static int soil_humi_high_threshold = 50;
extern int soil_humi_high_threshold_get;

static int illuminance = 0;
static int humi = 0;
static int last_humi = 0;
static int temp = 0;
static int soil_humi = 0;
// about device
extern bool motor_status;
extern bool motor_mannual_flag;
extern bool light_status;
extern bool light_mannual_flag;
extern bool pump_status;
extern bool pump_mannual_flag;

extern bool cloud_control_flag;

/**********************
 *       MACROS       *
 **********************/

/**********************
 *  GLOBAL FUNCTIONS  *
 **********************/
void measuring_timer()
{
    int msg_id;
    char jsonBuf[400];
    char databuf[350];

    timer_count++;
    // 判断是否有来自云平台的命令？
    if(cloud_control_flag)
    {
        // temp_low_threshold_get = temp_low_threshold_get / 5;
        lv_arc_set_value(arc1, temp_low_threshold_get);
        lv_arc_set_value(arc2, light_high_threshold_get);
        lv_slider_set_left_value(slider1, soil_humi_low_threshold_get, LV_ANIM_OFF);
        lv_slider_set_value(slider1, soil_humi_high_threshold_get, LV_ANIM_OFF);
        if(light_status == 1)
        {
            lv_obj_add_state(sw2, LV_STATE_CHECKED);
        }
        else if(light_status == 0)
        {
            lv_obj_clear_state(sw2, LV_STATE_CHECKED);
        }
        if(motor_status == 1)
        {
            lv_obj_add_state(sw1, LV_STATE_CHECKED);
        }
        else if(motor_status == 0)
        {
            lv_obj_clear_state(sw1, LV_STATE_CHECKED);
        }
        if(pump_status == 1)
        {
            lv_obj_add_state(sw3, LV_STATE_CHECKED);
        }
        else if(pump_status == 0)
        {
            lv_obj_clear_state(sw3, LV_STATE_CHECKED);
        }
        cloud_control_flag = 0;
    }
    // 根据云平台命令输出控制继电器的PWM信号
    motor_set_pwm((1-motor_status) * 100);
    pump_set_pwm((1-pump_status) * 100);
    light_set_pwm((1-light_status) * 100);

    ESP_LOGI(TAG, "temp_low_threshold_get->%d", temp_low_threshold_get);
    // uint16_t * count = timer -> user_data;
    /* Get illuminance data */
    illuminance = illuminance_read();
    light_high_threshold = lv_arc_get_value(arc2); 
    illuminance = 100 - (illuminance - 100) / 15;
    ESP_LOGI(TAG, "[%lld] illuminance->%d", esp_timer_get_time(), illuminance);
    // 判断是否有人为操作补光灯的行为？（优先执行人为的命令）
    if(light_mannual_flag == 0) 
    {
        if(light_high_threshold < illuminance)
        {
            light_status = 0;
            lv_obj_clear_state(sw2, LV_STATE_CHECKED);
            led_set_lux(0);
            light_set_pwm(100);
        }
        else
        {
            light_status = 1;
            lv_obj_add_state(sw2, LV_STATE_CHECKED);
            led_set_lux(100);
            light_set_pwm(0);
        }
    }
    /* Get temp and environment humi data */
    dht11_start_get(&temp, &humi);
    // last_humi = humi;                               // 存储上一个湿度数据，滤除干扰项
    temp_low_threshold = lv_arc_get_value(arc1); 
    ESP_LOGI(TAG, "[%lld] temp->%i.%i C     hum->%i.%i%%", esp_timer_get_time(), temp / 10, temp % 10, humi / 10, humi % 10);
    // 判断是否有人为操作风扇的行为？（优先执行人为的命令）
    if(motor_mannual_flag == 0)
    {
        if((temp_low_threshold * 5) < temp)
        {
            motor_status = 1;
            lv_obj_add_state(sw1, LV_STATE_CHECKED);
            // do something with the motor
            motor_set_pwm(0);
        }
        else
        {
            motor_status = 0;
            lv_obj_clear_state(sw1, LV_STATE_CHECKED);
            // do something with the motor
            motor_set_pwm(100);
        }
    }
    /* Get soil humi data */
    soil_humi = solidHumi_read();
    soil_humi_low_threshold = lv_slider_get_left_value(slider1); 
    soil_humi_high_threshold = lv_slider_get_value(slider1);
    // 值越大越湿，值越小越干
    soil_humi = 100 - (soil_humi - 800) / 13;  // 这个式子还得调整
    if(soil_humi < 0)
        soil_humi = 0;
    else if(soil_humi > 100)
        soil_humi = 100;
    ESP_LOGI(TAG, "[%lld] soil_humi->%d", esp_timer_get_time(), soil_humi);
    // 判断是否有人为操作水泵的行为？（优先执行人为的命令）
    if(pump_mannual_flag == 0)
    {
        if((soil_humi_low_threshold < soil_humi) && (soil_humi_high_threshold > soil_humi))
        {
            pump_status = 1;
            lv_obj_add_state(sw3, LV_STATE_CHECKED);
            // do something with the pump
            pump_set_pwm(0);
        }
        else
        {
            pump_status = 0;
            lv_obj_clear_state(sw3, LV_STATE_CHECKED);
            // do something with the pump
            pump_set_pwm(100);
        }
    }
    temp = temp;
    humi = humi / 10;

    lv_chart_set_next_value(chart1, ser1, temp);
    lv_chart_set_next_value(chart2, ser2, humi);
    // 判断是否到长定时周期？
    if(timer_count >= 10)
    {
        lv_chart_set_next_value(chart3, ser3, soil_humi);
        timer_count = 0;
    }

    sprintf(databuf, "{ \"Temp\":[{\"v\":%.1f}] ,\
    \"Humi\":[{\"v\":%.1f}], \
    \"Soil_humi\":[{\"v\":%d}], \
    \"Illuminance\":[{\"v\":%d}], \
    \"Temp_low_threshold\":[{\"v\":%d}], \
    \"Light_high_threshold\":[{\"v\":%d}], \
    \"Soil_humi_low_threshold\":[{\"v\":%d}], \
    \"Soil_humi_high_threshold\":[{\"v\":%d}], \
    \"Motor_status\":[{\"v\":%d}], \
    \"Light_status\":[{\"v\":%d}], \
    \"Pump_status\":[{\"v\":%d}] }", \
    temp/10.0, humi * 1.0, soil_humi, illuminance, temp_low_threshold, light_high_threshold, soil_humi_low_threshold, soil_humi_high_threshold, \
    motor_status, light_status, pump_status); 
    sprintf(jsonBuf, ONENET_POST_BODY_FORMAT, databuf);
    msg_id = esp_mqtt_client_publish(client, ONENET_TOPIC_DP_PUBLISH, jsonBuf , 0, 1, 0);
    ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
}

void ui_display(void)
{
    // 声明显示屏的大/中/小
    if(LV_HOR_RES <= 320) disp_size = DISP_SMALL;
    else if(LV_HOR_RES < 720) disp_size = DISP_MEDIUM;
    else disp_size = DISP_LARGE;

    // 将font_large和font_normal都定义成LV_FONT_DEFAULT，方便后续改字体？
    font_large = LV_FONT_DEFAULT;
    font_normal = LV_FONT_DEFAULT;

    // 定义不同显示屏大小下Tab的宽度和使用的两种字体（font_large和font_normal）的大小
    lv_coord_t tab_h;  // 定义一个Tabview的高度，即上方选项卡的高度
    if(disp_size == DISP_LARGE) {
        tab_h = 70;
#if LV_FONT_MONTSERRAT_24  // 如果声明了24号字体
        font_large     = &lv_font_montserrat_24;
#else
        LV_LOG_WARN("LV_FONT_MONTSERRAT_24 is not enabled for the widgets demo. Using LV_FONT_DEFAULT instead.");
#endif
#if LV_FONT_MONTSERRAT_16  // 如果声明了16号字体
        font_normal    = &lv_font_montserrat_16;
#else
        LV_LOG_WARN("LV_FONT_MONTSERRAT_16 is not enabled for the widgets demo. Using LV_FONT_DEFAULT instead.");
#endif
    } else if(disp_size == DISP_MEDIUM) {
        tab_h = 45;
#if LV_FONT_MONTSERRAT_20
        font_large     = &lv_font_montserrat_20;
#else
        LV_LOG_WARN("LV_FONT_MONTSERRAT_20 is not enabled for the widgets demo. Using LV_FONT_DEFAULT instead.");
#endif
#if LV_FONT_MONTSERRAT_14
        font_normal    = &lv_font_montserrat_14;
#else
        LV_LOG_WARN("LV_FONT_MONTSERRAT_14 is not enabled for the widgets demo. Using LV_FONT_DEFAULT instead.");
#endif
    } else { /* disp_size == DISP_SMALL */
        tab_h = 45;
#if LV_FONT_MONTSERRAT_18
        font_large     = &lv_font_montserrat_18;
#else
    LV_LOG_WARN("LV_FONT_MONTSERRAT_18 is not enabled for the widgets demo. Using LV_FONT_DEFAULT instead.");
#endif
#if LV_FONT_MONTSERRAT_12
        font_normal    = &lv_font_montserrat_12;
#else
    LV_LOG_WARN("LV_FONT_MONTSERRAT_12 is not enabled for the widgets demo. Using LV_FONT_DEFAULT instead.");
#endif
    }

// 接下来定义样式风格
#if LV_USE_THEME_DEFAULT
    lv_theme_default_init(NULL, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), LV_THEME_DEFAULT_DARK, font_normal);
#endif

    // 设计不可选文字的风格（不透明度减半）
    lv_style_init(&style_text_muted);
    lv_style_set_text_opa(&style_text_muted, LV_OPA_50);

    // 设计标题文字的风格（使用大号字体）
    lv_style_init(&style_title);
    lv_style_set_text_font(&style_title, font_large);

    // 设计符号的风格
    lv_style_init(&style_icon);
    lv_style_set_text_color(&style_icon, lv_theme_get_color_primary(NULL));
    lv_style_set_text_font(&style_icon, font_large);

    // 设计bullet文字的风格
    lv_style_init(&style_bullet);
    lv_style_set_border_width(&style_bullet, 0);
    lv_style_set_radius(&style_bullet, LV_RADIUS_CIRCLE);

    // 设计控制界面的风格
    lv_style_init(&style_control);
    lv_style_set_text_font(&style_control, &br759_font_lcd_segment_30);

    // 创建一个选项卡，并为其配置好样式
    tv = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, tab_h);

    lv_obj_set_style_text_font(lv_scr_act(), font_normal, 0);

    // 增加选项卡中的三个选项
    // 我习惯于说“三个界面”
    lv_obj_t * t1 = lv_tabview_add_tab(tv, "Info");
    lv_obj_t * t2 = lv_tabview_add_tab(tv, "Measuring");
    lv_obj_t * t3 = lv_tabview_add_tab(tv, "Control");
    // 调用三个函数来创建每个选项的界面
    info_create(t1);
    measuring_create(t2);
    control_create(t3);
    // lv_timer_t * timer = lv_timer_create(measuring_timer, 2000, &led_lux);
    lv_timer_create(measuring_timer, 2000, NULL);

    // 一个例程提供的改变选项卡颜色的函数
    color_changer_create(tv);
}

/**********************
 *  STATIC FUNCTIONS  *
 **********************/

// 第一个选项：profile界面
static void info_create(lv_obj_t * parent)
{
    lv_obj_t * panel1 = lv_obj_create(parent);
    lv_obj_set_height(panel1, LV_SIZE_CONTENT);

    LV_IMG_DECLARE(entrepreneurLiu);
    // LV_IMG_DECLARE(MingmingMeichan);
    // LV_IMG_DECLARE(Meichan3);
    lv_obj_t * avatar = lv_img_create(panel1);
    lv_img_set_src(avatar, &entrepreneurLiu);
    // lv_img_set_src(avatar, &MingmingMeichan);
    // lv_img_set_src(avatar, &Meichan3);

    lv_obj_t * name = lv_label_create(panel1);
    lv_label_set_text(name, "514 Anti-Rolling Dep.");
    lv_obj_add_style(name, &style_title, 0);

    lv_obj_t * dsc = lv_label_create(panel1);
    lv_obj_add_style(dsc, &style_text_muted, 0);
    lv_label_set_text(dsc, "I'm the most handsome man in 514!" );
    lv_label_set_long_mode(dsc, LV_LABEL_LONG_WRAP);

    lv_obj_t * email_icn = lv_label_create(panel1);
    lv_obj_add_style(email_icn, &style_icon, 0);
    lv_label_set_text(email_icn, LV_SYMBOL_ENVELOPE);

    lv_obj_t * email_label = lv_label_create(panel1);
    lv_label_set_text(email_label, "breakfast759@163.com");

    lv_obj_t * call_icn = lv_label_create(panel1);
    lv_obj_add_style(call_icn, &style_icon, 0);
    lv_label_set_text(call_icn, LV_SYMBOL_CALL);

    lv_obj_t * call_label = lv_label_create(panel1);
    lv_label_set_text(call_label, "+86 151 679 48050");

    lv_obj_t * log_out_btn = lv_btn_create(panel1);
    lv_obj_set_height(log_out_btn, LV_SIZE_CONTENT);

    lv_obj_t * label = lv_label_create(log_out_btn);
    lv_label_set_text(label, "Chat with me!");
    lv_obj_center(label);

    lv_obj_t * invite_btn = lv_btn_create(panel1);
    lv_obj_add_state(invite_btn, LV_STATE_DISABLED);
    lv_obj_set_height(invite_btn, LV_SIZE_CONTENT);

    label = lv_label_create(invite_btn);
    lv_label_set_text(label, "Invite");
    lv_obj_center(label);

    /*Create a keyboard*/
    lv_obj_t * kb = lv_keyboard_create(lv_scr_act());
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);

    /*Create the second panel*/
    lv_obj_t * panel2 = lv_obj_create(parent);
    lv_obj_set_height(panel2, LV_SIZE_CONTENT);

    lv_obj_t * panel2_title = lv_label_create(panel2);
    lv_label_set_text(panel2_title, "Wifi Info");
    lv_obj_add_style(panel2_title, &style_title, 0);

    /* TODO
     * 改为dropdown对象，用于选择Wifi
     * 扫描后将扫描到的Wifi作为选项显示出来   */
    lv_obj_t * user_name_label = lv_label_create(panel2);
    lv_label_set_text(user_name_label, "Connect to");
    lv_obj_add_style(user_name_label, &style_text_muted, 0);

    user_name = lv_textarea_create(panel2);
    lv_textarea_set_one_line(user_name, true);
    lv_textarea_set_placeholder_text(user_name, "Wifi name");
    lv_obj_add_event_cb(user_name, ta_event_cb, LV_EVENT_ALL, kb);

    lv_obj_t * password_label = lv_label_create(panel2);
    lv_label_set_text(password_label, "Password");
    lv_obj_add_style(password_label, &style_text_muted, 0);

    password = lv_textarea_create(panel2);
    lv_textarea_set_one_line(password, true);
    lv_textarea_set_password_mode(password, true);
    lv_textarea_set_placeholder_text(password, "Wifi password");
    lv_obj_add_event_cb(password, ta_event_cb, LV_EVENT_ALL, kb);

    lv_obj_t * connect_btn = lv_btn_create(panel2);
    lv_obj_set_height(connect_btn, LV_SIZE_CONTENT);
    lv_obj_add_event_cb(connect_btn, connect_btn_cb, LV_EVENT_ALL, NULL);

    label = lv_label_create(connect_btn);
    lv_label_set_text(label, "Connect!");
    lv_obj_center(label);

    // SMALL显示屏的布局
    /* 首先对parent对象（这里就是profile界面）进行布局
     * parent对象包含三个子对象    --panel1
                                --panel2
                                --panel3 
        这里先划分控件                               
    */
    // LV_GRID_FR(1)意味着将所有的剩余空间都分配给第1列，LV_GRID_FR(x)中的x决定了剩余空间的分配比例
    static lv_coord_t grid_main_col_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t grid_main_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
    lv_obj_set_grid_dsc_array(parent, grid_main_col_dsc, grid_main_row_dsc);


    /*Create the top panel*/
    /* 对panel1进行布局
     * panel1包含如下对象     --avatar          Avatar
                            --name            Name
                            --dsc             Description
                            --email_icn       Email
                            --email_label
                            --call_icn        Phone number 
                            --call_label 
                            --log_out_btn     Button1
                            --invite_btn      Button2
    */
    static lv_coord_t grid_1_col_dsc[] = {LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t grid_1_row_dsc[] = {LV_GRID_CONTENT,      /*Avatar*/
                                        LV_GRID_CONTENT,        /*Name*/
                                        LV_GRID_CONTENT,        /*Description*/
                                        LV_GRID_CONTENT,        /*Email*/
                                        LV_GRID_CONTENT,        /*Phone number*/
                                        LV_GRID_CONTENT,        /*Button1*/
                                        LV_GRID_CONTENT,        /*Button2*/
                                        LV_GRID_TEMPLATE_LAST};

    lv_obj_set_grid_dsc_array(panel1, grid_1_col_dsc, grid_1_row_dsc);

    static lv_coord_t grid_2_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t grid_2_row_dsc[] = {
            LV_GRID_CONTENT,            /*Title*/
            5,                          /*Separator*/
            LV_GRID_CONTENT,            /*Wifi name title*/
            40,                         /*Wifi name box*/
            LV_GRID_CONTENT,            /*Wifi password title*/
            40,                         /*Wifi password box*/
            LV_GRID_CONTENT,            /*Connect btn*/
            LV_GRID_TEMPLATE_LAST   /*Box*/
    };

    lv_obj_set_grid_dsc_array(panel2, grid_2_col_dsc, grid_2_row_dsc);

    lv_obj_set_grid_cell(panel1, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    lv_obj_set_style_text_align(dsc, LV_TEXT_ALIGN_CENTER, 0);

    lv_obj_set_grid_cell(avatar, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_CENTER, 0, 1);
    lv_obj_set_grid_cell(name, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_CENTER, 1, 1);
    lv_obj_set_grid_cell(dsc, LV_GRID_ALIGN_STRETCH, 0, 2, LV_GRID_ALIGN_START, 2, 1);
    lv_obj_set_grid_cell(email_icn, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 3, 1);
    lv_obj_set_grid_cell(email_label, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_CENTER, 3, 1);
    lv_obj_set_grid_cell(call_icn, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 4, 1);
    lv_obj_set_grid_cell(call_label, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_CENTER, 4, 1);
    lv_obj_set_grid_cell(log_out_btn, LV_GRID_ALIGN_STRETCH, 0, 2, LV_GRID_ALIGN_CENTER, 5, 1);
    lv_obj_set_grid_cell(invite_btn, LV_GRID_ALIGN_STRETCH, 0, 2, LV_GRID_ALIGN_CENTER, 6, 1);

    lv_obj_set_grid_cell(panel2, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_START, 1, 1);
    lv_obj_set_grid_cell(panel2_title, LV_GRID_ALIGN_START, 0, 2, LV_GRID_ALIGN_CENTER, 0, 1);
    lv_obj_set_grid_cell(user_name_label, LV_GRID_ALIGN_START, 0, 2, LV_GRID_ALIGN_START, 2, 1);
    lv_obj_set_grid_cell(user_name, LV_GRID_ALIGN_STRETCH, 0, 2, LV_GRID_ALIGN_START, 3, 1);
    lv_obj_set_grid_cell(password_label, LV_GRID_ALIGN_START, 0, 2, LV_GRID_ALIGN_START, 4, 1);
    lv_obj_set_grid_cell(password, LV_GRID_ALIGN_STRETCH, 0, 2, LV_GRID_ALIGN_START, 5, 1);
    lv_obj_set_grid_cell(connect_btn, LV_GRID_ALIGN_STRETCH, 0, 2, LV_GRID_ALIGN_CENTER, 6, 1);

}
// 第二个选项：measuring界面
static void measuring_create(lv_obj_t * parent)
{
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_ROW_WRAP);

    static lv_coord_t grid_chart_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_FR(1), 10, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t grid_chart_col_dsc[] = {20, LV_GRID_FR(1), 20, LV_GRID_TEMPLATE_LAST};

    /*******************************
     * 第1张，折线图，用于显示环境温度
     *******************************/
    lv_obj_t * chart1_cont = lv_obj_create(parent);
    lv_obj_set_flex_grow(chart1_cont, 1);
    lv_obj_set_grid_dsc_array(chart1_cont, grid_chart_col_dsc, grid_chart_row_dsc);

    lv_obj_set_height(chart1_cont, LV_PCT(100));
    lv_obj_set_style_max_height(chart1_cont, 300, 0);

    lv_obj_t * title = lv_label_create(chart1_cont);
    lv_label_set_text(title, "Temp");
    lv_obj_add_style(title, &style_title, 0);
    lv_obj_set_grid_cell(title, LV_GRID_ALIGN_START, 0, 3, LV_GRID_ALIGN_START, 0, 1);

    chart1 = lv_chart_create(chart1_cont);
    lv_group_add_obj(lv_group_get_default(), chart1);
    lv_obj_add_flag(chart1, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_grid_cell(chart1, LV_GRID_ALIGN_STRETCH, 1, 2, LV_GRID_ALIGN_STRETCH, 1, 1);
    lv_chart_set_range(chart1, LV_CHART_AXIS_PRIMARY_Y, 0, 500);
    lv_chart_set_axis_tick(chart1, LV_CHART_AXIS_PRIMARY_Y, 0, 0, 5, 1, true, 80);
    lv_chart_set_axis_tick(chart1, LV_CHART_AXIS_PRIMARY_X, 0, 0, 12, 1, true, 50);
    lv_chart_set_div_line_count(chart1, 0, 12);
    lv_chart_set_point_count(chart1, 12);
    lv_obj_add_event_cb(chart1, chart_event_cb, LV_EVENT_ALL, NULL);
    if(disp_size == DISP_SMALL) lv_chart_set_zoom_x(chart1, 256 * 3);
    else if(disp_size == DISP_MEDIUM) lv_chart_set_zoom_x(chart1, 256 * 2);

    lv_obj_set_style_border_side(chart1, LV_BORDER_SIDE_LEFT | LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_radius(chart1, 0, 0);

    ser1 = lv_chart_add_series(chart1, lv_theme_get_color_primary(chart1), LV_CHART_AXIS_PRIMARY_Y);
    lv_chart_set_next_value(chart1, ser1, 0);
    lv_chart_set_next_value(chart1, ser1, 0);
    lv_chart_set_next_value(chart1, ser1, 0);
    lv_chart_set_next_value(chart1, ser1, 0);
    lv_chart_set_next_value(chart1, ser1, 0);
    lv_chart_set_next_value(chart1, ser1, 0);
    lv_chart_set_next_value(chart1, ser1, 0);
    lv_chart_set_next_value(chart1, ser1, 0);
    lv_chart_set_next_value(chart1, ser1, 0);
    lv_chart_set_next_value(chart1, ser1, 0);
    lv_chart_set_next_value(chart1, ser1, 0);
    lv_chart_set_next_value(chart1, ser1, 0);
    lv_chart_set_next_value(chart1, ser1, 0);
    // lv_chart_set_next_value(chart1, ser1, lv_rand(10, 80));

    /*******************************
     * 第2张，折线图，用于显示环境湿度
     *******************************/
    lv_obj_t * chart2_cont = lv_obj_create(parent);
    lv_obj_set_flex_grow(chart2_cont, 1);
    lv_obj_add_flag(chart2_cont, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);

    lv_obj_set_height(chart2_cont, LV_PCT(100));
    lv_obj_set_style_max_height(chart2_cont, 300, 0);

    lv_obj_set_grid_dsc_array(chart2_cont, grid_chart_col_dsc, grid_chart_row_dsc);

    title = lv_label_create(chart2_cont);
    lv_label_set_text(title, "Humi");
    lv_obj_add_style(title, &style_title, 0);
    lv_obj_set_grid_cell(title, LV_GRID_ALIGN_START, 0, 3, LV_GRID_ALIGN_START, 0, 1);

    chart2 = lv_chart_create(chart2_cont);
    lv_group_add_obj(lv_group_get_default(), chart2);
    lv_obj_add_flag(chart2, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_grid_cell(chart2, LV_GRID_ALIGN_STRETCH, 1, 2, LV_GRID_ALIGN_STRETCH, 1, 1);
    lv_chart_set_axis_tick(chart2, LV_CHART_AXIS_PRIMARY_Y, 0, 0, 5, 1, true, 80);
    lv_chart_set_axis_tick(chart2, LV_CHART_AXIS_PRIMARY_X, 0, 0, 12, 1, true, 50);
    lv_chart_set_div_line_count(chart2, 0, 12);
    lv_chart_set_point_count(chart2, 12);
    lv_obj_add_event_cb(chart2, chart_event_cb, LV_EVENT_ALL, NULL);
    lv_chart_set_zoom_x(chart2, 256 * 3);

    lv_obj_set_style_border_side(chart2, LV_BORDER_SIDE_LEFT | LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_radius(chart2, 0, 0);

    ser2 = lv_chart_add_series(chart2, lv_theme_get_color_primary(chart1), LV_CHART_AXIS_PRIMARY_Y);
    lv_chart_set_next_value(chart2, ser2, lv_rand(10, 80));
    lv_chart_set_next_value(chart2, ser2, lv_rand(10, 80));
    lv_chart_set_next_value(chart2, ser2, lv_rand(10, 80));
    lv_chart_set_next_value(chart2, ser2, lv_rand(10, 80));
    lv_chart_set_next_value(chart2, ser2, lv_rand(10, 80));
    lv_chart_set_next_value(chart2, ser2, lv_rand(10, 80));
    lv_chart_set_next_value(chart2, ser2, lv_rand(10, 80));
    lv_chart_set_next_value(chart2, ser2, lv_rand(10, 80));
    lv_chart_set_next_value(chart2, ser2, lv_rand(10, 80));
    lv_chart_set_next_value(chart2, ser2, lv_rand(10, 80));
    lv_chart_set_next_value(chart2, ser2, lv_rand(10, 80));
    lv_chart_set_next_value(chart2, ser2, lv_rand(10, 80));
    lv_chart_set_next_value(chart2, ser2, lv_rand(10, 80));

    /*******************************
     * 第3张，柱状图，用于显示土壤湿度
     *******************************/
    lv_obj_t * chart3_cont = lv_obj_create(parent);
    lv_obj_add_flag(chart3_cont, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
    lv_obj_set_flex_grow(chart3_cont, 1);

    lv_obj_set_height(chart3_cont, LV_PCT(100));
    lv_obj_set_style_max_height(chart3_cont, 300, 0);

    lv_obj_set_grid_dsc_array(chart3_cont, grid_chart_col_dsc, grid_chart_row_dsc);

    title = lv_label_create(chart3_cont);
    lv_label_set_text(title, "Soil Humi");
    lv_obj_add_style(title, &style_title, 0);
    lv_obj_set_grid_cell(title, LV_GRID_ALIGN_START, 0, 3, LV_GRID_ALIGN_START, 0, 1);

    chart3 = lv_chart_create(chart3_cont);
    lv_group_add_obj(lv_group_get_default(), chart3);
    lv_obj_add_flag(chart3, LV_OBJ_FLAG_SCROLL_ON_FOCUS);

    lv_obj_set_grid_cell(chart3, LV_GRID_ALIGN_STRETCH, 1, 2, LV_GRID_ALIGN_STRETCH, 1, 1);
    lv_chart_set_axis_tick(chart3, LV_CHART_AXIS_PRIMARY_Y, 0, 0, 5, 1, true, 80);
    lv_chart_set_axis_tick(chart3, LV_CHART_AXIS_PRIMARY_X, 0, 0, 12, 1, true, 50);
    lv_obj_set_size(chart3, LV_PCT(100), LV_PCT(100));
    lv_chart_set_type(chart3, LV_CHART_TYPE_BAR);
    lv_chart_set_div_line_count(chart3, 6, 0);
    lv_chart_set_point_count(chart3, 12);
    lv_obj_add_event_cb(chart3, chart_event_cb, LV_EVENT_ALL, NULL);
    lv_chart_set_zoom_x(chart3, 256 * 2);
    lv_obj_set_style_border_side(chart3, LV_BORDER_SIDE_LEFT | LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_radius(chart3, 0, 0);

    lv_obj_set_style_pad_gap(chart3, 0, LV_PART_ITEMS);
    lv_obj_set_style_pad_gap(chart3, 2, LV_PART_MAIN);

    ser3 = lv_chart_add_series(chart3, lv_theme_get_color_primary(chart1), LV_CHART_AXIS_PRIMARY_Y);
    lv_chart_set_next_value(chart3, ser3, 0);
    lv_chart_set_next_value(chart3, ser3, 0);
    lv_chart_set_next_value(chart3, ser3, 0);
    lv_chart_set_next_value(chart3, ser3, 0);
    lv_chart_set_next_value(chart3, ser3, 0);
    lv_chart_set_next_value(chart3, ser3, 0);
    lv_chart_set_next_value(chart3, ser3, 0);
    lv_chart_set_next_value(chart3, ser3, 0);
    lv_chart_set_next_value(chart3, ser3, 0);
    lv_chart_set_next_value(chart3, ser3, 0);
    lv_chart_set_next_value(chart3, ser3, 0);
    lv_chart_set_next_value(chart3, ser3, 0);
    lv_chart_set_next_value(chart3, ser3, 0);
    // lv_chart_set_next_value(chart3, ser3, lv_rand(10, 80));

    /*******************************
     * 第4张，bar控件，用于显示槽内水位
     *******************************/
    lv_obj_t * chart4_cont = lv_obj_create(parent);
    lv_obj_add_flag(chart4_cont, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
    lv_obj_set_flex_grow(chart4_cont, 1);

    lv_obj_set_height(chart4_cont, LV_PCT(50));
    lv_obj_set_style_max_height(chart4_cont, 300, 0);

    lv_obj_set_grid_dsc_array(chart4_cont, grid_chart_col_dsc, grid_chart_row_dsc);

    title = lv_label_create(chart4_cont);
    lv_label_set_text(title, "Water Level");
    lv_obj_add_style(title, &style_title, 0);
    lv_obj_set_grid_cell(title, LV_GRID_ALIGN_START, 0, 3, LV_GRID_ALIGN_START, 0, 1);

    chart4 = lv_bar_create(chart4_cont);
    lv_obj_set_size(chart4, 200, 20);
    lv_bar_set_value(chart4, 80, LV_ANIM_OFF);
    lv_obj_align(chart4, LV_ALIGN_CENTER, 0, 30);
    lv_obj_set_grid_cell(chart4, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_START, 1, 1);
}
// 第三个选项：control界面
static void control_create(lv_obj_t * parent)
{
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_ROW_WRAP);

    // 事实上这里还不是弹性修改布局，之后改，以适应不同的显示设备
    static lv_coord_t grid_chart_row_dsc[] = {70, LV_GRID_CONTENT, LV_GRID_CONTENT, 20, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t grid_chart_col_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    
    /*******************************
     * 第1控件，用于控制电机，进而控制温度
     *******************************/
    lv_obj_t * panel1 = lv_obj_create(parent);
    lv_obj_set_flex_grow(panel1, 1);
    lv_obj_set_height(panel1, LV_PCT(140));

    lv_obj_set_grid_dsc_array(panel1, grid_chart_col_dsc, grid_chart_row_dsc);

    arc1 = lv_arc_create(panel1);
    lv_arc_set_value(arc1, 50);
    lv_obj_set_size(arc1, 130, 130);
    lv_arc_set_bg_angles(arc1, 135, 45);
    lv_arc_set_mode(arc1, LV_ARC_MODE_REVERSE);
    lv_obj_set_grid_cell(arc1, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 0, 2);
    lv_obj_add_event_cb(arc1, temp_arc_cb, LV_EVENT_ALL, 0);

    lv_obj_t * title = lv_label_create(panel1);
    lv_label_set_text(title, "Motor");
    lv_obj_add_style(title, &style_control, 0);
    lv_obj_set_grid_cell(title, LV_GRID_ALIGN_CENTER, 1, 2, LV_GRID_ALIGN_CENTER, 0, 1);

    sw1 = lv_switch_create(panel1);
    lv_obj_set_grid_cell(sw1, LV_GRID_ALIGN_CENTER, 1, 2, LV_GRID_ALIGN_CENTER, 1, 1);
    lv_obj_add_event_cb(sw1, motor_sw_cb, LV_EVENT_ALL, 0);

    lv_obj_t * label1 = lv_label_create(arc1);
    lv_obj_align(label1, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(label1, "50");
    lv_obj_add_style(label1, &style_control, LV_STATE_DEFAULT);

    label1 = lv_label_create(panel1);
    lv_label_set_text(label1, "Temp Threshold");
    lv_obj_add_style(label1, &style_control, LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(label1, LV_GRID_ALIGN_CENTER, 0, 3, LV_GRID_ALIGN_START, 2, 1);

    lv_obj_t * dsc = lv_label_create(panel1);
    lv_obj_add_style(dsc, &style_text_muted, 0);
    lv_label_set_text(dsc, "Set the high threshold at which the motor\rneeds to be turned on by dragging the \rknob of arc above!" );
    lv_label_set_long_mode(dsc, LV_LABEL_LONG_WRAP);
    lv_obj_set_grid_cell(dsc, LV_GRID_ALIGN_START, 0, 2, LV_GRID_ALIGN_STRETCH, 3, 2);

    /***********************************
     * 第2控件，用于控制补光灯，进而控制光照度
     ***********************************/
    lv_obj_t * panel2 = lv_obj_create(parent);
    lv_obj_add_flag(panel2, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
    lv_obj_set_flex_grow(panel2, 1);
    lv_obj_set_height(panel2, LV_PCT(140));

    lv_obj_set_grid_dsc_array(panel2, grid_chart_col_dsc, grid_chart_row_dsc);

    arc2 = lv_arc_create(panel2);
    lv_arc_set_value(arc2, 50);
    lv_obj_set_size(arc2, 130, 130);
    lv_arc_set_bg_angles(arc2, 135, 45);
    lv_obj_set_grid_cell(arc2, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 0, 2);
    lv_obj_add_event_cb(arc2, light_arc_cb, LV_EVENT_ALL, 0);

    title = lv_label_create(panel2);
    lv_label_set_text(title, "Light");
    lv_obj_add_style(title, &style_control, 0);
    lv_obj_set_grid_cell(title, LV_GRID_ALIGN_CENTER, 1, 2, LV_GRID_ALIGN_CENTER, 0, 1);

    sw2 = lv_switch_create(panel2);
    lv_obj_set_grid_cell(sw2, LV_GRID_ALIGN_CENTER, 1, 2, LV_GRID_ALIGN_CENTER, 1, 1);
    lv_obj_add_event_cb(sw2, light_sw_cb, LV_EVENT_ALL, 0);

    lv_obj_t * label2 = lv_label_create(arc2);
    lv_obj_align(label2, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(label2, "50");
    lv_obj_add_style(label2, &style_control, LV_STATE_DEFAULT);

    label2 = lv_label_create(panel2);
    lv_label_set_text(label2, "Light Threshold");
    lv_obj_add_style(label2, &style_control, LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(label2, LV_GRID_ALIGN_CENTER, 0, 3, LV_GRID_ALIGN_START, 2, 1);

    dsc = lv_label_create(panel2);
    lv_obj_add_style(dsc, &style_text_muted, 0);
    lv_label_set_text(dsc, "Set the low threshold at which the light\rneeds to be turned on by dragging the\rknob of arc above!" );
    lv_label_set_long_mode(dsc, LV_LABEL_LONG_WRAP);
    lv_obj_set_grid_cell(dsc, LV_GRID_ALIGN_START, 0, 2, LV_GRID_ALIGN_STRETCH, 3, 2);

    /***********************************
     * 第3控件，用于控制水泵，进而控制光照度 
     ***********************************/
    lv_obj_t * panel3 = lv_obj_create(parent);
    lv_obj_add_flag(panel3, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
    lv_obj_set_flex_grow(panel3, 1);
    lv_obj_set_height(panel3, LV_PCT(120));

    lv_obj_set_grid_dsc_array(panel3, grid_chart_col_dsc, grid_chart_row_dsc);

    // 以panel3为父对象创建一个slider对象
    slider1 = lv_slider_create(panel3);
    lv_obj_set_width(slider1, LV_PCT(95));
    lv_obj_refresh_ext_draw_size(slider1);
    lv_slider_set_mode(slider1, LV_SLIDER_MODE_RANGE);
    lv_slider_set_value(slider1, 50, LV_ANIM_OFF);
    lv_obj_set_grid_cell(slider1, LV_GRID_ALIGN_CENTER, 0, 3, LV_GRID_ALIGN_END, 0, 2);
    lv_obj_add_event_cb(slider1, humi_slider_cb, LV_EVENT_ALL, NULL);

    title = lv_label_create(panel3);
    lv_label_set_text(title, "Pump");
    lv_obj_add_style(title, &style_control, 0);
    lv_obj_set_grid_cell(title, LV_GRID_ALIGN_START, 0, 2, LV_GRID_ALIGN_START, 0, 1);

    sw3 = lv_switch_create(panel3);
    lv_obj_set_grid_cell(sw3, LV_GRID_ALIGN_CENTER, 1, 2, LV_GRID_ALIGN_START, 0, 1);
    lv_obj_add_event_cb(sw3, pump_sw_cb, LV_EVENT_ALL, 0);

    lv_obj_t * label = lv_label_create(panel3);
    lv_label_set_text(label, "Soil Humi Threshold");
    lv_obj_add_style(label, &style_control, LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(label, LV_GRID_ALIGN_CENTER, 0, 3, LV_GRID_ALIGN_START, 3, 2);

    dsc = lv_label_create(panel3);
    lv_obj_add_style(dsc, &style_text_muted, 0);
    lv_label_set_text(dsc, "Set the low and high threshold at which the\rpump needs to be turned on by dragging\rthe knob of slider above!" );
    lv_label_set_long_mode(dsc, LV_LABEL_LONG_WRAP);
    lv_obj_set_grid_cell(dsc, LV_GRID_ALIGN_START, 0, 2, LV_GRID_ALIGN_STRETCH, 4, 1);

}

static void color_changer_create(lv_obj_t * parent)
{
    static lv_palette_t palette[] = {
            LV_PALETTE_BLUE, LV_PALETTE_GREEN, LV_PALETTE_PINK,  LV_PALETTE_ORANGE,
            LV_PALETTE_RED, LV_PALETTE_PURPLE, LV_PALETTE_TEAL, _LV_PALETTE_LAST };

    lv_obj_t * color_cont = lv_obj_create(parent);
    lv_obj_remove_style_all(color_cont);
    lv_obj_set_flex_flow(color_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(color_cont, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_add_flag(color_cont, LV_OBJ_FLAG_FLOATING);

    lv_obj_set_style_bg_color(color_cont, lv_color_white(), 0);
    lv_obj_set_style_pad_right(color_cont, disp_size == DISP_SMALL ? LV_DPX(47) : LV_DPX(55), 0);
    lv_obj_set_style_bg_opa(color_cont, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(color_cont, LV_RADIUS_CIRCLE, 0);

    if(disp_size == DISP_SMALL) lv_obj_set_size(color_cont, LV_DPX(52), LV_DPX(52));
    else lv_obj_set_size(color_cont, LV_DPX(60), LV_DPX(60));

    lv_obj_align(color_cont, LV_ALIGN_BOTTOM_RIGHT, - LV_DPX(10),  - LV_DPX(10));

    // 调色盘选项栏
    uint32_t i;
    for(i = 0; palette[i] != _LV_PALETTE_LAST; i++) {
        lv_obj_t * c = lv_btn_create(color_cont);
        if(i == 2)  // 如果是自定义的粉色（由LV_PALETTE_PINK调色而来）
        {
            lv_obj_set_style_bg_color(c, lv_color_lighten(lv_palette_main(palette[i]), 180), 0);
        }
        else
        {
            lv_obj_set_style_bg_color(c, lv_palette_main(palette[i]), 0);
        }
        lv_obj_set_style_radius(c, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_opa(c, LV_OPA_TRANSP, 0);
        lv_obj_set_size(c, 20, 20);
        lv_obj_add_event_cb(c, color_event_cb, LV_EVENT_ALL, &palette[i]);
        lv_obj_clear_flag(c, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    }

    lv_obj_t * btn = lv_btn_create(parent);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_FLOATING | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_color(btn, lv_color_white(), LV_STATE_CHECKED);
    lv_obj_set_style_pad_all(btn, 10, 0);
    lv_obj_set_style_radius(btn, LV_RADIUS_CIRCLE, 0);
    // 注册改变风格颜色的回调函数
    lv_obj_add_event_cb(btn, color_changer_event_cb, LV_EVENT_ALL, color_cont);
    lv_obj_set_style_shadow_width(btn, 0, 0);
    lv_obj_set_style_bg_img_src(btn, LV_SYMBOL_TINT, 0);

    if(disp_size == DISP_SMALL) {
         lv_obj_set_size(btn, LV_DPX(42), LV_DPX(42));
         lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -LV_DPX(15), -LV_DPX(15));
     } else {
         lv_obj_set_size(btn, LV_DPX(50), LV_DPX(50));
         lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -LV_DPX(15), -LV_DPX(15));
     }
}

static void color_changer_anim_cb(void * var, int32_t v)
{
    lv_obj_t * obj = var;
    lv_coord_t max_w = lv_obj_get_width(lv_obj_get_parent(obj)) - LV_DPX(20);
    lv_coord_t w;

    if(disp_size == DISP_SMALL) {
        w = lv_map(v, 0, 256, LV_DPX(52), max_w);
        lv_obj_set_width(obj, w);
        lv_obj_align(obj, LV_ALIGN_BOTTOM_RIGHT, - LV_DPX(10),  - LV_DPX(10));
    } else {
        w = lv_map(v, 0, 256, LV_DPX(60), max_w);
        lv_obj_set_width(obj, w);
        lv_obj_align(obj, LV_ALIGN_BOTTOM_RIGHT, - LV_DPX(10),  - LV_DPX(10));
    }

    if(v > LV_OPA_COVER) v = LV_OPA_COVER;

    uint32_t i;
    for(i = 0; i < lv_obj_get_child_cnt(obj); i++) {
        lv_obj_set_style_opa(lv_obj_get_child(obj, i), v, 0);
    }

}

static void color_changer_event_cb(lv_event_t *e)
{
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        lv_obj_t * color_cont = lv_event_get_user_data(e);
        if(lv_obj_get_width(color_cont) < LV_HOR_RES / 2) {
            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, color_cont);
            lv_anim_set_exec_cb(&a, color_changer_anim_cb);
            lv_anim_set_values(&a, 0, 256);
            lv_anim_set_time(&a, 200);
            lv_anim_start(&a);
        } else {
            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, color_cont);
            lv_anim_set_exec_cb(&a, color_changer_anim_cb);
            lv_anim_set_values(&a, 256, 0);
            lv_anim_set_time(&a, 200);
            lv_anim_start(&a);
        }
    }
}

static void color_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);

    if(code == LV_EVENT_FOCUSED) {
        lv_obj_t * color_cont = lv_obj_get_parent(obj);
        if(lv_obj_get_width(color_cont) < LV_HOR_RES / 2) {
            lv_anim_t a;
            lv_anim_init(&a);
            lv_anim_set_var(&a, color_cont);
            lv_anim_set_exec_cb(&a, color_changer_anim_cb);
            lv_anim_set_values(&a, 0, 256);
            lv_anim_set_time(&a, 200);
            lv_anim_start(&a);
        }
    }
    else if(code == LV_EVENT_CLICKED) {
        lv_color_t color;
        lv_palette_t * palette_primary = lv_event_get_user_data(e);
        lv_palette_t palette_secondary = (*palette_primary) + 3; /*Use another palette as secondary*/
        if(palette_secondary >= _LV_PALETTE_LAST) palette_secondary = 0;
        /* 由于LVGL提供的调色盘中的粉色颜色太深，不是我想要的粉嫩嫩的那种，所以要进行提亮处理
         * 在color_changer_create函数中已经对这个配色风格的选项进行了提亮处理，
         * 但是选项所显示的颜色并不代表最终改变的颜色，此处的处理才是对所有粉色进行提亮处理 */
        if(* palette_primary == LV_PALETTE_PINK)  // 如果传递的参数是粉色，就做提亮处理
        {
            lv_theme_default_init(NULL, lv_color_lighten(lv_palette_main(*palette_primary), 180), lv_palette_main(palette_secondary), LV_THEME_DEFAULT_DARK, font_normal);
            color = lv_palette_main(*palette_primary);
            color = lv_color_lighten(color, 180);
        }
        else 
        {
            lv_theme_default_init(NULL, lv_palette_main(*palette_primary), lv_palette_main(palette_secondary), LV_THEME_DEFAULT_DARK, font_normal);
            color = lv_palette_main(*palette_primary);
        }    
// #if LV_USE_THEME_DEFAULT
//         lv_theme_default_init(NULL, lv_palette_main(*palette_primary), lv_palette_main(palette_secondary), LV_THEME_DEFAULT_DARK, font_normal);
// #endif
        lv_style_set_text_color(&style_icon, color);
        lv_chart_set_series_color(chart1, ser1, color);
        lv_chart_set_series_color(chart2, ser2, color);
        lv_chart_set_series_color(chart3, ser3, color);
    }
}

static void ta_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);
    lv_obj_t * kb = lv_event_get_user_data(e);
    if(code == LV_EVENT_FOCUSED) {
        if(lv_indev_get_type(lv_indev_get_act()) != LV_INDEV_TYPE_KEYPAD) {
            lv_keyboard_set_textarea(kb, ta);
            lv_obj_set_style_max_height(kb, LV_HOR_RES * 2 / 3, 0);
            lv_obj_update_layout(tv);   /*Be sure the sizes are recalculated*/
            lv_obj_set_height(tv, LV_VER_RES - lv_obj_get_height(kb));
            lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
            lv_obj_scroll_to_view_recursive(ta, LV_ANIM_OFF);
        }
    }
    else if(code == LV_EVENT_DEFOCUSED) {
        lv_keyboard_set_textarea(kb, NULL);
        lv_obj_set_height(tv, LV_VER_RES);
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
        lv_indev_reset(NULL, ta);

    }
    else if(code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
        lv_obj_set_height(tv, LV_VER_RES);
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_state(ta, LV_STATE_FOCUSED);
        lv_indev_reset(NULL, ta);   /*To forget the last clicked object to make it focusable again*/
        if(ta == user_name)
        {
            wifi_ssid = lv_textarea_get_text(ta);
            ESP_LOGI(TAG, "Ready, current ssid: %d", wifi_ssid[5]);
            // ESP_LOGI(TAG, "Ready, current password: %s", wifi_password);
        }
        else if(ta == password)
        {
            wifi_password = lv_textarea_get_text(ta);
            ESP_LOGI(TAG, "Ready, current ssid: %s", wifi_ssid);
            ESP_LOGI(TAG, "Ready, current password: %s", wifi_password);
        } 
    }
}

static void connect_btn_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED)
    {
        wifi_init_sta(wifi_connect_flag, wifi_ssid, wifi_password);
        wifi_connect_flag |= 1;
        if(mqtt_connect_flag)
        {
            esp_mqtt_client_destroy(client);
        }
        // esp_log_level_set("*", ESP_LOG_INFO);
        // esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
        // esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
        // esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
        // esp_log_level_set("TRANSPORT_BASE", ESP_LOG_VERBOSE);
        // esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
        // esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);
        // esp_event_loop_create_default();
        mqtt_app_start();
    }
    
}

static void chart_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    // 按下或者释放的时候
    if(code == LV_EVENT_PRESSED || code == LV_EVENT_RELEASED) {
        lv_obj_invalidate(obj); /*To make the value boxes visible*/
    }
    else if(code == LV_EVENT_DRAW_PART_BEGIN) {
        // 获取作为参数传回的控件
        lv_obj_draw_part_dsc_t * dsc = lv_event_get_param(e);
        /*Set the markers' text*/
        // 如果是X轴刻度
        if(dsc->part == LV_PART_TICKS && dsc->id == LV_CHART_AXIS_PRIMARY_X) {
                const char * time[] = {"-5.5h", "-5h", "-4.5h", "-4h", "-3.5h", "-3h", "-2.5h", "-2h", "-1.5h", "-1h", "-0.5h", "Now"};
                lv_snprintf(dsc->text, dsc->text_length, "%s", time[dsc->value]);
        }

        /*Add the faded area before the lines are drawn */
        else if(dsc->part == LV_PART_ITEMS) {
#if LV_DRAW_COMPLEX
            /*Add  a line mask that keeps the area below the line*/
            if(dsc->p1 && dsc->p2) {
                lv_draw_mask_line_param_t line_mask_param;
                lv_draw_mask_line_points_init(&line_mask_param, dsc->p1->x, dsc->p1->y, dsc->p2->x, dsc->p2->y, LV_DRAW_MASK_LINE_SIDE_BOTTOM);
                int16_t line_mask_id = lv_draw_mask_add(&line_mask_param, NULL);

                /*Add a fade effect: transparent bottom covering top*/
                lv_coord_t h = lv_obj_get_height(obj);
                lv_draw_mask_fade_param_t fade_mask_param;
                lv_draw_mask_fade_init(&fade_mask_param, &obj->coords, LV_OPA_COVER, obj->coords.y1 + h / 8, LV_OPA_TRANSP, obj->coords.y2);
                int16_t fade_mask_id = lv_draw_mask_add(&fade_mask_param, NULL);

                /*Draw a rectangle that will be affected by the mask*/
                lv_draw_rect_dsc_t draw_rect_dsc;
                lv_draw_rect_dsc_init(&draw_rect_dsc);
                draw_rect_dsc.bg_opa = LV_OPA_50;
                draw_rect_dsc.bg_color = dsc->line_dsc->color;

                lv_area_t obj_clip_area;
                _lv_area_intersect(&obj_clip_area, dsc->draw_ctx->clip_area, &obj->coords);
                const lv_area_t * clip_area_ori = dsc->draw_ctx->clip_area;
                dsc->draw_ctx->clip_area = &obj_clip_area;
                lv_area_t a;
                a.x1 = dsc->p1->x;
                a.x2 = dsc->p2->x - 1;
                a.y1 = LV_MIN(dsc->p1->y, dsc->p2->y);
                a.y2 = obj->coords.y2;
                lv_draw_rect(dsc->draw_ctx, &draw_rect_dsc, &a);
                dsc->draw_ctx->clip_area = clip_area_ori;
                /*Remove the masks*/
                lv_draw_mask_remove_id(line_mask_id);
                lv_draw_mask_remove_id(fade_mask_id);
            }
#endif


            const lv_chart_series_t * ser = dsc->sub_part_ptr;

            if(lv_chart_get_pressed_point(obj) == dsc->id) {
                if(lv_chart_get_type(obj) == LV_CHART_TYPE_LINE) {
                    dsc->rect_dsc->outline_color = lv_color_white();
                    dsc->rect_dsc->outline_width = 2;
                } else {
                    dsc->rect_dsc->shadow_color = ser->color;
                    dsc->rect_dsc->shadow_width = 15;
                    dsc->rect_dsc->shadow_spread = 0;
                }

                char buf[8];
                lv_snprintf(buf, sizeof(buf), "%"LV_PRIu32, dsc->value);

                lv_point_t text_size;
                lv_txt_get_size(&text_size, buf, font_normal, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);

                lv_area_t txt_area;
                if(lv_chart_get_type(obj) == LV_CHART_TYPE_BAR) {
                    txt_area.y2 = dsc->draw_area->y1 - LV_DPX(15);
                    txt_area.y1 = txt_area.y2 - text_size.y;
                    if(ser == lv_chart_get_series_next(obj, NULL)) {
                        txt_area.x1 = dsc->draw_area->x1 + lv_area_get_width(dsc->draw_area) / 2;
                        txt_area.x2 = txt_area.x1 + text_size.x;
                    } else {
                        txt_area.x2 = dsc->draw_area->x1 + lv_area_get_width(dsc->draw_area) / 2;
                        txt_area.x1 = txt_area.x2 - text_size.x;
                    }
                } else {
                    txt_area.x1 = dsc->draw_area->x1 + lv_area_get_width(dsc->draw_area) / 2 - text_size.x / 2;
                    txt_area.x2 = txt_area.x1 + text_size.x;
                    txt_area.y2 = dsc->draw_area->y1 - LV_DPX(15);
                    txt_area.y1 = txt_area.y2 - text_size.y;
                }

                lv_area_t bg_area;
                bg_area.x1 = txt_area.x1 - LV_DPX(8);
                bg_area.x2 = txt_area.x2 + LV_DPX(8);
                bg_area.y1 = txt_area.y1 - LV_DPX(8);
                bg_area.y2 = txt_area.y2 + LV_DPX(8);

                lv_draw_rect_dsc_t rect_dsc;
                lv_draw_rect_dsc_init(&rect_dsc);
                rect_dsc.bg_color = ser->color;
                rect_dsc.radius = LV_DPX(5);
                lv_draw_rect(dsc->draw_ctx, &rect_dsc, &bg_area);

                lv_draw_label_dsc_t label_dsc;
                lv_draw_label_dsc_init(&label_dsc);
                label_dsc.color = lv_color_white();
                label_dsc.font = font_normal;
                lv_draw_label(dsc->draw_ctx, &label_dsc, &txt_area,  buf, NULL);
            } else {
                dsc->rect_dsc->outline_width = 0;
                dsc->rect_dsc->shadow_width = 0;
            }
        }
    }
}

static void motor_sw_cb(lv_event_t * e)
{

    lv_event_code_t code = lv_event_get_code(e);
    // lv_obj_t * obj = lv_event_get_target(e);
    if(code == LV_EVENT_PRESSED)
    {
        motor_mannual_flag = !motor_mannual_flag;
        if(lv_obj_has_state(sw1, LV_STATE_CHECKED))  // 返回的是改变前的值
        {
            motor_set_pwm(100);
            motor_status = 0;
        }
        else
        {
            motor_set_pwm(0);
            motor_status = 1;
        }
    }

}

static void light_sw_cb(lv_event_t * e)
{

    lv_event_code_t code = lv_event_get_code(e);
    // lv_obj_t * obj = lv_event_get_target(e);
    if(code == LV_EVENT_PRESSED)
    {
        light_mannual_flag = !light_mannual_flag;
        if(lv_obj_has_state(sw2, LV_STATE_CHECKED))
        {
            led_set_lux(0);
            light_set_pwm(100);
            light_status = 0;
        }
        else
        {
            led_set_lux(100);
            light_set_pwm(0);
            light_status = 1;
        }
    }

}

static void pump_sw_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    // lv_obj_t * obj = lv_event_get_target(e);
    if(code == LV_EVENT_PRESSED)
    {
        // do something with pump
        pump_mannual_flag = !pump_mannual_flag;         // 进入人工控制状态
        if(lv_obj_has_state(sw3, LV_STATE_CHECKED))
        {
            pump_set_pwm(100);        // 高电平，触发继电器，打开泵机
            pump_status = 0;
        }
        else
        {
            pump_set_pwm(0);      // 低电平，不触发继电器，关闭泵机
            pump_status = 1;
        }
    }

}

static void temp_arc_cb(lv_event_t * e)
{
    int angle = 0;
    lv_obj_t * arc = lv_event_get_target(e);            //获取当前事件对象
    angle = lv_arc_get_value(arc);                      // 获取对象的值（以百分比表示）
    angle = angle * 5;
    static char buf[8];
	lv_snprintf(buf, sizeof(buf), "%d", angle);			// 将值变为字符串
	lv_obj_t * label = lv_obj_get_child(arc, 0);		// 获取事件对象的标签子对象
	lv_label_set_text(label, buf);						// 设置标签文本
	lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);	        // 标签文件有改变要重新设置对齐
}

static void light_arc_cb(lv_event_t * e)
{
    // lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * arc = lv_event_get_target(e);        //获取当前事件对象
    light_high_threshold = lv_arc_get_value(arc);                      // 获取对象的值（以百分比表示）
    // ESP_LOGI(TAG, "Now we set light_high_threshold to:%d", light_high_threshold);
    static char buf[8];
	lv_snprintf(buf, sizeof(buf), "%d", light_high_threshold);			// 将值变为字符串
	lv_obj_t * label = lv_obj_get_child(arc, 0);		// 获取事件对象的标签子对象
	lv_label_set_text(label, buf);						// 设置标签文本
	lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);	        // 标签文件有改变要重新设置对齐
}

static void humi_slider_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);

    if(code == LV_EVENT_REFR_EXT_DRAW_SIZE) {
        lv_coord_t *s = lv_event_get_param(e);
        *s = LV_MAX(*s, 60);
    } 
    else if(code == LV_EVENT_DRAW_PART_END) {
        // 获取对象
        lv_obj_draw_part_dsc_t * dsc = lv_event_get_param(e);
        // 如果是对象的手柄被按下
        if(dsc->part == LV_PART_KNOB && dsc->id == 0 && lv_obj_has_state(obj, LV_STATE_PRESSED)) {
            // 将获取的值转化成字符串，放进buf里
            soil_humi_high_threshold = lv_slider_get_value(obj);
            char buf[8];
            lv_snprintf(buf, sizeof(buf), "%"LV_PRId32, lv_slider_get_value(obj));

            lv_point_t text_size;
            lv_txt_get_size(&text_size, buf, font_normal, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);

            lv_area_t txt_area;
            txt_area.x1 = dsc->draw_area->x1 + lv_area_get_width(dsc->draw_area) / 2 - text_size.x / 2;
            txt_area.x2 = txt_area.x1 + text_size.x;
            txt_area.y2 = dsc->draw_area->y1 - 10;
            txt_area.y1 = txt_area.y2 - text_size.y;

            lv_area_t bg_area;
            bg_area.x1 = txt_area.x1 - LV_DPX(8);
            bg_area.x2 = txt_area.x2 + LV_DPX(8);
            bg_area.y1 = txt_area.y1 - LV_DPX(8);
            bg_area.y2 = txt_area.y2 + LV_DPX(8);

            // 画一个圆角矩形，用来显示手柄的值
            lv_draw_rect_dsc_t rect_dsc;
            lv_draw_rect_dsc_init(&rect_dsc);
            rect_dsc.bg_color = lv_palette_darken(LV_PALETTE_GREY, 3);
            rect_dsc.radius = LV_DPX(5);
            lv_draw_rect(dsc->draw_ctx, &rect_dsc, &bg_area);

            // 用来显示设置值的label对象
            lv_draw_label_dsc_t label_dsc;
            lv_draw_label_dsc_init(&label_dsc);
            label_dsc.color = lv_color_white();
            label_dsc.font = font_normal;
            lv_draw_label(dsc->draw_ctx, &label_dsc, &txt_area, buf, NULL);
        }
    }
    else if(code == LV_EVENT_DRAW_PART_BEGIN) {
        // 获取对象
        lv_obj_draw_part_dsc_t * dsc = lv_event_get_param(e);
        // 如果是对象的手柄被按下
        if(dsc->part == LV_PART_KNOB && dsc->id == 1 && lv_obj_has_state(obj, LV_STATE_PRESSED)) {
            // 将获取的值转化成字符串，放进buf里
            soil_humi_low_threshold = lv_slider_get_left_value(obj);
            char buf[8];
            lv_snprintf(buf, sizeof(buf), "%"LV_PRId32, lv_slider_get_left_value(obj));

            lv_point_t text_size;
            lv_txt_get_size(&text_size, buf, font_normal, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);

            lv_area_t txt_area;
            txt_area.x1 = dsc->draw_area->x1 + lv_area_get_width(dsc->draw_area) / 2 - text_size.x / 2;
            txt_area.x2 = txt_area.x1 + text_size.x;
            txt_area.y2 = dsc->draw_area->y1 - 10;
            txt_area.y1 = txt_area.y2 - text_size.y;

            lv_area_t bg_area;
            bg_area.x1 = txt_area.x1 - LV_DPX(8);
            bg_area.x2 = txt_area.x2 + LV_DPX(8);
            bg_area.y1 = txt_area.y1 - LV_DPX(8);
            bg_area.y2 = txt_area.y2 + LV_DPX(8);

            // 画一个圆角矩形，用来显示手柄的值
            lv_draw_rect_dsc_t rect_dsc;
            lv_draw_rect_dsc_init(&rect_dsc);
            rect_dsc.bg_color = lv_palette_darken(LV_PALETTE_GREY, 3);
            rect_dsc.radius = LV_DPX(5);
            lv_draw_rect(dsc->draw_ctx, &rect_dsc, &bg_area);

            // 用来显示设置值的label对象
            lv_draw_label_dsc_t label_dsc;
            lv_draw_label_dsc_init(&label_dsc);
            label_dsc.color = lv_color_white();
            label_dsc.font = font_normal;
            lv_draw_label(dsc->draw_ctx, &label_dsc, &txt_area, buf, NULL);
        }
    }
}
