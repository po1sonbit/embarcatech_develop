# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(final_project_receptor C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(final_project_receptor 
                final_project_receptor.c
                inc/Lora/lora_sx1280.c 
                inc/Mqtt_Wifi/mqtt_wifi.c
                inc/SSD1306/ssd1306.c
                inc/SX1280/Settings.h
                inc/SX1280/SX1280_Def.h
                inc/SX1280/SX1280.h
                inc/SX1280/SX1280.c
                )

pico_set_program_name(final_project_receptor "final_project_receptor")
pico_set_program_version(final_project_receptor "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(final_project_receptor 0)
pico_enable_stdio_usb(final_project_receptor 1)

# Add the standard library to the build
target_link_libraries(final_project_receptor
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(final_project_receptor PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/inc
)

# Add any user requested libraries
target_link_libraries(final_project_receptor 
        hardware_spi
        hardware_i2c
        hardware_watchdog
        hardware_timer
        pico_cyw43_arch_lwip_threadsafe_background
        pico_lwip_mqtt
        )

pico_add_extra_outputs(final_project_receptor)

