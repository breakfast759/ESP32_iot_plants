idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS        "."
                    REQUIRES    LED
                                pump
                                motor
                                light
                                led_strip
                                lvgl_esp32_drivers
                                sd_fatfs_br759
                                ui_display
                                wifi_connection
                                mqtt_ssl
                                nvs_flash
                                solid_humi_adc
                                illuminance_adc
                                dht11)
