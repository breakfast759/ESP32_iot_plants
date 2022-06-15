#include <stdio.h>
#include "dht11.h"
#include "esp_log.h"

#define TAG "dht11"

int temp_x10 = 123;
int humidity = 60;
const int channel = 1;

uint8_t DHT11_PIN = -1;
/*RMT分频后可以获得一个方波信号，这里将这个方波信号的周期称为tick，
之后由RMT生成的信号都是基于tick的
比如说tick为1us，那么生成信号低电平或高电平时间必然是n*tick，n=0，1，2，...*/
void dht11_init(uint8_t dht11_pin)
{
	DHT11_PIN = dht11_pin;
	rmt_config_t rmt_rx = RMT_DEFAULT_CONFIG_RX(dht11_pin, channel);
	const int RMT_CLK_DIV = 80;                                   // RMT计数器时钟分频器
	const int RMT_TICK_10_US = (80000000 / RMT_CLK_DIV / 100000); // RMT计数器10us.(时钟源是APB时钟)
	const int rmt_item32_tIMEOUT_US = 1000;                       // RMT接收超时us
	// rmt_config_t rmt_rx;  //定义一个rmt_config_t的对象用于配置rmt驱动
	rmt_rx.gpio_num = dht11_pin;  //用于发送或接收的GPIO
	rmt_rx.channel = channel;  //选择通道0
	//一般来说RMT的时钟源是APB CLK，默认是80MHz，80分频就是说RMT时钟频率为1MHz
	//当RMT_CHANNEL_FLAGS_AWARE_DFS的标志位被设置时, RMT时钟源可以改为 REF_TICK 或者 XTAL.
	rmt_rx.clk_div = RMT_CLK_DIV;  //确定由 RMT 发送器生成或由接收器识别的脉冲长度范围
	rmt_rx.mem_block_num = 1;  //使用1个内存块，即最多接收64个高低电平信息
	rmt_rx.rmt_mode = RMT_MODE_RX;  //用于接收
	//rmt_config_t的成员rx_config是一个结构体成员
	rmt_rx.rx_config.filter_en = false;  //关滤波器
	rmt_rx.rx_config.filter_ticks_thresh = 100;
	rmt_rx.rx_config.idle_threshold = rmt_item32_tIMEOUT_US / 10 * (RMT_TICK_10_US);
	ESP_ERROR_CHECK(rmt_config(&rmt_rx));
	//给RMT RX的环形缓冲区分配1000字节大小
	ESP_ERROR_CHECK(rmt_driver_install(rmt_rx.channel, 1000, 0));
}

// 将RMT读取到的脉冲数据处理为温度和湿度
static int parse_items(rmt_item32_t *item, int item_num, int *humidity, int *temp_x10)
{
	int i = 0;
	unsigned rh = 0, temp = 0, checksum = 0;
	if (item_num < 41){					// 检查是否有足够的脉冲数，开始信号+40位数据+结束信号
		return 0;
	}
	// item++;								// 跳过开始信号脉冲
	for (i = 0; i < 16; i++, item++){	// 提取湿度数据，前两个字节共16位是湿度数据
		rh = (rh << 1) + (item->duration1 < 35 ? 0 : 1);  //高电平<35us就是0，不然就是1，这是由DHT11决定的
	}
	for (i = 0; i < 16; i++, item++){	// 提取温度数据
		temp = (temp << 1) + (item->duration1 < 35 ? 0 : 1);
	}
	for (i = 0; i < 8; i++, item++){	// 提取校验数据
		checksum = (checksum << 1) + (item->duration1 < 35 ? 0 : 1);
	}
	// 检查校验
	if ((((temp >> 8) + temp + (rh >> 8) + rh) & 0xFF) != checksum){
		printf("Checksum failure %4X %4X %2X\n", temp, rh, checksum);
		return 0;
	}
	// 返回数据
	*humidity = (rh >> 8) * 10 + (rh & 0xFF);  //因为接收到的是16位数据，换算的时候要把低8位去掉（因为都是0）
	*temp_x10 = (temp >> 8) * 10 + (temp & 0xFF);
	return 1;
}


// 使用RMT接收DHT11数据
int dht11_start_get(int *temp_x10, int *humidity)
{
	RingbufHandle_t rb = NULL;
	size_t rx_size = 0;
	rmt_item32_t *item;
	int rtn = 0;
	//获得RMT RX环形缓冲区句柄，并处理RX数据
	rmt_get_ringbuf_handle(channel, &rb);
	if (!rb){
		return 0;
	}
	//发送20ms脉冲启动DHT11单总线
	gpio_set_level(DHT11_PIN, 1);
	gpio_set_direction(DHT11_PIN, GPIO_MODE_OUTPUT);
	ets_delay_us(1000);
	gpio_set_level(DHT11_PIN, 0);
	ets_delay_us(20000);

	//将rmt_rx_start和rmt_rx_stop放入缓存
	//启动RMT接收DHT11的应答信号？删掉好像也没影响
	// rmt_rx_start(channel, 1);
	// rmt_rx_stop(channel);

	//信号线设置为输入准备接收数据
	gpio_set_level(DHT11_PIN, 1);
	gpio_set_direction(DHT11_PIN, GPIO_MODE_INPUT);
	ets_delay_us(30);									// 传感器转换需要时间

	//这次启动RMT接收器以获取数据
	rmt_rx_start(channel, 1);

	//从环形缓冲区中取出数据
	item = (rmt_item32_t *)xRingbufferReceive(rb, &rx_size, 2);
	if (item){
		int n;
		n = rx_size / 4 - 0;
		// vTaskDelay(10 / portTICK_PERIOD_MS);
		// ESP_LOGI(TAG, "n is: %d", n);
		// 解析来自ringbuffer的数据值.
		rtn = parse_items(item, n, humidity, temp_x10);
		// 解析数据后，将空格返回到ringbuffer.
		vRingbufferReturnItem(rb, (void *)item);
	}
	//停止RMT接收
	rmt_rx_stop(channel);
	return rtn;
}
