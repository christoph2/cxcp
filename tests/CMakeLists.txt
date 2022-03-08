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

option(PROFILE "Enable profiling" ON)

set(SOURCES
    checksum_mocks.c
    xcp_init.c
    ../src/xcp_checksum.c
    ../src/xcp_daq.c
    ../src/xcp_util.c
    ../flsemu/common.c
    fls_mocks.c
    xcp_mocks.c
)

set(INCLUDES
    .
    ../inc
    ../flsemu
)

if(WIN32)
    set(ADD_LIBS
        "ws2_32"
    )
elseif(UNIX)
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
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DNDEBUG -O3 -flto ")
endif()



if (WIN32)
    add_library(xcp_test SHARED ${SOURCES})
    target_compile_definitions(xcp_test PUBLIC TP_ETHER)
    target_include_directories(xcp_test PUBLIC ${INCLUDES})
    target_link_libraries(xcp_test ${ADD_LIBS})
    target_compile_features(xcp_test PRIVATE c_std_11)

elseif(UNIX)
    add_library(xcp_test SHARED ${SOURCES})
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        set_target_properties(xcp_test PROPERTIES_COMPILE_FLAGS "-m32" PROPERTIES_LINK_FLAGS "-m32")
    endif()
    target_compile_definitions(xcp_test PUBLIC TP_ETHER)
    target_include_directories(xcp_test PUBLIC ${INCLUDES})
    target_link_libraries(xcp_test ${ADD_LIBS})
    target_compile_features(xcp_test PRIVATE c_std_11)
endif()

