# README of 11 Version1.0.1.20220531 RC  
## 1 Overview  
* 修改了Kconfig/LVGL configuration/Feature configuring/Drawing/Default gradient buffer为32（原来为0）  
* 修改了Kconfig/LVGL ESP Driver/LVGL TFT Display controller/Select a custom frequency为80MHz（原来为40MHz）  
* 以上两个行为被认为是造成程序乱跑的罪魁祸首
* 去除了原来的灰色风格选项，替换为了樱花粉风格选项
## 2 TODO  
* 优化各个回调函数的代码  
* 实现自动扫描周围Wifi AP，并通过LVGL的Drop-down list组件展示出来的方式来确定要连接的Wifi名称  
* 实现chart新数据从左边显示，而不是右边   
* 实现半小时更新一次图表数据  
* 对云平台应用侧进行开发  
* 对土壤湿度ADC的换算公式进一步修正（目前会出现负数的情况）  
* 进一步完善土壤湿度测量的逻辑（明确数值大代表土壤越湿还是越干）  
## 3 Notice  
components中的LED组件，其中包含了几个不能用的函数，如果只需要LED开和关这两个状态，可以使用 *led_set_lux(uint16_t duty)* 函数，当 *duty* 为100时，就是完全打开，当 *duty* 为0时，就是完全关闭。   
## 4 Changelog  
### Version 1.0.0.20220529 RC  
* 修复了水泵控制逻辑的BUG  
* 开机将会自动连接到默认WiFi热点（ **ssid** ：12306； **password** ：zc20010623），如果连接三次没有连接成功，将会顺序执行初始化，可通过在ui界面手动输入待连接WiFi的ssid和password进行连接  
### Version 1.0.0.20220523 Base  
* 基本达成课设的软件功能，接下来主要进行细节修缮  
### Version 2.3.0.20220519 Base  
* 加入了mqtt_ssl组件，实现了实时上传温湿度和光照度信息至Onenet云平台的功能，实现云平台下发命令后进行解析并回复的功能  
* 完善了mqtt_ssl组件，目前可以上传环境温湿度、光照度等传感器采集到的信息和电机工作阈值、水泵工作阈值、补光灯工作阈值等用户配置信息。需要注意的是mqtt ssl组件与ui display组件之间是通过全局变量（... get）进行传递与交流的  
* 实现了云平台下发用户配置信息至嵌入式设备的功能，可以在云平台上进行电机工作阈值等的配置，嵌入式设备解析后会进行回复  
* 追加了pump、motor、illumination_adc组件，为需要用到的传感器和外部设备都准备好了程序接口  
* 进一步完善了mqtt_ssl组件，可以通过云平台打开和关闭设备  
### Version 2.2.0.20220519 Base  
* 对硬件进行了升级，将4M Flash的ESP32模组更换为了16M Flash的ESP32模组。需要注意的是，由于焊接原因，插入type-C接口时需要注意，否则会出现无法使用串口通讯的问题  
* 修复了手动打开外部设备时，会因为自动控制而无效的BUG  
* 增加了定时器执行计数变量，可以实现长时间计时  
* 删除了部分摸索学习期间的注释，如果需要进行学习，可以查看之前版本的代码  
* 再次修改了分区表，主要增加了factory区的大小，具体配置如下：  
<table>
    <tr>
        <td>Name</td>
        <td>Type</td>
        <td>Sub Type</td>
        <td>Offset</td>
        <td>Size</td>
        <td>Encrypted</td>
    </tr>
    <tr>
        <td>nvs</td>
        <td>data</td>
        <td>nvs</td>
        <td>0x9000</td>
        <td>0x6000</td>
        <td>No</td>
    </tr>
    <tr>
        <td>phy_int</td>
        <td>data</td>
        <td>phy</td>
        <td>0xf000</td>
        <td>0x1000</td>
        <td>No</td>
    </tr>
    <tr>
        <td>factory</td>
        <td>app</td>
        <td>factory</td>
        <td>0x10000</td>
        <td>4M</td>
        <td>No</td>
    </tr>
</table>  
### Version 2.1.0.20220514 Base  
* 将 *ui_display* 作为一个单独的组件从main组件中分离了出来，方便管理并优化编译链  
* 实现了输入Wifi名称和Wifi密码后手动连接的功能  
* 加入了 *solid_humi* 和 *dht11* 组件，实现了通过500ms定时器定期采集光照度，温度，环境湿度数据并显示在measuring界面的功能。  
* 修改了measuring界面chart1的纵坐标（0-500，用以显示温度，e.g.256代表25.6℃）和chart1，chart2，chart3的横坐标（-5.5h-Now，用以显示过去6小时内的温度）  
* 为了解决flash不足的问题，修改了分区表，不再使用默认配置，具体分区表如下：
<table>
    <tr>
        <td>Name</td>
        <td>Type</td>
        <td>Sub Type</td>
        <td>Offset</td>
        <td>Size</td>
        <td>Encrypted</td>
    </tr>
    <tr>
        <td>nvs</td>
        <td>data</td>
        <td>nvs</td>
        <td>0x9000</td>
        <td>0x6000</td>
        <td>No</td>
    </tr>
    <tr>
        <td>phy_int</td>
        <td>data</td>
        <td>phy</td>
        <td>0xf000</td>
        <td>0x1000</td>
        <td>No</td>
    </tr>
    <tr>
        <td>factory</td>
        <td>app</td>
        <td>factory</td>
        <td>0x10000</td>
        <td>2M</td>
        <td>No</td>
    </tr>
</table>
### Version 2.0.0.20220513 Base  
* 优化了 *sd_fatfs_br759* 组件，将其中一些与初始化无关的读写测试程序从初始化程序（ *sd_fatfs_config(void)* ）中移了出来，加快了系统初始化速度。  
* 基于LVGL8.2提供的demo *Show some widget* 设计了基本的界面，各个widget的回调函数已经定义好并已经能够实现一定的功能。  
### Version1.1.0.20220512 Base  
* 成功将LVGL8.2移植到了ESP32平台。  
* 成功将LVGL文件系统进行了对接。  
* 成功向LED组件中添加了PWM调光功能的代码。  
