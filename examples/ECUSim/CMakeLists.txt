cmake_minimum_required(VERSION 3.10)
project(ECUSim LANGUAGES C CXX)

if (NOT CMAKE_BUILD_TYPE)
    # Default build type if not provided by the user
    if (NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
    endif ()

    # Set compiler-specific flags depending on CMAKE_BUILD_TYPE
    string(TOUPPER "${CMAKE_BUILD_TYPE}" _BUILD_TYPE_UPPER)

    if (MSVC)
        # MSVC: Debug uses /MDd and debug info, Release uses /MD and optimizations
        if (_BUILD_TYPE_UPPER STREQUAL "DEBUG")
            set(CMAKE_C_FLAGS_DEBUG "/MDd /Zi /Od" CACHE STRING "" FORCE)
            set(CMAKE_CXX_FLAGS_DEBUG "/MDd /Zi /Od" CACHE STRING "" FORCE)
        else ()
            set(CMAKE_C_FLAGS_RELEASE "/MD /O2 /DNDEBUG" CACHE STRING "" FORCE)
            set(CMAKE_CXX_FLAGS_RELEASE "/MD /O2 /DNDEBUG" CACHE STRING "" FORCE)
        endif ()
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        # GCC/Clang: Debug with -g -O0, Release with -O3 and -DNDEBUG
        if (_BUILD_TYPE_UPPER STREQUAL "DEBUG")
            set(CMAKE_C_FLAGS_DEBUG "-g -O0 -Wall" CACHE STRING "" FORCE)
            set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall" CACHE STRING "" FORCE)
        else ()
            set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG -Wall" CACHE STRING "" FORCE)
            set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -Wall" CACHE STRING "" FORCE)
        endif ()
    endif ()
    message(STATUS "Build type not specified: defaulting to Release.")
endif ()

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

set(SOURCES
        ../../src/xcp.c
        ../../src/xcp_checksum.c
        ../../src/xcp_daq.c
        ../../src/xcp_util.c
        ../../src/xcp_timecorr.c
        ../../src/tl/xcp_tl_timeout.c
        ../../src/hw/terminal.c
        ../../src/hw/options.c
        ../../src/hw/threads.c
        ../../flsemu/common.c
        ../../flsemu/flsemu_xcp.c
        app_config.c
        application.cpp
        main.cpp
        # ../../src/tl/eth/esp32.c
)

set(INCLUDES
        .
        ../../inc
        ../../flsemu
)

if (CMAKE_HOST_WIN32)
    list(APPEND SOURCES
            ../../src/hw/win/hw.c
            ../../flsemu/win/flsemu.c
            ../../src/tl/eth/wineth.c
            ../../src/tl/eth/common.c
    )
    set(PLATFORM_LIBRARIES ws2_32 Iphlpapi)
elseif (CMAKE_HOST_UNIX)
    list(APPEND SOURCES
            ../../src/hw/posix/hw.c
            ../../flsemu/posix/flsemu.c
            ../../src/tl/eth/linuxeth.c
            ../../src/tl/eth/common.c
    )
    if (CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
        list(APPEND SOURCES ../../src/hw/macos/hw.c)
    else ()
        list(APPEND SOURCES ../../src/hw/linux/hw.c)
        set(PLATFORM_LIBRARIES rt)
    endif ()
else ()
    message(FATAL_ERROR "ECUSim supports Windows, Linux, and macOS hosts only.")
endif ()

add_executable(ECUSim ${SOURCES})
target_include_directories(ECUSim PRIVATE ${INCLUDES})
target_compile_definitions(ECUSim PRIVATE XCP_TRANSPORT_LAYER=5)
target_link_libraries(ECUSim PRIVATE Threads::Threads ${PLATFORM_LIBRARIES})

if (MSVC)
    target_compile_options(ECUSim PRIVATE /W4)
else ()
    target_compile_options(ECUSim PRIVATE -Wall -Wextra -Wno-unused-parameter -g -ggdb)
    if (CMAKE_HOST_UNIX)
        target_compile_definitions(ECUSim PRIVATE _GNU_SOURCE)
    endif ()
    target_link_libraries(ECUSim PRIVATE m)
endif ()

if (MINGW)
    # Keep runtime DLLs toolchain-local to avoid incompatible copies from PATH.
    get_filename_component(MINGW_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
    foreach (RUNTIME_DLL IN ITEMS
            libstdc++-6.dll
            libgcc_s_seh-1.dll
            libgcc_s_dw2-1.dll
            libgcc_s_sjlj-1.dll
            libwinpthread-1.dll
            libssp-0.dll)
        if (EXISTS "${MINGW_BIN_DIR}/${RUNTIME_DLL}")
            add_custom_command(TARGET ECUSim POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${MINGW_BIN_DIR}/${RUNTIME_DLL}"
                    "$<TARGET_FILE_DIR:ECUSim>/${RUNTIME_DLL}")
        endif ()
    endforeach ()
endif ()
