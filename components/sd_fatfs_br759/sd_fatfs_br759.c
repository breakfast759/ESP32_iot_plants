#include <stdio.h>
#include "sd_fatfs_br759.h"
#include "esp_log.h"
#include "lvgl.h"

static const char *TAG = "sd_fatfs_br759";

static void get_fatfs_usage(size_t* out_total_bytes, size_t* out_free_bytes);

void sd_fatfs_config(void)
{
	esp_err_t ret;								// ESP错误定义
	sdmmc_card_t* card;							// SD / MMC卡信息结构
	const char mount_point[] = MOUNT_POINT;		// 根目录

	esp_vfs_fat_sdmmc_mount_config_t mount_config = {	// 文件系统挂载配置
		.format_if_mount_failed = false,				// 如果挂载失败：true会重新分区和格式化/false不会重新分区和格式化
		.max_files = 5,									// 打开文件最大数量
		.allocation_unit_size = 16 * 1024
	};

	ESP_LOGI(TAG, "Initializing SD card\r\n");
	ESP_LOGI(TAG, "Using SPI peripheralr\r\n");

	sdmmc_host_t host = SDSPI_HOST_DEFAULT();
	spi_bus_config_t bus_cfg = {
		.mosi_io_num = PIN_NUM_MOSI,
		.miso_io_num = PIN_NUM_MISO,
		.sclk_io_num = PIN_NUM_CLK,
		.quadwp_io_num = -1,
		.quadhd_io_num = -1,
		.max_transfer_sz = 4 * 1024 * sizeof(uint8_t),
	};
	host.slot = 2;
	// SPI总线初始化
	ret = spi_bus_initialize(host.slot, &bus_cfg, SPI_DMA_CHAN);
	if (ret != ESP_OK) {
		// printf("%s->Failed to initialize bus.\r\n",TAG);
		ESP_LOGI(TAG, "Failed to initialize bus.\r\n");
		return;
	}
	// 这将初始化没有卡检测（CD）和写保护（WP）信号的插槽。
	// 如果您的主板有这些信号，请修改slot_config.gpio_cd和slot_config.gpio_wp。
	sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();// SPI2_HOST  1
	slot_config.gpio_cs = PIN_NUM_CS;
	slot_config.host_id = host.slot;
	// 挂载文件系统
	ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);
	if (ret != ESP_OK) {
		if (ret == ESP_FAIL) {
			printf("%s->Failed to mount filesystem.%s\r\n",TAG,
				"If you want the card to be formatted, set the EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
		} else {
			printf("%s->Failed to initialize the card %s  (%s). ",TAG,
				"Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
		}
		return;
	}
	// TF卡已经初始化，打印TF卡属性
	sdmmc_card_print_info(stdout, card);


	// Print FAT FS size information
	size_t bytes_total, bytes_free;
	get_fatfs_usage(&bytes_total, &bytes_free);
}

static void get_fatfs_usage(size_t* out_total_bytes, size_t* out_free_bytes)
{
	FATFS *fs;
	size_t free_clusters;
	int res = f_getfree("0:", &free_clusters, &fs);//
	assert(res == FR_OK);
	size_t total_sectors = (fs->n_fatent - 2) * fs->csize;
	size_t free_sectors = free_clusters * fs->csize;

	size_t sd_total = total_sectors/1024;
	size_t sd_total_KB = sd_total*fs->ssize;
	size_t sd_total_MB = (sd_total*fs->ssize)/1024;
	size_t sd_free = free_sectors/1024;
	size_t sd_free_KB = sd_free*fs->ssize;
	size_t sd_free_MB = (sd_free*fs->ssize)/1024;

	ESP_LOGI(TAG, "SD Cart sd_total_MB %d KByte", sd_total_MB);
	ESP_LOGI(TAG, "SD Cart sd_free_MB %d MByte", sd_free_MB);

	// 假设总大小小于4GiB，对于SPI Flash应该为true
	if (out_total_bytes != NULL) {
		*out_total_bytes = sd_total_KB;
	}
	if (out_free_bytes != NULL) {
		*out_free_bytes = sd_free_KB;
	}
}