<% cmake = "$" %>\
cmake_minimum_required(VERSION 3.16)
project(${project['name']} LANGUAGES C CXX)

if(NOT DEFINED ENV{CXCP_SRC_PATH} OR "$ENV{CXCP_SRC_PATH}" STREQUAL "")
    message(FATAL_ERROR "CXCP_SRC_PATH must point to the cXCP repository root before configuring this project.")
endif()
set(CXCP_SRC_PATH "$ENV{CXCP_SRC_PATH}")
if(NOT EXISTS "${cmake}{CXCP_SRC_PATH}/inc/xcp.h")
    message(FATAL_ERROR "CXCP_SRC_PATH='${cmake}{CXCP_SRC_PATH}' does not contain inc/xcp.h.")
endif()

set(CXCP_SOURCES
    "${cmake}{CXCP_SRC_PATH}/src/xcp.c"
    "${cmake}{CXCP_SRC_PATH}/src/xcp_checksum.c"
    "${cmake}{CXCP_SRC_PATH}/src/xcp_daq.c"
    "${cmake}{CXCP_SRC_PATH}/src/xcp_timecorr.c"
    "${cmake}{CXCP_SRC_PATH}/src/xcp_util.c"
    "${cmake}{CXCP_SRC_PATH}/src/hw/options.c"
    "${cmake}{CXCP_SRC_PATH}/src/hw/terminal.c"
    "${cmake}{CXCP_SRC_PATH}/src/hw/threads.c"
    "${cmake}{CXCP_SRC_PATH}/src/tl/eth/common.c"
    "${cmake}{CXCP_SRC_PATH}/src/tl/xcp_tl_timeout.c"
    "${cmake}{CXCP_SRC_PATH}/flsemu/common.c"
    "${cmake}{CXCP_SRC_PATH}/flsemu/flsemu_xcp.c"
)

if(WIN32)
    list(APPEND CXCP_SOURCES
        "${cmake}{CXCP_SRC_PATH}/src/hw/win/hw.c"
        "${cmake}{CXCP_SRC_PATH}/src/tl/eth/wineth.c"
        "${cmake}{CXCP_SRC_PATH}/flsemu/win/flsemu.c"
    )
    set(CXCP_PLATFORM_LIBS ws2_32 Iphlpapi)
elseif(APPLE)
    list(APPEND CXCP_SOURCES
        "${cmake}{CXCP_SRC_PATH}/src/hw/posix/hw.c"
        "${cmake}{CXCP_SRC_PATH}/src/hw/macos/hw.c"
        "${cmake}{CXCP_SRC_PATH}/src/tl/eth/linuxeth.c"
        "${cmake}{CXCP_SRC_PATH}/flsemu/posix/flsemu.c"
    )
    set(CXCP_PLATFORM_LIBS pthread)
else()
    list(APPEND CXCP_SOURCES
        "${cmake}{CXCP_SRC_PATH}/src/hw/posix/hw.c"
        "${cmake}{CXCP_SRC_PATH}/src/hw/linux/hw.c"
        "${cmake}{CXCP_SRC_PATH}/src/tl/eth/linuxeth.c"
        "${cmake}{CXCP_SRC_PATH}/flsemu/posix/flsemu.c"
    )
    set(CXCP_PLATFORM_LIBS pthread rt dl)
endif()

add_executable(${project['name']} main.cpp application.cpp ${cmake}{CXCP_SOURCES})
target_include_directories(${project['name']} PRIVATE
    "${cmake}{CMAKE_CURRENT_SOURCE_DIR}" "${cmake}{CXCP_SRC_PATH}/inc" "${cmake}{CXCP_SRC_PATH}/flsemu")
target_compile_features(${project['name']} PRIVATE c_std_11 cxx_std_17)
target_link_libraries(${project['name']} PRIVATE ${cmake}{CXCP_PLATFORM_LIBS})
if(NOT MSVC)
    target_compile_definitions(${project['name']} PRIVATE _GNU_SOURCE)
endif()
install(TARGETS ${project['name']} RUNTIME DESTINATION "${cmake}{CMAKE_CURRENT_SOURCE_DIR}/.")
