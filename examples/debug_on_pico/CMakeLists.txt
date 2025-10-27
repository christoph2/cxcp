cmake_minimum_required(VERSION 3.20)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
set(FAMILY rp2040)
set(BOARD pico_sdk)

include($ENV{PICO_SDK_PATH}/lib/tinyusb/hw/bsp/family_support.cmake)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.4.0")
  message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.4.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

project(debug_on_pico C CXX ASM)

pico_sdk_init()

check_and_add_pico_pio_usb_support()

if (CMAKE_HOST_UNIX AND CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
  SET(MACOS_BUILD true)
else ()
  SET(MACOS_BUILD false)
endif()

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
  message(STATUS "Build type not specified: defaulting to release.")
endif()

IF (CMAKE_C_COMPILER_ID STREQUAL "GNU")
  SET(CMAKE_C_STANDARD 11)
ELSEIF(CMAKE_C_COMPILER_ID MATCHES "Clang")
  SET(CMAKE_C_STANDARD 11)
ELSE ()
  #message(WARNING "C standard could not be set because compiler is not GNU or Clang.")
ENDIF ()

option(DISABLE_SOCKET "Disable SocketCAN build" OFF)
option(PROFILE "Enable profiling" OFF)

set(SOURCES
        ../../src/xcp.c
        ../../src/xcp_checksum.c
        ../../src/xcp_daq.c
        ../../src/xcp_util.c
        ../../src/hw/pico/hw.c
        ../../src/tl/sxi/pico_cdc.c
        ../../src/tl/sxi/xcp_tl.c
        main.c
        ../../src/tl/xcp_tl_timeout.c
        usb_descriptors.c
)

set(INCLUDES
        .
        ../../inc
        ${PICO_SDK_PATH}/lib/tinyusb/hw
        ${PICO_SDK_PATH}/lib/tinyusb/src/portable/raspberrypi/rp2040
        ${PICO_SDK_PATH}/lib/tinyusb/src/portable/raspberrypi/rp2040/dcd_rp2040
)

if (MSVC)
  # Use case-insensitive match to catch Debug/Release values
  if (CMAKE_BUILD_TYPE MATCHES "^[Dd]ebug$")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Od /Zi /D_DEBUG")
  else ()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /O2 /Oy /DNDEBUG")
  endif ()
else ()
  #
  # clang or gcc
  #
  add_definitions(-D_GNU_SOURCE)

  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable  -ffunction-sections")

  if ("${PROFILE}" STREQUAL "ON")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg -fprofile-arcs -ftest-coverage --coverage")
  endif ()

  if (CMAKE_BUILD_TYPE MATCHES "^[Dd]ebug$")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb -D_DEBUG -Og -fno-omit-frame-pointer -fno-optimize-sibling-calls")
    if (NOT CMAKE_HOST_WIN32)
      #set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DFORTIFY_SOURCE=2 -fstack-protector-all --param=ssp-buffer-size=4 -fstack-check -fsanitize=address -fsanitize=undefined ")
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DFORTIFY_SOURCE=2 -fstack-protector-all --param=ssp-buffer-size=4 -fstack-check -fsanitize=undefined ")
      if (CMAKE_C_COMPILER_ID MATCHES "Clang")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wthread-safety")
      endif ()
    endif ()
  else ()
    # Treat non-Debug as Release here
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DNDEBUG -O2")
  endif ()
endif (MSVC)


# Pico-only target for this example
add_executable(debug_on_pico ${SOURCES} tusb_config.h)
target_include_directories(debug_on_pico PUBLIC ${INCLUDES}
        ${PICO_SDK_PATH}/lib/tinyusb/src/portable/raspberrypi/rp2040
        ${PICO_SDK_PATH}/lib/tinyusb/src/portable/raspberrypi/rp2040/dcd_rp2040
        ${PICO_SDK_PATH}/lib/tinyusb/hw
)
#

target_compile_features(debug_on_pico PRIVATE c_std_11)
# Link against Pico SDK libraries only
target_link_libraries(debug_on_pico pico_stdlib
        tinyusb_device
        tinyusb_board
)


# add_executable(debug_on_pico main.cpp)
# target_link_libraries(debug_on_pico pico_stdlib)
pico_add_extra_outputs(debug_on_pico)
pico_enable_stdio_usb(debug_on_pico 0)
pico_enable_stdio_uart(debug_on_pico 0)
