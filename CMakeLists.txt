# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

set(EXTRA_COMPONENT_DIRS $ENV{ESP_IDF}/examples/common_components/led_strip
                         $ENV{ESP_IDF}/examples/common_components/protocol_examples_common)

include($ENV{ESP_IDF}/tools/cmake/project.cmake)
project(11_Version1.0.1.20220531_RC)
