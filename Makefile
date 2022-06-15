#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := ESP32_iot_plants

EXTRA_COMPONENT_DIRS = $(ESP_IDF)/examples/common_components/led_strip

include $(ESP_IDF)/make/project.mk
