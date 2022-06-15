set(EXTRA_COMPONENT_DIRS $ENV{ESP_IDF}/examples/common_components/led_strip)

idf_component_register(SRCS "LED.c"
                    INCLUDE_DIRS "include"
                    REQUIRES    led_strip)
