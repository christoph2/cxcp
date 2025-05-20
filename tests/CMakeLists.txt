
cmake_minimum_required(VERSION 3.20...3.30)
project(cxcp LANGUAGES C CXX)


if (POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif ()
cmake_policy(SET CMP0094 NEW)

find_package(Python3 COMPONENTS Interpreter Development.Module REQUIRED)
find_package(pybind11 CONFIG)

SET(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 23)

message(STATUS "Found pybind11 v${pybind11_VERSION} ${pybind11_VERSION_TYPE}: ${pybind11_INCLUDE_DIRS}")

# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/dist")


SET(GCC_N_CLANG_BASE_OPTIONS "-std=c++23 -Wall -Wextra -Wpedantic -Warray-bounds -mtune=native -fexceptions")
SET(MSVC_BASE_OPTIONS "/W3 /permissive- /EHsc /bigobj /Zc:__cplusplus /std:c++latest")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    if (MSVC)
        SET(MSVC_BASE_OPTIONS "${MSVC_BASE_OPTIONS}  /Od /fsanitize=address /Zi")
    else ()
        SET(GCC_N_CLANG_BASE_OPTIONS "${GCC_N_CLANG_BASE_OPTIONS} -Og -g3 -ggdb -fno-omit-frame-pointer -fsanitize=address -fsanitize=undefined -fsanitize=bounds") # -fsanitize=hwaddress
    endif ()
else ()
    if (MSVC)
        SET(MSVC_BASE_OPTIONS "${MSVC_BASE_OPTIONS} /Ox")
    else ()
        SET(GCC_N_CLANG_BASE_OPTIONS "${GCC_N_CLANG_BASE_OPTIONS} -O3 -fomit-frame-pointer")
    endif ()
endif ()


if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(ENV{MACOSX_DEPLOYMENT_TARGET} "11.0")
    SET(GCC_N_CLANG_EXTRA_OPTIONS "-stdlib=libc++")
    message("Platform is Darwin")
elseif (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    message("Platform is WINDOWS")
    SET(MSVC_EXTRA_OPTIONS "")
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    SET(GCC_N_CLANG_EXTRA_OPTIONS "-fvisibility=hidden -g0") # -fcoroutines
    message("Platform is LINUX")
endif ()


IF (CMAKE_C_COMPILER_ID STREQUAL "GNU")

ELSEIF (CMAKE_C_COMPILER_ID MATCHES "Clang")

ELSEIF (CMAKE_C_COMPILER_ID MATCHES "MSVC")

ELSE ()

ENDIF ()

IF (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")

ELSEIF (CMAKE_CXX_COMPILER_ID MATCHES "Clang")

ELSEIF (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")

ELSE ()


ENDIF ()

message("Compiling C with: " ${CMAKE_C_COMPILER_ID})
message("Compiling Cpp with: " ${CMAKE_CXX_COMPILER_ID})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(EXTENSION_INCS ${CMAKE_CURRENT_SOURCE_DIR}/../inc ${CMAKE_CURRENT_SOURCE_DIR}/. ${CMAKE_CURRENT_SOURCE_DIR}/../flsemu)

pybind11_add_module(cxcp wrapper.cpp checksum_mocks.c ../src/xcp.c ../src/xcp_checksum.c
        ../src/xcp_daq.c ../src/xcp_util.c
        ../flsemu/common.c fls_mocks.c xcp_mocks.c
)
# xcp_init.c

target_include_directories(cxcp PRIVATE ${EXTENSION_INCS})

# install(TARGETS cxcp LIBRARY DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/.)