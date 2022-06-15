/**
 * @file XPT2046.h
 *
 */

#ifndef XPT2046_H
#define XPT2046_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#include <stdint.h>
#include <stdbool.h>
#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

/*********************
 *      DEFINES
 *********************/
#define XPT2046_IRQ CONFIG_LV_TOUCH_PIN_IRQ
//my defines
//#define XPT2046_IRQ			36
#define XPT2046_MOSI		32
#define XPT2046_MISO		39
#define XPT2046_CLK			25
#define XPT2046_CS			33

#define XPT2046_AVG             4
#define XPT2046_X_MIN           CONFIG_LV_TOUCH_X_MIN
#define XPT2046_Y_MIN           CONFIG_LV_TOUCH_Y_MIN
#define XPT2046_X_MAX           CONFIG_LV_TOUCH_X_MAX
#define XPT2046_Y_MAX           CONFIG_LV_TOUCH_Y_MAX
#define XPT2046_X_INV           CONFIG_LV_TOUCH_INVERT_X
#define XPT2046_Y_INV           CONFIG_LV_TOUCH_INVERT_Y
#define XPT2046_XY_SWAP		    CONFIG_LV_TOUCH_XY_SWAP
#define XPT2046_TOUCH_THRESHOLD 400 // Threshold for touch detection
#define XPT2046_TOUCH_IRQ       CONFIG_LV_TOUCH_DETECT_IRQ
#define XPT2046_TOUCH_IRQ_PRESS CONFIG_LV_TOUCH_DETECT_IRQ_PRESSURE
#define XPT2046_TOUCH_PRESS     CONFIG_LV_TOUCH_DETECT_PRESSURE

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void xpt2046_init(void);
void xpt2046_init_multiplex(void);
bool xpt2046_read(lv_indev_drv_t * drv, lv_indev_data_t * data);
bool xpt2046_read_multiplex(lv_indev_drv_t * drv, lv_indev_data_t * data);

void xpt2046_gpio_write_byte(uint8_t data);		// 向XPT2016写入一个字节
uint16_t xpt2046_gpio_read_reg(uint8_t reg);

uint16_t TP_Read_XOY(uint8_t xy);
uint8_t TP_Read_XY(uint16_t *x,uint16_t *y);
uint8_t TP_Read_XY2(uint16_t *x,uint16_t *y);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* XPT2046_H */
