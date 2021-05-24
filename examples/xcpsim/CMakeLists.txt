cmake_minimum_required(VERSION 3.10)
project(xcpsim LANGUAGES C)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
    message(STATUS "Build type not specified: defaulting to release.")
endif()

IF(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    SET(CMAKE_C_STANDARD 11)
ELSEIF(CMAKE_C_COMPILER_ID MATCHES "Clang")
    SET(CMAKE_C_STANDARD 11)
ELSE()
    message(WARNING "C standard could not be set because compiler is not GNU or Clang.")
ENDIF()

option(DISABLE_SOCKET "Disable SocketCAN build" OFF)
option(PROFILE "Enable profiling" OFF)

set(SOURCES
    ../../src/xcp.c
    ../../src/xcp_checksum.c
    ../../src/xcp_daq.c
    ../../src/xcp_util.c
    ../../src/hw/terminal.c
    ../../src/hw/options.c
    ../../src/hw/threads.c
    ../../flsemu/common.c
    app_config.c
    appl1.c
    main.c
)

set(INCLUDES
    .
    ../../inc
    ../../flsemu
)

if(WIN32)
    set(ADD_LIBS
        "ws2_32"
    )
    list(APPEND SOURCES "../../src/hw/win/hw.c")
    list(APPEND SOURCES "../../flsemu/win/flsemu.c")
elseif(UNIX)
    list(APPEND SOURCES "../../src/hw/linux/hw.c")
    list(APPEND SOURCES "../../flsemu/posix/flsemu.c")
    set(ADD_LIBS
        "pthread"
        "rt"
    )
endif(WIN32)

IF(CMAKE_C_COMPILER_ID MATCHES "Clang")
    add_link_options("-fuse-ld=lld")
ENDIF()

add_definitions(-D_GNU_SOURCE)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable  -ffunction-sections")

if ("${PROFILE}" STREQUAL "ON")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg -fprofile-arcs -ftest-coverage --coverage")
endif()

if (CMAKE_BUILD_TYPE STREQUAL "DEBUG")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb -D_DEBUG -Og -fno-omit-frame-pointer -fno-optimize-sibling-calls")
    if (NOT WIN32)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DFORTIFY_SOURCE=2 -fstack-protector-all --param=ssp-buffer-size=4 -fstack-check -fsanitize=address -fsanitize=undefined ")
        if(CMAKE_C_COMPILER_ID MATCHES "Clang")
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wthread-safety")
        endif()
    endif()
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DNDEBUG -O3 -flto -fomit-frame-pointer")
endif()

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
endif()


if (WIN32)
    add_definitions(-DETHER)
    add_executable(xcp_sim ${SOURCES} "../../src/tl/eth/wineth.c" "../../src/tl/eth/common.c")
    target_include_directories(xcp_sim PUBLIC ${INCLUDES})
    target_link_libraries(xcp_sim ${ADD_LIBS})
    target_compile_features(xcp_sim PRIVATE c_std_11)
elseif(UNIX)
    add_executable(xcp_sim_eth ${SOURCES} "../../src/tl/eth/linuxeth.c" "../../src/tl/eth/common.c")
    target_compile_definitions(xcp_sim_eth PUBLIC ETHER)
    target_include_directories(xcp_sim_eth PUBLIC ${INCLUDES})
    target_link_libraries(xcp_sim_eth ${ADD_LIBS})
    target_compile_features(xcp_sim_eth PRIVATE c_std_11)

    add_executable(xcp_sim_can ${SOURCES} "../../src/tl/can/linux_socket_can.c")
    target_compile_definitions(xcp_sim_can PUBLIC SOCKET_CAN)
    target_include_directories(xcp_sim_can PUBLIC ${INCLUDES})
    target_link_libraries(xcp_sim_can ${ADD_LIBS})
    target_compile_features(xcp_sim_can PRIVATE c_std_11)
endif()

