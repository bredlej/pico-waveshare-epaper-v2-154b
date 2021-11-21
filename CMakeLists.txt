cmake_minimum_required(VERSION 3.12)
include(${PICO_SDK_PATH}/pico_sdk_init.cmake)

project(e-paper C CXX ASM)
pico_sdk_init()

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
add_executable(blink
        blink.c
        pico-waveshare_ePaper_1_54in.c)

target_include_directories(blink PRIVATE .)
# pull in common dependencies
target_link_libraries(blink pico_stdlib hardware_spi)

# create map/bin/hex file etc.
pico_add_extra_outputs(blink)

# add url via pico_set_program_url
# example_auto_set_url(blink)