# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# initalize pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH "/home/phooky/tools/pico/pico-sdk")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(Epson_CP C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(Epson_CP Epson_CP.c keymap.c video.c )

pico_set_program_name(Epson_CP "Epson_CP")
pico_set_program_version(Epson_CP "0.1")

pico_enable_stdio_uart(Epson_CP 0)
pico_enable_stdio_usb(Epson_CP 1)

pico_generate_pio_header(Epson_CP ${CMAKE_CURRENT_LIST_DIR}/video.pio)

# Add the standard library to the build
target_link_libraries(Epson_CP 
    pico_stdlib 
    pico_multicore
    hardware_pio
    )

pico_add_extra_outputs(Epson_CP)

