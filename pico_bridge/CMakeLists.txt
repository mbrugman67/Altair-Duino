cmake_minimum_required(VERSION 3.28)

# C standard to follow
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# hardware type for RP2530 with WiFi
set(PICO_PLATFORM rp2350)
set(PICO_BOARD pico2_w)

# Include the .cmake file to get the SDK
include(./pico_sdk_import.cmake)

# Project declaration
project(88-wifi C CXX ASM)

file(GLOB SOURCE_FILES
    "./src/*.h"
    "./src/*.cpp"
    "./src/utils.h"
    "./src/utils.cpp"
    "./src/sys/*.h"
    "./src/sys/*.cpp"
)

add_executable(88-wifi ${SOURCE_FILES})

#${CMAKE_CURRENT_LIST_DIR}/build/generated)

# Initalise the SDK
pico_sdk_init()

# Libraries
target_link_libraries(88-wifi pico_stdlib pico_cyw43_arch_none)

# Enable USB, UART output
pico_enable_stdio_usb(88-wifi 1)
pico_enable_stdio_uart(88-wifi 0)

# Enable extra outputs
pico_add_extra_outputs(88-wifi)
