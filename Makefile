#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := 11_Version1.0.1.20220531_RC

EXTRA_COMPONENT_DIRS = $(ESP_IDF)/examples/common_components/led_strip

include $(ESP_IDF)/make/project.mk
