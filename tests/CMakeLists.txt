cmake_minimum_required(VERSION 3.20...3.30)
project(cxcp_tests LANGUAGES C CXX)

if (POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif ()
cmake_policy(SET CMP0094 NEW)

find_package(Python3 COMPONENTS Interpreter Development.Module REQUIRED)
find_package(pybind11 CONFIG REQUIRED)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(CXCP_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(CXCP_WARNINGS_AS_ERRORS "Treat C compiler warnings as errors" ON)

set(EXTENSION_INCS
    ${CMAKE_CURRENT_SOURCE_DIR}/test_config
    ${CMAKE_CURRENT_SOURCE_DIR}/../inc
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../flsemu
    ${CMAKE_CURRENT_SOURCE_DIR}/../tools
)

function(configure_test_target target)
    if (MSVC)
        target_compile_options(${target} PRIVATE
            $<$<COMPILE_LANGUAGE:C>:/W4>
            $<$<COMPILE_LANGUAGE:C>:/WX>
        )
    else ()
        target_compile_options(${target} PRIVATE
            $<$<COMPILE_LANGUAGE:C>:-Wall>
            $<$<COMPILE_LANGUAGE:C>:-Wextra>
            $<$<COMPILE_LANGUAGE:C>:-Wpedantic>
            $<$<COMPILE_LANGUAGE:C>:-Warray-bounds>
        )
        if (CXCP_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE
                $<$<COMPILE_LANGUAGE:C>:-Werror>
                $<$<COMPILE_LANGUAGE:C>:-Wno-error=unused-parameter>
                $<$<COMPILE_LANGUAGE:C>:-Wno-error=unused-variable>
                $<$<COMPILE_LANGUAGE:C>:-Wno-error=unused-but-set-variable>
                $<$<COMPILE_LANGUAGE:C>:-Wno-error=missing-field-initializers>
            )
        endif ()
        if (CXCP_ENABLE_SANITIZERS)
            target_compile_options(${target} PRIVATE
                $<$<COMPILE_LANGUAGE:C>:-fsanitize=address,undefined>
                $<$<COMPILE_LANGUAGE:C>:-fno-omit-frame-pointer>
            )
            target_link_options(${target} PRIVATE -fsanitize=address,undefined)
        endif ()
    endif ()

    if (MSVC AND CXCP_ENABLE_SANITIZERS)
        target_compile_options(${target} PRIVATE $<$<COMPILE_LANGUAGE:C>:/fsanitize=address>)
    endif ()
endfunction()

pybind11_add_module(cxcp
    wrapper.cpp
    checksum_mocks.c
    ../src/xcp.c
    ../src/xcp_checksum.c
    ../src/xcp_daq.c
    ../src/xcp_util.c
    ../flsemu/common.c
    ../flsemu/flsemu_xcp.c
    cmake_test_hooks.c
    fls_mocks.c
    xcp_mocks.c
)
target_include_directories(cxcp PRIVATE ${EXTENSION_INCS})
target_compile_definitions(cxcp PRIVATE XCP_BUILD_TYPE=XCP_DEBUG_BUILD XCP_BUILD_TESTS=1)
configure_test_target(cxcp)
