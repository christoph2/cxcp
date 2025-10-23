cmake_minimum_required(VERSION 3.20)

set(CMAKE_C_STANDARD 20)
set(CMAKE_CXX_STANDARD 20)


# set(PICO_SDK_PATH {sdk_path})
# set(PICO_BOARD {board_type} CACHE STRING \"Board type\")
# Pull in Raspberry Pi Pico SDK (must be before project)\

include(pico_sdk_import.cmake)
# include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.4.0")
  message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.4.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

project(debug_on_pico C CXX ASM)

pico_sdk_init()


add_executable(debug_on_pico main.cpp)
target_link_libraries(debug_on_pico pico_stdlib)
pico_add_extra_outputs(debug_on_pico)
pico_enable_stdio_usb(debug_on_pico 1)
pico_enable_stdio_uart(debug_on_pico 0)
