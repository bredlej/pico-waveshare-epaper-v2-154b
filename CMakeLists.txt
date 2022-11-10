cmake_minimum_required(VERSION 3.12)
include(${PICO_SDK_PATH}/pico_sdk_init.cmake)

project(e-paper C CXX ASM)
include(PicoSDKConfig.cmake)
pico_sdk_init()

if(DEFINED SYSPROGS_FRAMEWORKS_FILE)
include(${SYSPROGS_FRAMEWORKS_FILE})
endif()
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

# C library for handling PIO stuff
add_library(pio_handling pio_spi.c)
target_include_directories(pio_handling PRIVATE .)
target_link_libraries(pio_handling hardware_pio hardware_spi  pico_stdlib)
pico_generate_pio_header(pio_handling ${CMAKE_CURRENT_LIST_DIR}/spi.pio)

add_executable(e-paper
        e-paper.cpp
        Waveshare154V2b.cpp)
target_compile_options(e-paper PUBLIC -std=c++20)
target_include_directories(e-paper PRIVATE .)
target_link_libraries(e-paper pico_stdlib pio_handling hardware_gpio Profiler)
pico_enable_stdio_usb(e-paper 0)
pico_enable_stdio_uart(e-paper 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(e-paper)
