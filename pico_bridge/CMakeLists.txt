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
    "./include/*.h"
    "./src/utils.*h"
    "./src/utils/*cpp"
    "./src/sys/*.h"
    "./src/sys/*.cpp"
    "./src/core1/*.h"
    "./src/core1/*.cpp"
)

add_executable(88-wifi ${SOURCE_FILES})

# Initalise the SDK
pico_sdk_init()

# Libraries
target_link_libraries(88-wifi
    pico_stdlib
    pico_cyw43_arch_lwip_threadsafe_background
    pico_multicore
    pico_lwip_sntp
    pico_aon_timer
    hardware_watchdog
    hardware_gpio
)

# Needed to pull in lwipopts.h in the ./src/include directory
target_include_directories(88-wifi PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/src/include
)

# pull in WiFI credentials from environment
target_compile_definitions(88-wifi PRIVATE
        WIFI_SSID="virus"
        WIFI_PASSWORD="sn0wball"
)

# Enable USB stdin/stdout
pico_enable_stdio_usb(88-wifi 1)
pico_enable_stdio_uart(88-wifi 0)

# Enable extra outputs
pico_add_extra_outputs(88-wifi)
