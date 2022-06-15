idf_component_register(SRCS "mqtt_ssl.c"
                    INCLUDE_DIRS "include"
                    REQUIRES        protocol_examples_common
                                    mqtt
                                    app_update
                                    nvs_flash
                    EMBED_TXTFILES  MQTTS-certificate.pem
                                    mqtt_eclipseprojects_io.pem)
