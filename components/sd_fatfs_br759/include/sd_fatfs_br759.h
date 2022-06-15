#ifndef _SD_FATFS_BR759_H_
#define _SD_FATFS_BR759_H_

#ifdef __cplusplus
extern "C" {
#endif
/*
    INCLUDE
*/
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "driver/sdspi_host.h"
#include "driver/spi_common.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"

/*
    DEFINE
*/
//在测试SD和SPI模式时，请记住，一旦在SPI模式下初始化了卡，在不接通卡电源的情况下就无法在SD模式下将其重新初始化。
#define PIN_NUM_MISO 19
#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK  18
#define PIN_NUM_CS   5

#define MOUNT_POINT		"/sdcard"		// 第一个字节必须是‘/’
#define SPI_DMA_CHAN    2

/*
    GLOBOL
*/
void sd_fatfs_config(void);

#endif /* LVGL_HELPERS_H */
