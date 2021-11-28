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
add_executable(e-paper
        e-paper.cpp
        pio_spi.c
        EPaperDevice.cpp)

pico_generate_pio_header(e-paper ${CMAKE_CURRENT_LIST_DIR}/spi.pio)
target_include_directories(e-paper PRIVATE .)
# pull in common dependencies
target_link_libraries(e-paper pico_stdlib hardware_spi hardware_pio Profiler)

# create map/bin/hex file etc.
pico_add_extra_outputs(e-paper)

# add url via pico_set_program_url
# example_auto_set_url(blink)
