idf_component_register(SRCS "Meichan3.c" "MingmingMeichan.c" "entrepreneurLiu.c" "ui_display.c" 
                    INCLUDE_DIRS "include"
                    REQUIRES    LED
                                pump
                                motor
                                light
                                lvgl
                                wifi_connection
                                solid_humi_adc
                                illuminance_adc
                                dht11
                                mqtt_ssl)
