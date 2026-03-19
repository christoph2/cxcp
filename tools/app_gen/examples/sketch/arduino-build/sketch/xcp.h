#line 1 "C:\\csProjects\\cxcp\\tools\\app_gen\\examples\\sketch\\xcp.h"

////////////////////////////////////////////////////////////////////////////////
//                                 xcp_log.h                                  //
////////////////////////////////////////////////////////////////////////////////

/*
 * BlueParrot XCP
 *
 * (C) 2007-2025 by Christoph Schueler <github.com/Christoph2,
 *                                      cpu12.gems@googlemail.com>
 *
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * s. FLOSS-EXCEPTION.txt
 */
#if !defined(__XCP_LOG_H)
    #define __XCP_LOG_H

    #if XCP_ENABLE_LOGGING == XCP_ON

        #if defined(_WIN32) || defined(__unix__) || defined(__APPLE__)
            #define DBG_PRINT(format, ...) printf(format, ##__VA_ARGS__)
            #define DBG_TRACE(format, ...) DBG_PRINT(format, ##__VA_ARGS__)
        #elif defined(PICO_BOARD)  // defined(RASPBERRYPI_PICO)
            #define DBG_PRINT(format, ...) tu_printf(format, ##__VA_ARGS__)
            #define DBG_TRACE(format, ...) DBG_PRINT(format, ##__VA_ARGS__)
        #elif defined(__XTENSA__)  // defined(ESP32) defined(ESP_IDF)

            #include "esp_log.h"

            #define DBG_PRINT(format, ...) ESP_LOGI("cXCP", format, ##__VA_ARGS__)
            #define DBG_TRACE(format, ...) ESP_LOGI("cXCP", format, ##__VA_ARGS__)
        #else
            #define DBG_PRINT(format, ...) ((void)0)
            #define DBG_TRACE(format, ...) ((void)0)
        #endif  // defined(_WIN32)
    #else
        #define DBG_PRINT(format, ...) ((void)0)
        #define DBG_TRACE(format, ...) ((void)0)
    #endif /* XCP_ENABLE_LOGGING */

#endif /* __XCP_LOG_H */

////////////////////////////////////////////////////////////////////////////////
//                                xcp_macros.h                                //
////////////////////////////////////////////////////////////////////////////////

/*
 * BlueParrot XCP
 *
 * (C) 2007-2022 by Christoph Schueler <github.com/Christoph2,
 *                                      cpu12.gems@googlemail.com>
 *
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * s. FLOSS-EXCEPTION.txt
 */

/** @file xcp_macros.h
 *  @brief Common macros and function like macros.
 *
 *  All macros are prefixed with **XCP_** to prevent collisions
 *  esp. with Windows.h
 *
 *  @author Christoph Schueler (cpu12.gems@googlemail.com)
 */

#if !defined(__XCP_MACROS_H)
    #define __XCP_MACROS_H

    #if XCP_ENABLE_EXTERN_C_GUARDS == XCP_ON
        #if defined(__cplusplus)
extern "C" {

        #endif /* __cplusplus */
    #endif     /* XCP_EXTERN_C_GUARDS */

    #include <assert.h>

    #define XCP_DEBUG_BUILD   (1)
    #define XCP_RELEASE_BUILD (2)

    #if defined(_WIN32) || (XCP_BUILD_TYPE == XCP_DEBUG_BUILD)
        #include <stdio.h>
    #endif /* defined(_WIN32) */

    #if !defined(XCP_LOBYTE)
        #define XCP_LOBYTE(w) ((uint8_t)((w) & (uint8_t)0xff)) /**< Get the low-byte from a word. */
    #endif

    #if !defined(XCP_HIBYTE)
        #define XCP_HIBYTE(w) ((uint8_t)(((w) & (uint16_t)0xff00U) >> 8U)) /**< Get the high-byte from a word. */
    #endif

    #if !defined(XCP_LOWORD)
        #define XCP_LOWORD(w) ((uint16_t)((w) & (uint16_t)0xffff)) /**< Get the low-word from a double-word. */
    #endif

    #if !defined(XCP_HIWORD)
        #define XCP_HIWORD(w) ((uint16_t)(((w) & (uint32_t)0xffff0000) >> 16U)) /**< Get the high-word from a doubleword. */
    #endif

    #if !defined(XCP_MAKEWORD)
        #define XCP_MAKEWORD(h, l)                                                                                                 \
            ((((uint16_t)((h) & ((uint8_t)0xff))) << (uint16_t)8) |                                                                \
             ((uint16_t)((l) & ((uint8_t)0xff)))) /**< Make word from high and low bytes. */
    #endif

    #if !defined(XCP_MAKEDWORD)
        #define XCP_MAKEDWORD(h, l)                                                                                                \
            ((((uint32_t)((h) & ((uint16_t)0xffffu))) << (uint32_t)16) |                                                           \
             ((uint32_t)((l) & ((uint16_t)0xffffu)))) /**< Make double-word from high                                              \
                                                         and low words. */
    #endif

    #if !defined(XCP_MAX)
        #define XCP_MAX(l, r) (((l) > (r)) ? (l) : (r)) /**< Computes the maximum of \a l and \a r. */
    #endif

    #if !defined(XCP_MIN)
        #define XCP_MIN(l, r) (((l) < (r)) ? (l) : (r)) /**< Computes the minimum of \a l and \a r. */
    #endif

    #if !defined(XCP_TRUE)
        #define XCP_TRUE (1) /**< Boolean \a TRUE value. */
    #endif

    #if !defined(XCP_FALSE)
        #define XCP_FALSE (0) /**< Boolean \a FALSE value. */
    #endif

    #if !defined(XCP_NULL)
        #if defined(__cplusplus)
            #define XCP_NULL (0)
        #else
            #define XCP_NULL ((void *)0)
        #endif
    #endif

    #if XCP_BUILD_TYPE == XCP_DEBUG_BUILD
        #define XCP_STATIC            /**< Static only on DEBUG builds. Rationale: (unit-)                                         \
                                                 testing. */
    #else
        #define XCP_STATIC static
    #endif /* XCP_BUILD_TYPE */

    #define XCP_ON_CAN_EXT_IDENTIFIER (0x80000000)

    #if !defined(XCP_UNREFERENCED_PARAMETER)
        #define XCP_UNREFERENCED_PARAMETER(x) (x) = (x) /*lint  -esym( 714, x ) */
    #endif

    #define XCP_FOREVER for (;;)

    #if !defined(INLINE)
        #if defined(__cplusplus)
            /* C++ */
            #define INLINE inline
        #elif (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) || defined(__GNUC__)
            /* C99 or GCC */
            #define INLINE inline
        #elif defined(_MSC_VER)
            /* MSVC */
            #define INLINE __inline
        #else
            /* Fallback for other C89 compilers */
            #define INLINE
        #endif
    #endif /* INLINE */

    #define XCP_ARRAY_SIZE(arr) (sizeof((arr)) / sizeof((arr)[0])) /**< Calculates the number of elements of \a arr */

    #if XCP_ENABLE_EXTERN_C_GUARDS == XCP_ON
        #if defined(__cplusplus)
}
        #endif /* __cplusplus */
    #endif     /* XCP_EXTERN_C_GUARDS */

    #define XCP_ASSERT(x) assert(x)

#endif /* __XCP_MACROS_H */

////////////////////////////////////////////////////////////////////////////////
//                                xcp_types.h                                 //
////////////////////////////////////////////////////////////////////////////////

/*
 * BlueParrot XCP
 *
 * (C) 2007-2022 by Christoph Schueler <github.com/Christoph2,
 *                                      cpu12.gems@googlemail.com>
 *
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * s. FLOSS-EXCEPTION.txt
 */

#if !defined(__XCP_TYPES_H)
    #define __XCP_TYPES_H

    #include <limits.h>

    #if XCP_ENABLE_EXTERN_C_GUARDS == XCP_ON
        #if defined(__cplusplus)
extern "C" {
        #endif /* __cplusplus */
    #endif     /* XCP_EXTERN_C_GUARDS */

    /* check for C99-Compiler */
    #if defined(__STDC_VERSION__)
        #if __STDC_VERSION__ >= 199901L
            #define C99_COMPILER
        #endif
    #endif

    /* check for C1x-Compiler */
    #if defined(__STDC_VERSION__)
        #if __STDC_VERSION__ >= 201112L
            #define C11_COMPILER
        #endif
    #endif

    #if (defined(__CSMC__) || (!defined(C99_COMPILER) && !defined(C11_COMPILER))) && !defined(__cplusplus)

        #if defined(_MSC_VER)

            #include <stdbool.h>
            #include <stdint.h>
        #else
typedef unsigned char bool;
            #if !defined(__int8_t_defined)
typedef signed char   int8_t;
typedef unsigned char uint8_t;
            #endif
            #if !defined(__int16_t_defined)
typedef signed short   int16_t;
typedef unsigned short uint16_t;
            #endif
            #if !defined(__int32_t_defined)
typedef signed long   int32_t;
typedef unsigned long uint32_t;
            #endif
            #if !defined(__int64_t_defined)
typedef signed long long   int64_t;
typedef unsigned long long uint64_t;
            #endif
        #endif

    #else

        #include <stdbool.h>
        #include <stdint.h>
    #endif

    /* 64-bit */
    #if UINTPTR_MAX == 0xffffffffffffffff
        #define ENV64BIT
    typedef uint64_t Xcp_PointerSizeType;
    #elif UINTPTR_MAX == 0xffffffff
typedef uint32_t Xcp_PointerSizeType;
        #define ENV32BIT
    #else
        #define ENV16BIT
typedef uint32_t Xcp_PointerSizeType;
    #endif

    #define UINT8(x) ((uint8_t)(x))
    #define INT8(x)  ((int8_t)(x))

    #define UINT16(x) ((uint16_t)(x))
    #define INT16(x)  ((int16_t)(x))

    #define UINT32(x) ((uint32_t)(x))
    #define INT32(x)  ((int32_t)(x))

    #define UINT64(x) ((uint64_t)(x))
    #define INT64(x)  ((int64_t)(x))

    #if XCP_ENABLE_EXTERN_C_GUARDS == XCP_ON
        #if defined(__cplusplus)
}
        #endif /* __cplusplus */
    #endif     /* XCP_EXTERN_C_GUARDS */

#endif /* __XCP_TYPES_H */

////////////////////////////////////////////////////////////////////////////////
//                                 xcp_eth.h                                  //
////////////////////////////////////////////////////////////////////////////////

/*
 * BlueParrot XCP
 *
 * (C) 2021 by Christoph Schueler <github.com/Christoph2,
 *                                      cpu12.gems@googlemail.com>
 *
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * s. FLOSS-EXCEPTION.txt
 */

#if !defined(__XCP_ETH_H)
    #define __XCP_ETH_H

    #if defined(_WIN32)
        #include <WinSock2.h>
        #include <Ws2tcpip.h>
    #elif defined(__unix__) || defined(__APPLE__)

        #include <arpa/inet.h>
        #include <errno.h>
        #include <fcntl.h>
        #include <netdb.h>
        #include <netinet/in.h>
        #include <signal.h>
        #include <string.h>
        #include <sys/socket.h>
        #include <sys/types.h>
        #include <sys/wait.h>
        #include <unistd.h>
    #endif

    #if defined(_WIN32)
typedef struct tagXcpTl_ConnectionType {
    SOCKADDR_STORAGE connectionAddress;
    SOCKADDR_STORAGE currentAddress;
    SOCKADDR_STORAGE localAddress;
    SOCKET           boundSocket;
    SOCKET           connectedSocket;
    SOCKET           multicastSocket;
    bool             connected;
    int              socketType;
} XcpTl_ConnectionType;
    #elif defined(__unix__) || defined(__APPLE__)
typedef struct tagXcpTl_ConnectionType {
    struct sockaddr_storage connectionAddress;
    struct sockaddr_storage currentAddress;
    struct sockaddr_storage localAddress;
    int                     boundSocket;
    int                     connectedSocket;
    int                     multicastSocket;
    bool                    connected;
    int                     socketType;
} XcpTl_ConnectionType;
    #endif

    #if defined(__unix__) || defined(__APPLE__)
        #define SOCKET_ERROR     (-1)
        #define INVALID_SOCKET   (-1)
        #define ZeroMemory(b, l) memset((b), 0, (l))
    #endif

#endif /* __XCP_ETH_H */

////////////////////////////////////////////////////////////////////////////////
//                                 xcp_task.h                                 //
////////////////////////////////////////////////////////////////////////////////

/*
* BlueParrot XCP
 *
 * (C) 2025 by Christoph Schueler <github.com/Christoph2,
 *                                      cpu12.gems@googlemail.com>
 *
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * s. FLOSS-EXCEPTION.txt
 */


#if !defined(__XCP_ARDUINO_HPP)
#define __XCP_ARDUINO_HPP

#if defined(__cplusplus)

#include <stdint.h>

extern "C" void XcpDaq_TriggerEvent(uint8_t eventChannelNumber);

/*
** Arduinos have a very constrained  C++ library, so we have to use a replacement for <functional>
*/
struct Delegate {
    using Func = void(*)
    (
    );

    Func m_func;

    Delegate (Func func = nullptr
    )
    :
    m_func (func){}

    void operator()() const {
        if (m_func) {
            m_func();
        }
    }

    static Delegate create(void (*function)()) {
        return Delegate(function);
    }
};

struct DelegateU32 {
    using Func = uint32_t(*)();

    Func m_func;

    DelegateU32 (Func func = nullptr
    )
    :
    m_func (func){}

    uint32_t operator()() const {
        if (m_func) {
            return m_func();
        }
        return 0;
    }

    static DelegateU32 create(uint32_t (*function)()) {
        return DelegateU32(function);
    }
};

/**
 * Simple periodic DAQ task management for foreground/background systems like Arduino.
 */
class PeriodicTask {
public:

    PeriodicTask(uint32_t interval, uint16_t event_num, Delegate task, DelegateU32 timestamp_function
    )
    :
    m_interval(interval), m_event_num(event_num), m_task(task), m_timestamp_function(timestamp_function), m_time_point(0UL),
        m_elapsed_time(0UL), m_start(0UL), m_stop(0UL)
    {
        m_time_point = m_timestamp_function();
    }

    void invoke()
    {
        if ((m_timestamp_function() - m_time_point) >= (m_interval * 1000)) {
            m_time_point = m_timestamp_function();
            m_start      = m_timestamp_function();

            m_task();

            XcpDaq_TriggerEvent(m_event_num);

            m_stop         = m_timestamp_function();
            m_elapsed_time = m_stop - m_start;
            m_time_point -= m_elapsed_time;
        }
    }

private:
    uint32_t m_interval;
    uint16_t    m_event_num;
    Delegate    m_task;
    DelegateU32 m_timestamp_function;
    uint32_t    m_time_point;
    uint32_t    m_elapsed_time;
    uint32_t    m_start;
    uint32_t    m_stop;
};



#endif  // __cplusplus

#endif // __XCP_ARDUINO_HPP
////////////////////////////////////////////////////////////////////////////////
//                              xcp_tl_timeout.h                              //
////////////////////////////////////////////////////////////////////////////////

/*
 * BlueParrot XCP
 *
 * (C) 2007-2022 by Christoph Schueler <github.com/Christoph2,
 *                                      cpu12.gems@googlemail.com>
 *
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * s. FLOSS-EXCEPTION.txt
 */

/*
 *
 * Time-out handling functions for Transport-Layer.
 * Currently only applies to character-wise transport layers.
 * Could (should?) be generalized.
 *
 */

#ifndef __XCP_TL_TIMEOUT_H
#define __XCP_TL_TIMEOUT_H

#if XCP_ENABLE_EXTERN_C_GUARDS == XCP_ON
    #if defined(__cplusplus)
extern "C" {
    #endif /* __cplusplus */
#endif     /* XCP_EXTERN_C_GUARDS */


    void XcpTl_TimeoutInit(uint16_t timeout_value, void (*timeout_function)(void));

    void XcpTl_TimeoutStart(void);

    void XcpTl_TimeoutStop(void);

    void XcpTl_TimeoutCheck(void);

    void XcpTl_TimeoutReset(void);

#if XCP_ENABLE_EXTERN_C_GUARDS == XCP_ON
    #if defined(__cplusplus)
}
    #endif /* __cplusplus */
#endif     /* XCP_EXTERN_C_GUARDS */

#endif  // __XCP_TL_TIMEOUT_H

////////////////////////////////////////////////////////////////////////////////
//                                 xcp_util.h                                 //
////////////////////////////////////////////////////////////////////////////////

/*
 * BlueParrot XCP
 *
 * (C) 2007-2019 by Christoph Schueler <github.com/Christoph2,
 *                                      cpu12.gems@googlemail.com>
 *
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * s. FLOSS-EXCEPTION.txt
 */

#if !defined(__XCP_UTIL_H)
    #define __XCP_UTIL_H

    #if XCP_ENABLE_EXTERN_C_GUARDS == XCP_ON
        #if defined(__cplusplus)
extern "C" {
        #endif /* __cplusplus */
    #endif     /* XCP_EXTERN_C_GUARDS */


    void     XcpUtl_MemCopy(void* dst, void const * src, uint32_t len);
    void     XcpUtl_MemSet(void* dest, uint8_t fill_char, uint32_t len);
    bool     XcpUtl_MemCmp(void const * dst, void const * src, uint32_t len);
    uint32_t XcpUtl_StrLen(/*@in@*/ char const * str);
    void     XcpUtl_Hexdump(uint8_t const * buf, uint16_t sz);
    void     XcpUtl_Itoa(uint32_t value, uint8_t base, uint8_t* buf);
    uint8_t  XcpUtl_SetResetBit8(uint8_t result, uint8_t value, uint8_t flag);
    #define XcpUtl_ZeroMem(dest, len) XcpUtl_MemSet((dest), '\0', (len))

    #if XCP_ENABLE_EXTERN_C_GUARDS == XCP_ON
        #if defined(__cplusplus)
}
        #endif /* __cplusplus */
    #endif     /* XCP_EXTERN_C_GUARDS */

#endif /* __XCP_UTIL_H */

////////////////////////////////////////////////////////////////////////////////
//                                   xcp.h                                    //
////////////////////////////////////////////////////////////////////////////////

/*
 * BlueParrot XCP
 *
 * (C) 2007-2025 by Christoph Schueler <github.com/Christoph2,
 *                                      cpu12.gems@googlemail.com>
 *
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * s. FLOSS-EXCEPTION.txt
 */

/*
 * File: xcp.h
 * Purpose:
 *   Central public header for BlueParrot XCP core and transport layers.
 *   This header collects protocol/transport version information, compile-time
 *   feature switches, transport selection, and a large set of constants and
 *   types used by the implementation and by applications.
 *
 */

#if !defined(__CXCP_H)
    #define __CXCP_H

    #define XCP_ON  (1)
    #define XCP_OFF (0)

    #include <assert.h>

    #include "xcp_config.h"

    #define XCP_PROTOCOL_VERSION_MAJOR   (1)
    #define XCP_PROTOCOL_VERSION_RELEASE (0)

    #define XCP_TRANSPORT_LAYER_VERSION_MAJOR   (1)
    #define XCP_TRANSPORT_LAYER_VERSION_RELEASE (1)

    #define XCP_ON_CAN      (1)
    #define XCP_ON_CANFD    (2)
    #define XCP_ON_FLEXRAY  (3)
    #define XCP_ON_USB      (4)
    #define XCP_ON_ETHERNET (5)
    #define XCP_ON_SXI      (6)
    #define XCP_ON_BTH      (128) /* XCP on Bluetooth -- non-standard. */

    #define XCP_DAQ_CONFIG_TYPE_STATIC  (0)
    #define XCP_DAQ_CONFIG_TYPE_DYNAMIC (1)
    #define XCP_DAQ_CONFIG_TYPE_NONE    (3)

    /*
    **  XCPonSXI Header Formats.
    */
    #define XCP_ON_SXI_HEADER_LEN_BYTE      (1)
    #define XCP_ON_SXI_HEADER_LEN_WORD      (2)
    #define XCP_ON_SXI_HEADER_LEN_CTR_BYTE  (3)
    #define XCP_ON_SXI_HEADER_LEN_CTR_WORD  (4)
    #define XCP_ON_SXI_HEADER_LEN_FILL_BYTE (5)
    #define XCP_ON_SXI_HEADER_LEN_FILL_WORD (6)

    /*
    ** XCPonSXI Checksum Variants.
    */
    #define XCP_ON_SXI_NO_CHECKSUM   (0)
    #define XCP_ON_SXI_CHECKSUM_BYTE (1)
    #define XCP_ON_SXI_CHECKSUM_WORD (2)

    /*
    ** XCPonSXI Framing.
    */
    #define XCP_ON_SXI_ESC_SYNC_CHAR (0x01)
    #define XCP_ON_SXI_ESC_ESC_CHAR  (0x00)

    /*
    ** XCP on Ethernet related.
    */
    #define XCP_ON_ETHERNET_DRIVER_ETHERNET (1)
    #define XCP_ON_ETHERNET_DRIVER_WIFI     (2)

    #define XCP_ON_ETHERNET_USE_UDP (0)
    #define XCP_ON_ETHERNET_USE_TCP (1)

    #if !defined(XCP_ON_ETHERNET_PROTOCOL)
        #define XCP_ON_ETHERNET_PROTOCOL XCP_ON_ETHERNET_USE_UDP
    #endif /* * XCP_ON_ETHERNET_PROTOCOL */

    #if XCP_ENABLE_EXTERN_C_GUARDS == XCP_ON
        #if defined(__cplusplus)
extern "C" {

        #endif /* __cplusplus */
    #endif     /* XCP_EXTERN_C_GUARDS */

    #if XCP_ENABLE_DEBUG_OUTPUT == XCP_ON

        #if defined(PICO_RP2040)

    extern int cdc_debug_printf(const char *fmt, ...);
            #define CFG_TUSB_DEBUG_PRINTF cdc_debug_printf
            #include "tusb.h"
        #endif

    #endif

    /*
    ** Configuration checks.
    */

    /* Check for unsupported features. */
    #if XCP_ENABLE_STIM == XCP_ON
        #error STIM not supported yet.
    #endif /* XCP_ENABLE_STIM */

    #if XCP_DAQ_ENABLE_ALTERNATING == XCP_ON
        #error XCP_DAQ_ENABLE_ALTERNATING not supported yet.
    #endif /* XCP_DAQ_ENABLE_ALTERNATING */

    #if XCP_DAQ_ENABLE_PRIORITIZATION == XCP_ON
        #error DAQ priorization not supported yet.
    #endif /* XCP_DAQ_ENABLE_PRIORITIZATION */

    #if XCP_DAQ_ENABLE_BIT_OFFSET == XCP_ON
        #error DAQ doesnt support bit-offsets yet.
    #endif /* XCP_DAQ_ENABLE_BIT_OFFSET */

    #if XCP_DAQ_ENABLE_ADDR_EXT == XCP_ON
        #error "DAQ doesn't support address extension."
    #endif /* XCP_DAQ_ENABLE_ADDR_EXT */

    #if XCP_DAQ_ENABLE_MULTIPLE_DAQ_LISTS_PER_EVENT == XCP_ON
        #error Multiple DAQ lists per event are not supported yet.
    #endif /* XCP_DAQ_ENABLE_MULTIPLE_DAQ_LISTS_PER_EVENT */

    #if XCP_ENABLE_MASTER_BLOCKMODE == XCP_ON
        #if XCP_ENABLE_DOWNLOAD_NEXT == XCP_OFF
            #error Master block-mode requires optional command 'downloadNext'.
        #endif /* XCP_ENABLE_DOWNLOAD_NEXT */
    #endif     /* XCP_ENABLE_MASTER_BLOCKMODE */

    #if XCP_MAX_CTO > 0xff
        #error XCP_MAX_CTO must be <= 255
    #endif

    #define XCP_DAQ_ENABLE_QUEUING (XCP_ON) /* Private setting for now. */

    #if XCP_TRANSPORT_LAYER == XCP_ON_CAN

        #if !defined(XCP_ON_SXI_BITRATE)
            #define XCP_ON_SXI_BITRATE (38400)
        #endif /* XCP_ON_SXI_BITRATE */

        #if !defined(XCP_ON_SXI_CONFIG)
            #define XCP_ON_SXI_CONFIG (SERIAL_8N1)
        #endif /* XCP_ON_SXI_CONFIG */

        #if ((XCP_ENABLE_CAN_GET_SLAVE_ID == XCP_ON) || (XCP_ENABLE_CAN_GET_DAQ_ID == XCP_ON) ||                                   \
             (XCP_ENABLE_CAN_SET_DAQ_ID == XCP_ON))
            #define XCP_ENABLE_TRANSPORT_LAYER_CMD (XCP_ON)
        #endif

        #if (!defined(XCP_ENABLE_CAN_FD)) || (XCP_ENABLE_CAN_FD == XCP_OFF)
            #ifdef XCP_MAX_CTO
                #undef XCP_MAX_CTO
            #endif /* XCP_MAX_CTO */
            #define XCP_MAX_CTO (8)

            #ifdef XCP_MAX_DTO
                #undef XCP_MAX_DTO
            #endif /* XCP_MAX_DTO */
            #define XCP_MAX_DTO (8)
        #else
            #if (XCP_MAX_CTO < 8) || (XCP_MAX_CTO > 64)
                #error MaxCTO must be in range [8 .. 64]
            #endif

            #if (XCP_MAX_DTO < 8) || (XCP_MAX_DTO > 64)
                #error MaxDTO must be in range [8 .. 64]
            #endif

        #endif

        #ifdef XCP_TRANSPORT_LAYER_LENGTH_SIZE
            #undef XCP_TRANSPORT_LAYER_LENGTH_SIZE
        #endif /* XCP_TRANSPORT_LAYER_LENGTH_SIZE */
        #define XCP_TRANSPORT_LAYER_LENGTH_SIZE (0)

        #ifdef XCP_TRANSPORT_LAYER_COUNTER_SIZE
            #undef XCP_TRANSPORT_LAYER_COUNTER_SIZE
        #endif /* XCP_TRANSPORT_LAYER_COUNTER_SIZE */
        #define XCP_TRANSPORT_LAYER_COUNTER_SIZE (0)

        #ifdef XCP_TRANSPORT_LAYER_CHECKSUM_SIZE
            #undef XCP_TRANSPORT_LAYER_CHECKSUM_SIZE
        #endif /* XCP_TRANSPORT_LAYER_CHECKSUM_SIZE */
        #define XCP_TRANSPORT_LAYER_CHECKSUM_SIZE (0)

    #endif /* XCP_TRANSPORT_LAYER */
    #if XCP_TRANSPORT_LAYER == XCP_ON_SXI
        #if !defined(XCP_ON_SXI_HEADER_FORMAT)
            #error "XCP_ON_SXI_HEADER_FORMAT is not defined."
        #endif

        #ifdef XCP_TRANSPORT_LAYER_LENGTH_SIZE
            #undef XCP_TRANSPORT_LAYER_LENGTH_SIZE
        #endif
        #ifdef XCP_TRANSPORT_LAYER_COUNTER_SIZE
            #undef XCP_TRANSPORT_LAYER_COUNTER_SIZE
        #endif

        #if !defined(XCP_ENABLE_LOGGING)
            #define XCP_ENABLE_LOGGING (XCP_OFF)
        #endif /* XCP_ENABLE_LOGGING */

        #if (XCP_ON_SXI_HEADER_FORMAT == XCP_ON_SXI_HEADER_LEN_BYTE)
            #define XCP_TRANSPORT_LAYER_LENGTH_SIZE  (1)
            #define XCP_TRANSPORT_LAYER_COUNTER_SIZE (0)
        #elif (XCP_ON_SXI_HEADER_FORMAT == XCP_ON_SXI_HEADER_LEN_CTR_BYTE)
            #define XCP_TRANSPORT_LAYER_LENGTH_SIZE  (1)
            #define XCP_TRANSPORT_LAYER_COUNTER_SIZE (1)
        #elif (XCP_ON_SXI_HEADER_FORMAT == XCP_ON_SXI_HEADER_LEN_FILL_BYTE)
            #define XCP_TRANSPORT_LAYER_LENGTH_SIZE  (1)
            #define XCP_TRANSPORT_LAYER_COUNTER_SIZE (0)
        #elif (XCP_ON_SXI_HEADER_FORMAT == XCP_ON_SXI_HEADER_LEN_WORD)
            #define XCP_TRANSPORT_LAYER_LENGTH_SIZE  (2)
            #define XCP_TRANSPORT_LAYER_COUNTER_SIZE (0)
        #elif (XCP_ON_SXI_HEADER_FORMAT == XCP_ON_SXI_HEADER_LEN_CTR_WORD)
            #define XCP_TRANSPORT_LAYER_LENGTH_SIZE  (2)
            #define XCP_TRANSPORT_LAYER_COUNTER_SIZE (2)
        #elif (XCP_ON_SXI_HEADER_FORMAT == XCP_ON_SXI_HEADER_LEN_FILL_WORD)
            #define XCP_TRANSPORT_LAYER_LENGTH_SIZE  (2)
            #define XCP_TRANSPORT_LAYER_COUNTER_SIZE (0)
        #else
            #error "Unknown XCP_ON_SXI_HEADER_FORMAT."
        #endif
    #endif /* XCP_TRANSPORT_LAYER == XCP_ON_SXI */

    #if XCP_TRANSPORT_LAYER == XCP_ON_SXI
    /* Already derived per header format above. */
    #elif (XCP_TRANSPORT_LAYER == XCP_ON_ETHERNET) || (XCP_TRANSPORT_LAYER == XCP_ON_BTH)
        #define XCP_TRANSPORT_LAYER_LENGTH_SIZE  (2)
        #define XCP_TRANSPORT_LAYER_COUNTER_SIZE (2)
    #else
        /* For remaining transports (e.g., CAN), length/counter are not used. */
        #define XCP_TRANSPORT_LAYER_LENGTH_SIZE  (0)
        #define XCP_TRANSPORT_LAYER_COUNTER_SIZE (0)
    #endif

    /* Derive transport-layer checksum size centrally: depends on transport.
       For SXI, bind to XCP_ON_SXI_TAIL_CHECKSUM. Others default to 0. */
    #ifdef XCP_TRANSPORT_LAYER_CHECKSUM_SIZE
        #undef XCP_TRANSPORT_LAYER_CHECKSUM_SIZE
    #endif /* XCP_TRANSPORT_LAYER_CHECKSUM_SIZE */

    #if XCP_TRANSPORT_LAYER == XCP_ON_SXI
        #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_NO_CHECKSUM)
            #define XCP_TRANSPORT_LAYER_CHECKSUM_SIZE (0)
        #elif (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_BYTE)
            #define XCP_TRANSPORT_LAYER_CHECKSUM_SIZE (1)
        #elif (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_WORD)
            #define XCP_TRANSPORT_LAYER_CHECKSUM_SIZE (2)
        #else
            #error "Unknown XCP_ON_SXI_TAIL_CHECKSUM value."
        #endif

        #if !defined(XCP_ON_SXI_CUSTOM_INTERFACE)
            #define XCP_ON_SXI_CUSTOM_INTERFACE (XCP_OFF)
        #endif

        #if !defined(XCP_ON_SXI_PORT_NAME)
            #define XCP_ON_SXI_PORT_NAME Serial
        #endif
    #elif (XCP_TRANSPORT_LAYER == XCP_ON_ETHERNET) || (XCP_TRANSPORT_LAYER == XCP_ON_BTH)
        #define XCP_TRANSPORT_LAYER_CHECKSUM_SIZE (0)
    #else
        /* For remaining transports (e.g., CAN), checksum size is not used */
        #define XCP_TRANSPORT_LAYER_CHECKSUM_SIZE (0)
    #endif

    #if (XCP_ENABLE_GET_SEED == XCP_ON) && (XCP_ENABLE_UNLOCK == XCP_ON)
        #define XCP_ENABLE_RESOURCE_PROTECTION (XCP_ON)
    #elif (XCP_ENABLE_GET_SEED == XCP_OFF) && (XCP_ENABLE_UNLOCK == XCP_OFF)
        #define XCP_ENABLE_RESOURCE_PROTECTION (XCP_OFF)
    #elif (XCP_ENABLE_GET_SEED == XCP_ON) && (XCP_ENABLE_UNLOCK == XCP_OFF)
        #error GET_SEED requires UNLOCK
    #elif (XCP_ENABLE_GET_SEED == XCP_OFF) && (XCP_ENABLE_UNLOCK == XCP_ON)
        #error UNLOCK requires GET_SEED
    #endif

    #if XCP_DAQ_CONFIG_TYPE == XCP_DAQ_CONFIG_TYPE_NONE
        #define XCP_DAQ_ENABLE_DYNAMIC_LISTS XCP_OFF
        #define XCP_DAQ_ENABLE_STATIC_LISTS  XCP_OFF
    #elif XCP_DAQ_CONFIG_TYPE == XCP_DAQ_CONFIG_TYPE_DYNAMIC
        #define XCP_DAQ_ENABLE_DYNAMIC_LISTS XCP_ON
        #define XCP_DAQ_ENABLE_STATIC_LISTS  XCP_OFF
    #elif XCP_DAQ_CONFIG_TYPE == XCP_DAQ_CONFIG_TYPE_STATIC
        #define XCP_DAQ_ENABLE_DYNAMIC_LISTS XCP_OFF
        #define XCP_DAQ_ENABLE_STATIC_LISTS  XCP_ON
    #endif /* XCP_DAQ_CONFIG_TYPE */

    #if XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_ON
        #define XCP_MIN_DAQ XcpDaq_PredefinedListCount
    #else
        #define XCP_MIN_DAQ ((XcpDaq_ListIntegerType)0)
    #endif /* XCP_DAQ_ENABLE_PREDEFINED_LISTS */

    #if (XCP_ENABLE_DAQ_COMMANDS == XCP_ON) && (XCP_DAQ_CONFIG_TYPE == XCP_DAQ_CONFIG_TYPE_NONE) &&                                \
        (XCP_DAQ_ENABLE_PREDEFINED_LISTS == STD_OFF)
        #error Neither predefined nor configurable lists are enabled.
    #endif

    #if (XCP_ENABLE_DAQ_COMMANDS == XCP_ON) && (XCP_DAQ_MAX_EVENT_CHANNEL < 1)
        #error XCP_DAQ_MAX_EVENT_CHANNEL must be at least 1
    #endif /* XCP_DAQ_MAX_EVENT_CHANNEL */

    #if (XCP_ENABLE_DAQ_COMMANDS == XCP_ON) && (XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_OFF)
        #if XCP_ENABLE_FREE_DAQ == XCP_ON
            #undef XCP_ENABLE_FREE_DAQ
            #define XCP_ENABLE_FREE_DAQ XCP_OFF
        #endif /* XCP_ENABLE_FREE_DAQ */

        #if XCP_ENABLE_ALLOC_DAQ == XCP_ON
            #undef XCP_ENABLE_ALLOC_DAQ
            #define XCP_ENABLE_ALLOC_DAQ XCP_OFF
        #endif /* XCP_ENABLE_ALLOC_DAQ */

        #if XCP_ENABLE_ALLOC_ODT == XCP_ON
            #undef XCP_ENABLE_ALLOC_ODT
            #define XCP_ENABLE_ALLOC_ODT XCP_OFF
        #endif /* XCP_ENABLE_ALLOC_ODT */

        #if XCP_ENABLE_ALLOC_ODT_ENTRY == XCP_ON
            #undef XCP_ENABLE_ALLOC_ODT_ENTRY
            #define XCP_ENABLE_ALLOC_ODT_ENTRY XCP_OFF
        #endif /* XCP_ENABLE_ALLOC_ODT_ENTRY */
    #endif

    #if (XCP_ENABLE_WRITE_DAQ_MULTIPLE == XCP_ON) && (XCP_MAX_CTO < 10)
        #error XCP_ENABLE_WRITE_DAQ_MULTIPLE requires XCP_MAX_CTO of at least 10
    #endif

    #define XCP_DAQ_ODT_ENTRY_OFFSET   ((1)) /* Currently fixed (only abs. ODT numbers supported). */
    #define XCP_DAQ_MAX_ODT_ENTRY_SIZE (XCP_MAX_DTO - XCP_DAQ_ODT_ENTRY_OFFSET) /* Max. payload. */

    #if XCP_TRANSPORT_LAYER == XCP_ON_CAN

    #else
        #if XCP_ON_CAN_MAX_DLC_REQUIRED == XCP_ON
            #error XCP_ON_CAN_MAX_DLC_REQUIRED only applies to XCP_ON_CAN
        #endif /* XCP_ON_CAN_MAX_DLC_REQUIRED */
    #endif     /* XCP_TRANSPORT_LAYER == XCP_ON_CAN */

    #if (XCP_TRANSPORT_LAYER_COUNTER_SIZE != 0) && (XCP_TRANSPORT_LAYER_COUNTER_SIZE != 1) &&                                      \
        (XCP_TRANSPORT_LAYER_COUNTER_SIZE != 2)
        #error XCP_TRANSPORT_LAYER_COUNTER_SIZE must be 0, 1, or 2
    #endif

    #if (XCP_TRANSPORT_LAYER_LENGTH_SIZE != 0) && (XCP_TRANSPORT_LAYER_LENGTH_SIZE != 1) && (XCP_TRANSPORT_LAYER_LENGTH_SIZE != 2)
        #error XCP_TRANSPORT_LAYER_LENGTH_SIZE  must be 0, 1, or 2
    #endif

    #define XCP_TRANSPORT_LAYER_BUFFER_OFFSET   (XCP_TRANSPORT_LAYER_COUNTER_SIZE + XCP_TRANSPORT_LAYER_LENGTH_SIZE)
    #define XCP_TRANSPORT_LAYER_CTO_BUFFER_SIZE (XCP_MAX_CTO + XCP_TRANSPORT_LAYER_BUFFER_OFFSET)
    #define XCP_TRANSPORT_LAYER_DTO_BUFFER_SIZE (XCP_MAX_DTO + XCP_TRANSPORT_LAYER_BUFFER_OFFSET)

    #if !defined(XCP_DAQ_ENABLE_WRITE_THROUGH)
        #define XCP_DAQ_ENABLE_WRITE_THROUGH XCP_OFF
    #endif

    #if !defined(XCP_DAQ_ENABLE_RESET_DYN_DAQ_CONFIG_ON_SEQUENCE_ERROR)
        #define XCP_DAQ_ENABLE_RESET_DYN_DAQ_CONFIG_ON_SEQUENCE_ERROR XCP_OFF
    #endif

    #if XCP_DAQ_MAX_DYNAMIC_ENTITIES < 256
        #define XCP_DAQ_ENTITY_TYPE uint8_t
    #elif XCP_DAQ_MAX_DYNAMIC_ENTITIES < 65536
        #define XCP_DAQ_ENTITY_TYPE uint16_t
    #else
        #define XCP_DAQ_ENTITY_TYPE uint32_t
    #endif

    #define XCP_DAQ_LISTS_STOP_ALL       UINT8(0x00)
    #define XCP_DAQ_LISTS_START_SELECTED UINT8(0x01)
    #define XCP_DAQ_LISTS_STOP_SELECTED  UINT8(0x02)

    #define XCP_SET_ID(name) { UINT16(sizeof((name)) - UINT16(1)), (uint8_t const *)(name) }

    #if !defined(XCP_MAX_BS)
        #define XCP_MAX_BS (0)
    #endif /* XCP_MAX_BS */

    #if !defined(XCP_MIN_ST)
        #define XCP_MIN_ST (0)
    #endif /* XCP_MIN_ST */

    #if !defined(XCP_MAX_BS_PGM)
        #define XCP_MAX_BS_PGM (0)
    #endif /* XCP_MAX_BS_PGM */

    #if !defined(XCP_MIN_ST_PGM)
        #define XCP_MIN_ST_PGM (0)
    #endif /* XCP_MIN_ST_PGM */

    #define XCP_DOWNLOAD_PAYLOAD_LENGTH ((XCP_MAX_CTO) - 2)

    /*
     * Packet Identifiers.
     */
    #define XCP_PACKET_IDENTIFIER_RES  UINT8(0xFF)
    #define XCP_PACKET_IDENTIFIER_ERR  UINT8(0xFE)
    #define XCP_PACKET_IDENTIFIER_EV   UINT8(0xFD)
    #define XCP_PACKET_IDENTIFIER_SERV UINT8(0xFC)

    /*
     *  Available Resources.
     */
    #define XCP_RESOURCE_PGM     UINT8(16)
    #define XCP_RESOURCE_STIM    UINT8(8)
    #define XCP_RESOURCE_DAQ     UINT8(4)
    #define XCP_RESOURCE_CAL_PAG UINT8(1)

    /*
     * Comm Mode Basic.
     */
    #define XCP_BYTE_ORDER_INTEL    (0)
    #define XCP_BYTE_ORDER_MOTOROLA (1)

    #define XCP_SLAVE_BLOCK_MODE UINT8(0x40)

    #define XCP_ADDRESS_GRANULARITY_0 UINT8(2)
    #define XCP_ADDRESS_GRANULARITY_1 UINT8(4)

    #define XCP_ADDRESS_GRANULARITY_BYTE  UINT8(0)
    #define XCP_ADDRESS_GRANULARITY_WORD  (XCP_ADDRESS_GRANULARITY_0)
    #define XCP_ADDRESS_GRANULARITY_DWORD (XCP_ADDRESS_GRANULARITY_1)

    #define XCP_OPTIONAL_COMM_MODE UINT8(0x80)

    #define XCP_MASTER_BLOCK_MODE UINT8(1)
    #define XCP_INTERLEAVED_MODE  UINT8(2)

    /*
     * GetID Mode.
     */
    #define XCP_COMPRESSED_ENCRYPTED UINT8(2)
    #define XCP_TRANSFER_MODE        UINT8(1)

    /*
     * Current Session Status.
     */
    #define RESUME        UINT8(0x80)
    #define DAQ_RUNNING   UINT8(0x40)
    #define CLEAR_DAQ_REQ UINT8(0x08)
    #define STORE_DAQ_REQ UINT8(0x04)
    #define STORE_CAL_REQ UINT8(0x01)

    /*
     * SetRequest Mode.
     */
    #define XCP_CLEAR_DAQ_REQ           UINT8(8)
    #define XCP_STORE_DAQ_REQ_RESUME    UINT8(4)
    #define XCP_STORE_DAQ_REQ_NO_RESUME UINT8(2)
    #define XCP_STORE_CAL_REQ           UINT8(1)

    /*
     * SetCalPage Mode.
     */
    #define XCP_SET_CAL_PAGE_ALL UINT8(0x80)
    #define XCP_SET_CAL_PAGE_XCP UINT8(0x02)
    #define XCP_SET_CAL_PAGE_ECU UINT8(0x01)

    /* DAQ List Modes. */
    #define XCP_DAQ_LIST_MODE_ALTERNATING UINT8(0x01)
    #define XCP_DAQ_LIST_MODE_DIRECTION   UINT8(0x02)
    #define XCP_DAQ_LIST_MODE_TIMESTAMP   UINT8(0x10)
    #define XCP_DAQ_LIST_MODE_PID_OFF     UINT8(0x20)
    #define XCP_DAQ_LIST_MODE_SELECTED    UINT8(0x40)
    #define XCP_DAQ_LIST_MODE_STARTED     UINT8(0x80)

    /* DAQ Properties */
    #define XCP_DAQ_PROP_OVERLOAD_EVENT      UINT8(0x80)
    #define XCP_DAQ_PROP_OVERLOAD_MSB        UINT8(0x40)
    #define XCP_DAQ_PROP_PID_OFF_SUPPORTED   UINT8(0x20)
    #define XCP_DAQ_PROP_TIMESTAMP_SUPPORTED UINT8(0x10)
    #define XCP_DAQ_PROP_BIT_STIM_SUPPORTED  UINT8(0x08)
    #define XCP_DAQ_PROP_RESUME_SUPPORTED    UINT8(0x04)
    #define XCP_DAQ_PROP_PRESCALER_SUPPORTED UINT8(0x02)
    #define XCP_DAQ_PROP_DAQ_CONFIG_TYPE     UINT8(0x01)

    /* DAQ Key Byte */
    #define XCP_DAQ_KEY_IDENTIFICATION_FIELD_TYPE_1 UINT8(0x80)
    #define XCP_DAQ_KEY_IDENTIFICATION_FIELD_TYPE_0 UINT8(0x40)
    #define XCP_DAQ_KEY_ADDRESS_EXTENSION_DAQ       UINT8(0x20)
    #define XCP_DAQ_KEY_ADDRESS_EXTENSION_ODT       UINT8(0x10)
    #define XCP_DAQ_KEY_OPTIMISATION_TYPE_3         UINT8(0x08)
    #define XCP_DAQ_KEY_OPTIMISATION_TYPE_2         UINT8(0x04)
    #define XCP_DAQ_KEY_OPTIMISATION_TYPE_1         UINT8(0x02)
    #define XCP_DAQ_KEY_OPTIMISATION_TYPE_0         UINT8(0x01)

    /* DAQ Event Channel Properties */
    #define XCP_DAQ_EVENT_CHANNEL_TYPE_DAQ  UINT8(0x04)
    #define XCP_DAQ_EVENT_CHANNEL_TYPE_STIM UINT8(0x08)

    /* DAQ Consistency */
    #define XCP_DAQ_CONSISTENCY_DAQ_LIST      UINT8(0x40)
    #define XCP_DAQ_CONSISTENCY_EVENT_CHANNEL UINT8(0x80)

    /* DAQ Time Units */
    #define XCP_DAQ_EVENT_CHANNEL_TIME_UNIT_1NS   UINT8(0)
    #define XCP_DAQ_EVENT_CHANNEL_TIME_UNIT_10NS  UINT8(1)
    #define XCP_DAQ_EVENT_CHANNEL_TIME_UNIT_100NS UINT8(2)
    #define XCP_DAQ_EVENT_CHANNEL_TIME_UNIT_1US   UINT8(3)
    #define XCP_DAQ_EVENT_CHANNEL_TIME_UNIT_10US  UINT8(4)
    #define XCP_DAQ_EVENT_CHANNEL_TIME_UNIT_100US UINT8(5)
    #define XCP_DAQ_EVENT_CHANNEL_TIME_UNIT_1MS   UINT8(6)
    #define XCP_DAQ_EVENT_CHANNEL_TIME_UNIT_10MS  UINT8(7)
    #define XCP_DAQ_EVENT_CHANNEL_TIME_UNIT_100MS UINT8(8)
    #define XCP_DAQ_EVENT_CHANNEL_TIME_UNIT_1S    UINT8(9)
    #define XCP_DAQ_EVENT_CHANNEL_TIME_UNIT_1PS   UINT8(10)
    #define XCP_DAQ_EVENT_CHANNEL_TIME_UNIT_10PS  UINT8(11)
    #define XCP_DAQ_EVENT_CHANNEL_TIME_UNIT_100PS UINT8(12)

    /* DAQ list properties */
    #define DAQ_LIST_PROPERTY_STIM        UINT8(8)
    #define DAQ_LIST_PROPERTY_DAQ         UINT8(4)
    #define DAQ_LIST_PROPERTY_EVENT_FIXED UINT8(2)
    #define DAQ_LIST_PROPERTY_PREDEFINED  UINT8(1)

    /*
     * DAQ List Mode
     */
    #define DAQ_CURRENT_LIST_MODE_RESUME    UINT8(0x80)
    #define DAQ_CURRENT_LIST_MODE_RUNNING   UINT8(0x40)
    #define DAQ_CURRENT_LIST_MODE_PID_OFF   UINT8(0x20)
    #define DAQ_CURRENT_LIST_MODE_TIMESTAMP UINT8(0x10)
    #define DAQ_CURRENT_LIST_MODE_DIRECTION UINT8(0x02)
    #define DAQ_CURRENT_LIST_MODE_SELECTED  UINT8(0x01)

    /*
     * XCP Event Codes
     */
    #define EV_RESUME_MODE        UINT8(0x00)
    #define EV_CLEAR_DAQ          UINT8(0x01)
    #define EV_STORE_DAQ          UINT8(0x02)
    #define EV_STORE_CAL          UINT8(0x03)
    #define EV_CMD_PENDING        UINT8(0x05)
    #define EV_DAQ_OVERLOAD       UINT8(0x06)
    #define EV_SESSION_TERMINATED UINT8(0x07)
    #define EV_TIME_SYNC          UINT8(0x08)
    #define EV_STIM_TIMEOUT       UINT8(0x09)
    #define EV_SLEEP              UINT8(0x0A)
    #define EV_WAKE_UP            UINT8(0x0B)
    #define EV_USER               UINT8(0xFE)
    #define EV_TRANSPORT          UINT8(0xFF)

    /* Function-like Macros for Events. */
    #define XcpEvent_ResumeMode()         Xcp_SendEventPacket(EV_RESUME_MODE, XCP_NULL, UINT8(0))
    #define XcpEvent_ClearDaq()           Xcp_SendEventPacket(EV_CLEAR_DAQ, XCP_NULL, UINT8(0))
    #define XcpEvent_StoreDaq()           Xcp_SendEventPacket(EV_STORE_DAQ, XCP_NULL, UINT8(0))
    #define XcpEvent_StoreCal()           Xcp_SendEventPacket(EV_STORE_CAL, XCP_NULL, UINT8(0))
    #define XcpEvent_CmdPending()         Xcp_SendEventPacket(EV_CMD_PENDING, XCP_NULL, UINT8(0))
    #define XcpEvent_DaqOverload()        Xcp_SendEventPacket(EV_DAQ_OVERLOAD, XCP_NULL, UINT8(0))
    #define XcpEvent_SessionTerminated()  Xcp_SendEventPacket(EV_SESSION_TERMINATED, XCP_NULL, UINT8(0))
    #define XcpEvent_TimeSync()           Xcp_SendEventPacket(EV_TIME_SYNC, XCP_NULL, UINT8(0))
    #define XcpEvent_StimTimeout()        Xcp_SendEventPacket(EV_STIM_TIMEOUT, XCP_NULL, UINT8(0))
    #define XcpEvent_Sleep()              Xcp_SendEventPacket(EV_SLEEP, XCP_NULL, UINT8(0))
    #define XcpEvent_WakeUp()             Xcp_SendEventPacket(EV_WAKE_UP, XCP_NULL, UINT8(0))
    #define XcpEvent_User(data, len)      Xcp_SendEventPacket(EV_USER, (data), (len))
    #define XcpEvent_Transport(data, len) Xcp_SendEventPacket(EV_TRANSPORT, (data), (len))

    /*
     * XCP Service Request Codes
     */
    #define SERV_RESET UINT8(0x00)
    #define SERV_TEXT  UINT8(0x01)

    /* Function-like Macros for Service Requests. */
    #define XcpService_Reset()            Xcp_SendServiceRequestPacket(SERV_RESET, XCP_NULL, UINT8(0))
    #define XcpService_Text(txt, txt_len) Xcp_SendServiceRequestPacket(SERV_TEXT, (txt), (txt_len))

    #define XCP_DAQ_PREDEFINDED_LIST_COUNT (sizeof(XcpDaq_PredefinedLists) / sizeof(XcpDaq_PredefinedLists[0]))

    /* DAQ Implementation Macros */
    // #define XCP_DAQ_DEFINE_ODT_ENTRY(meas) { { /*(uint32_t)*/ &(meas) }, sizeof((meas)) }
    #define XCP_DAQ_PREDEFINED_LISTS_BEGIN const XcpDaq_ListConfigurationType XcpDaq_PredefinedLists[] = {
    #define XCP_DAQ_PREDEFINED_LISTS_END                                                                                           \
        }                                                                                                                          \
        ;                                                                                                                          \
        XcpDaq_ListStateType         XcpDaq_PredefinedListsState[XCP_DAQ_PREDEFINDED_LIST_COUNT];                                  \
        const XcpDaq_ListIntegerType XcpDaq_PredefinedListCount = XCP_DAQ_PREDEFINDED_LIST_COUNT;

    #define XCP_DAQ_PREDEFINED_ODTS_BEGIN const XcpDaq_ODTType XcpDaq_PredefinedOdts[] = {
    #define XCP_DAQ_PREDEFINED_ODTS_END                                                                                            \
        }                                                                                                                          \
        ;

    #define XCP_DAQ_PREDEFINED_ODT_ENTRIES_BEGIN const XcpDaq_ODTEntryType XcpDaq_PredefinedOdtEntries[] = {
    #define XCP_DAQ_PREDEFINED_ODT_ENTRIES_END                                                                                     \
        }                                                                                                                          \
        ;

    #define XCP_DAQ_PREDEFINED_MEASUREMENT_VARIABLES_BEGIN XcpDaq_ODTEntryType XcpDaq_PredefinedMeasurementVariables[] = {
    #define XCP_DAQ_PREDEFINED_MEASUREMENT_VARIABLES_END                                                                           \
        }                                                                                                                          \
        ;

    #define XCP_DAQ_DEFINE_ODT_VARIABLE_IDX(idx) { { (idx) }, 0 }

    #define XCP_DAQ_DEFINE_MEASUREMENT_VARIABLE(meas) { { (Xcp_PointerSizeType)(Xcp_PointerSizeType *)&(meas) }, sizeof((meas)) }

    /* DAQ Event Implementation Macros */
    #define XCP_DAQ_BEGIN_EVENTS const XcpDaq_EventType XcpDaq_Events[XCP_DAQ_MAX_EVENT_CHANNEL] = {
    #define XCP_DAQ_END_EVENTS                                                                                                     \
        }                                                                                                                          \
        ;
    #define XCP_DAQ_DEFINE_EVENT(name, props, timebase, cycle)                                                                     \
        {                                                                                                                          \
            (uint8_t const * const)(name), sizeof((name)) - 1, (props), (timebase), (cycle),                                       \
        }

    #define XCP_DAQ_BEGIN_ID_LIST const uint32_t Xcp_DaqIDs[] = {
    #define XCP_DAQ_END_ID_LIST                                                                                                    \
        }                                                                                                                          \
        ;                                                                                                                          \
        const uint16_t Xcp_DaqIDCount = XCP_ARRAY_SIZE(Xcp_DaqIDs);

    /*
     * PAG Processor Properties.
     */
    #define XCP_PAG_PROCESSOR_FREEZE_SUPPORTED UINT8(1)

    /*
     * Page Properties.
     */
    #define XCP_WRITE_ACCESS_WITH_ECU    UINT8(32)
    #define XCP_WRITE_ACCESS_WITHOUT_ECU UINT8(16)
    #define XCP_READ_ACCESS_WITH_ECU     UINT8(8)
    #define XCP_READ_ACCESS_WITHOUT_ECU  UINT8(4)
    #define ECU_ACCESS_WITH_XCP          UINT8(2)
    #define ECU_ACCESS_WITHOUT_XCP       UINT8(1)

    /*
    **  PGM Capabilities.
    */
    #define XCP_PGM_NON_SEQ_PGM_REQUIRED  UINT8(0x80)
    #define XCP_PGM_NON_SEQ_PGM_SUPPORTED UINT8(0x40)
    #define XCP_PGM_ENCRYPTION_REQUIRED   UINT8(0x20)
    #define XCP_PGM_ENCRYPTION_SUPPORTED  UINT8(0x10)
    #define XCP_PGM_COMPRESSION_REQUIRED  UINT8(8)
    #define XCP_PGM_COMPRESSION_SUPPORTED UINT8(4)
    #define XCP_PGM_FUNCTIONAL_MODE       UINT8(2)
    #define XCP_PGM_ABSOLUTE_MODE         UINT8(1)

    /*
    **  XCPonCAN specific function-like macros.
    */
    #define XCP_ON_CAN_IS_EXTENDED_IDENTIFIER(i) (((i) & XCP_ON_CAN_EXT_IDENTIFIER) == XCP_ON_CAN_EXT_IDENTIFIER)
    #define XCP_ON_CAN_STRIP_IDENTIFIER(i)       ((i) & (~XCP_ON_CAN_EXT_IDENTIFIER))

    /*
    ** Synchronization macros.
    */
    #define XCP_HW_LOCK_XCP UINT8(0)
    #define XCP_HW_LOCK_TL  UINT8(1)
    #define XCP_HW_LOCK_DAQ UINT8(2)
    #define XCP_HW_LOCK_HW  UINT8(3)

    #define XCP_HW_LOCK_COUNT UINT8(4)

    #define XCP_ENTER_CRITICAL()     XcpHw_AcquireLock(XCP_HW_LOCK_XCP)
    #define XCP_LEAVE_CRITICAL()     XcpHw_ReleaseLock(XCP_HW_LOCK_XCP)
    #define XCP_HW_ENTER_CRITICAL()  XcpHw_AcquireLock(XCP_HW_LOCK_HW)
    #define XCP_HW_LEAVE_CRITICAL()  XcpHw_ReleaseLock(XCP_HW_LOCK_HW)
    #define XCP_TL_ENTER_CRITICAL()  XcpHw_AcquireLock(XCP_HW_LOCK_TL)
    #define XCP_TL_LEAVE_CRITICAL()  XcpHw_ReleaseLock(XCP_HW_LOCK_TL)
    #define XCP_DAQ_ENTER_CRITICAL() XcpHw_AcquireLock(XCP_HW_LOCK_DAQ)
    #define XCP_DAQ_LEAVE_CRITICAL() XcpHw_ReleaseLock(XCP_HW_LOCK_DAQ)
    #define XCP_STIM_ENTER_CRITICAL()
    #define XCP_STIM_LEAVE_CRITICAL()
    #define XCP_PGM_ENTER_CRITICAL()
    #define XCP_PGM_LEAVE_CRITICAL()
    #define XCP_CAL_ENTER_CRITICAL()
    #define XCP_CAL_LEAVE_CRITICAL()
    #define XCP_PAG_ENTER_CRITICAL()
    #define XCP_PAG_LEAVE_CRITICAL()

    /*
     * Interface defaults.
     */
    #define XCP_ETH_DEFAULT_PORT      (5555)
    #define XCP_SOCKET_CAN_DEFAULT_IF ("vcan0")

    #define XCP_ETH_HEADER_SIZE (4)

    /*
     * CAN Interfaces.
     */
    #define XCP_CAN_IF_SEED_STUDIO_CAN_SHIELD    (0x01)
    #define XCP_CAN_IF_SEED_STUDIO_CAN_FD_SHIELD (0x02)
    #define XCP_CAN_IF_MKR_ZERO_CAN_SHIELD       (0x03)
    #define XCP_CAN_IF_SPARKFUN_CAN_SHIELD       (0x04)

    // Set defaults for MCP25XX CAN controllers.
    #if !defined(XCP_CAN_IF_MCP25XX_PIN_CS)
        #define XCP_CAN_IF_MCP25XX_PIN_CS UINT8(9)
    #endif

    #if !defined(XCP_CAN_IF_MCP25XX_PIN_INT)
        #define XCP_CAN_IF_MCP25XX_PIN_INT UINT8(2)
    #endif

    /*
    ** Bounds-checking macros.
    */

    /**! Comparisions   */
    #define XCP_ASSERT_EQ(lhs, rhs) assert((lhs) == (rhs))
    #define XCP_ASSERT_NE(lhs, rhs) assert((lhs) != (rhs))
    #define XCP_ASSERT_LE(lhs, rhs) assert((lhs) <= (rhs))
    #define XCP_ASSERT_LT(lhs, rhs) assert((lhs) < (rhs))
    #define XCP_ASSERT_BE(lhs, rhs) assert((lhs) >= (rhs))
    #define XCP_ASSERT_BG(lhs, rhs) assert((lhs) > (rhs))

    /**! Check if address `a` is in range [`b` .. `b` + `l`] */
    #define XCP_CHECK_ADDRESS_IN_RANGE(b, l, a) assert((((a) >= (b))) && ((a) < ((b) + (l))))

    /**! Check if address range [`a` -- `a` + `l1`] is contained in range [`b` ..
     * `b` + `l0`] */
    #define XCP_BOUNDS_CHECK(b, l0, a, l1) assert((((a) >= (b))) && ((a) < ((b) + (l0))) && (((a) + (l1)) <= ((b) + (l0))))

    /*
    ** Global Types.
    */

    #if XCP_ENABLE_DAQ_COMMANDS == XCP_ON
    typedef XCP_DAQ_ENTITY_TYPE XcpDaq_ListIntegerType;
    typedef XCP_DAQ_ENTITY_TYPE XcpDaq_ODTIntegerType;
    typedef XCP_DAQ_ENTITY_TYPE XcpDaq_ODTEntryIntegerType;
    #endif /* XCP_ENABLE_DAQ_COMMANDS */

    typedef enum tagXcp_CommandType {
        /*
        ** STD
        */
        /*
        ** Mandatory Commands.
        */
        XCP_CONNECT    = UINT8(0xFF),
        XCP_DISCONNECT = UINT8(0xFE),
        XCP_GET_STATUS = UINT8(0xFD),
        XCP_SYNCH      = UINT8(0xFC),
        /*
        ** Optional Commands.
        */
        XCP_GET_COMM_MODE_INFO = UINT8(0xFB),
        XCP_GET_ID             = UINT8(0xFA),
        XCP_SET_REQUEST        = UINT8(0xF9),
        XCP_GET_SEED           = UINT8(0xF8),
        XCP_UNLOCK             = UINT8(0xF7),
        XCP_SET_MTA            = UINT8(0xF6),
        XCP_UPLOAD             = UINT8(0xF5),
        XCP_SHORT_UPLOAD       = UINT8(0xF4),
        XCP_BUILD_CHECKSUM     = UINT8(0xF3),

        XCP_TRANSPORT_LAYER_CMD = UINT8(0xF2),
        XCP_USER_CMD            = UINT8(0xF1),
        /*
        ** CAL
        */
        /*
        ** Mandatory Commands.
        */
        XCP_DOWNLOAD = UINT8(0xF0),
        /*
        ** Optional Commands.
        */
        XCP_DOWNLOAD_NEXT  = UINT8(0xEF),
        XCP_DOWNLOAD_MAX   = UINT8(0xEE),
        XCP_SHORT_DOWNLOAD = UINT8(0xED),
        XCP_MODIFY_BITS    = UINT8(0xEC),
        /*
        ** PAG
        */
        /*
        ** Mandatory Commands.
        */
        XCP_SET_CAL_PAGE = UINT8(0xEB),
        XCP_GET_CAL_PAGE = UINT8(0xEA),
        /*
        ** Optional Commands.
        */
        XCP_GET_PAG_PROCESSOR_INFO = UINT8(0xE9),
        XCP_GET_SEGMENT_INFO       = UINT8(0xE8),
        XCP_GET_PAGE_INFO          = UINT8(0xE7),
        XCP_SET_SEGMENT_MODE       = UINT8(0xE6),
        XCP_GET_SEGMENT_MODE       = UINT8(0xE5),
        XCP_COPY_CAL_PAGE          = UINT8(0xE4),
        /*
        ** DAQ
        */
        /*
        ** Mandatory Commands.
        */
        XCP_CLEAR_DAQ_LIST      = UINT8(0xE3),
        XCP_SET_DAQ_PTR         = UINT8(0xE2),
        XCP_WRITE_DAQ           = UINT8(0xE1),
        WRITE_DAQ_MULTIPLE      = UINT8(0xC7), /* NEW IN 1.1 */
        XCP_SET_DAQ_LIST_MODE   = UINT8(0xE0),
        XCP_GET_DAQ_LIST_MODE   = UINT8(0xDF),
        XCP_START_STOP_DAQ_LIST = UINT8(0xDE),
        XCP_START_STOP_SYNCH    = UINT8(0xDD),
        /*
        ** Optional Commands.
        */
        XCP_GET_DAQ_CLOCK           = UINT8(0xDC),
        XCP_READ_DAQ                = UINT8(0xDB),
        XCP_GET_DAQ_PROCESSOR_INFO  = UINT8(0xDA),
        XCP_GET_DAQ_RESOLUTION_INFO = UINT8(0xD9),
        XCP_GET_DAQ_LIST_INFO       = UINT8(0xD8),
        XCP_GET_DAQ_EVENT_INFO      = UINT8(0xD7),
        XCP_FREE_DAQ                = UINT8(0xD6),
        XCP_ALLOC_DAQ               = UINT8(0xD5),
        XCP_ALLOC_ODT               = UINT8(0xD4),
        XCP_ALLOC_ODT_ENTRY         = UINT8(0xD3),
        /*
        ** PGM
        */
        /*
        ** Mandatory Commands.
        */
        XCP_PROGRAM_START = UINT8(0xD2),
        XCP_PROGRAM_CLEAR = UINT8(0xD1),
        XCP_PROGRAM       = UINT8(0xD0),
        XCP_PROGRAM_RESET = UINT8(0xCF),
        /*
        ** Optional Commands.
        */
        XCP_GET_PGM_PROCESSOR_INFO = UINT8(0xCE),
        XCP_GET_SECTOR_INFO        = UINT8(0xCD),
        XCP_PROGRAM_PREPARE        = UINT8(0xCC),
        XCP_PROGRAM_FORMAT         = UINT8(0xCB),
        XCP_PROGRAM_NEXT           = UINT8(0xCA),
        XCP_PROGRAM_MAX            = UINT8(0xC9),
        XCP_PROGRAM_VERIFY         = UINT8(0xC8),
        /* NEW IN 1.1 */
        XCP_TIME_CORRELATION_PROPERTIES = UINT8(0xC6)
    } Xcp_CommandType;

    typedef enum tagXcp_ReturnType {
        ERR_CMD_SYNCH = UINT8(0x00), /* Command processor synchronization. S0 */

        ERR_CMD_BUSY   = UINT8(0x10), /* Command was not executed. S2 */
        ERR_DAQ_ACTIVE = UINT8(0x11), /* Command rejected because DAQ is running. S2 */
        ERR_PGM_ACTIVE = UINT8(0x12), /* Command rejected because PGM is running. S2 */

        ERR_CMD_UNKNOWN  = UINT8(0x20),      /* Unknown command or not implemented optional command. S2 */
        ERR_CMD_SYNTAX   = UINT8(0x21),      /* Command syntax invalid   S2 */
        ERR_OUT_OF_RANGE = UINT8(0x22),      /* Command syntax valid but command
                                                  parameter(s) out of range.   S2 */
        ERR_WRITE_PROTECTED   = UINT8(0x23), /* The memory location is write protected. S2 */
        ERR_ACCESS_DENIED     = UINT8(0x24), /* The memory location is not accessible. S2 */
        ERR_ACCESS_LOCKED     = UINT8(0x25), /* Access denied, Seed & Key is required S2 */
        ERR_PAGE_NOT_VALID    = UINT8(0x26), /* Selected page not available    S2 */
        ERR_MODE_NOT_VALID    = UINT8(0x27), /* Selected page mode not available    S2 */
        ERR_SEGMENT_NOT_VALID = UINT8(0x28), /* Selected segment not valid S2 */
        ERR_SEQUENCE          = UINT8(0x29), /* Sequence error          S2 */
        ERR_DAQ_CONFIG        = UINT8(0x2A), /* DAQ configuration not valid        S2 */

        ERR_MEMORY_OVERFLOW = UINT8(0x30), /* Memory overflow error S2 */
        ERR_GENERIC         = UINT8(0x31), /* Generic error.         S2 */
        ERR_VERIFY          = UINT8(0x32), /* The slave internal program verify routine detects
                                                an error.   S3 */

        /* NEW IN 1.1 */
        ERR_RESOURCE_TEMPORARY_NOT_ACCESSIBLE = UINT8(0x33), /* Access to the requested resource is temporary not
                                                                      possible.   S3 */
        ERR_TIMECORR_STATE_CHANGE = UINT8(0x34),             /* State change occurred during time correlation. S3 */
        /* Internal Success Code - not related to XCP spec. */
        ERR_SUCCESS = UINT8(0xff)
    } Xcp_ReturnType;

    /*
     **  Transport Layer Commands / XcpOnCAN
     */
    #define XCP_GET_SLAVE_ID (0xFF)
    #define XCP_GET_DAQ_ID   (0xFE)
    #define XCP_SET_DAQ_ID   (0xFD)

    typedef struct tagXcp_MtaType {
        uint8_t             ext;
        Xcp_PointerSizeType address;
    } Xcp_MtaType;

    #if XCP_ENABLE_DAQ_COMMANDS == XCP_ON
    typedef struct tagXcpDaq_MtaType {
        #if XCP_DAQ_ENABLE_ADDR_EXT == XCP_ON
        uint8_t ext;
        #endif /* XCP_DAQ_ENABLE_ADDR_EXT */
        Xcp_PointerSizeType address;
    } XcpDaq_MtaType;

    typedef enum tagXcpDaq_ProcessorStateType {
        XCP_DAQ_STATE_UNINIT         = 0,
        XCP_DAQ_STATE_CONFIG_INVALID = 1,
        XCP_DAQ_STATE_CONFIG_VALID   = 2,
        XCP_DAQ_STATE_STOPPED        = 3,
        XCP_DAQ_STATE_RUNNING        = 4
    } XcpDaq_ProcessorStateType;

    typedef struct tagXcpDaq_ProcessorType {
        XcpDaq_ProcessorStateType state;
    } XcpDaq_ProcessorType;

    typedef struct tagXcpDaq_PointerType {
        XcpDaq_ListIntegerType     daqList;
        XcpDaq_ODTIntegerType      odt;
        XcpDaq_ODTEntryIntegerType odtEntry;
    } XcpDaq_PointerType;
    #endif /* XCP_ENABLE_DAQ_COMMANDS */

    #if XCP_ENABLE_PGM_COMMANDS == XCP_ON
    typedef enum tagXcpPgm_ProcessorStateType {
        XCP_PGM_STATE_UNINIT = 0,
        XCP_PGM_IDLE         = 1,
        XCP_PGM_ACTIVE       = 2,
    } XcpPgm_ProcessorStateType;

    typedef struct tagXcpPgm_ProcessorType {
        XcpPgm_ProcessorStateType state;
    } XcpPgm_ProcessorType;
    #endif /* ENABLE_PGM_COMMANDS */

    #if (XCP_ENABLE_SLAVE_BLOCKMODE == XCP_ON) || (XCP_ENABLE_MASTER_BLOCKMODE == XCP_ON)
    typedef struct tagXcp_BlockModeStateType {
        bool    blockTransferActive;
        uint8_t remaining;
    } Xcp_BlockModeStateType;
    #endif /* XCP_ENABLE_SLAVE_BLOCKMODE */

    #if XCP_ENABLE_STATISTICS == XCP_ON
    typedef struct tagXcp_StatisticsType {
        uint32_t ctosReceived;
        uint32_t crosSend;
        uint32_t crosBusy;
    } Xcp_StatisticsType;
    #endif /* XCP_ENABLE_STATISTICS */

    typedef struct tagXcp_StateType {
        /* TODO: replace with bitmap. */
        bool connected;
        bool busy;
        bool programming;
    #if XCP_ENABLE_DAQ_COMMANDS == XCP_ON
        XcpDaq_ProcessorType daqProcessor;
        XcpDaq_PointerType   daqPointer;
    #endif /* XCP_ENABLE_DAQ_COMMANDS */
    #if XCP_ENABLE_PGM_COMMANDS == XCP_ON
        XcpPgm_ProcessorType pgmProcessor;
    #endif /* ENABLE_PGM_COMMANDS */
    #if XCP_TRANSPORT_LAYER_COUNTER_SIZE != 0
        uint16_t counter;
    #endif /* XCP_TRANSPORT_LAYER_COUNTER_SIZE */
        uint8_t mode;
    #if XCP_ENABLE_RESOURCE_PROTECTION == XCP_ON
        uint8_t resourceProtection;
        uint8_t seedRequested;
    #endif /* XCP_ENABLE_RESOURCE_PROTECTION */
        Xcp_MtaType mta;
    #if XCP_ENABLE_SLAVE_BLOCKMODE == XCP_ON
        Xcp_BlockModeStateType slaveBlockModeState;
    #endif /* XCP_ENABLE_SLAVE_BLOCKMODE */
    #if XCP_ENABLE_MASTER_BLOCKMODE == XCP_ON
        Xcp_BlockModeStateType masterBlockModeState;
    #endif /* XCP_ENABLE_MASTER_BLOCKMODE */
    #if XCP_ENABLE_STATISTICS == XCP_ON
        Xcp_StatisticsType statistics;
    #endif /* XCP_ENABLE_STATISTICS */
    } Xcp_StateType;

    typedef enum tagXcp_DTOType {
        EVENT_MESSAGE          = 254,
        COMMAND_RETURN_MESSAGE = 255
    } Xcp_DTOType;

    typedef enum tagXcp_ConnectionStateType {
        XCP_DISCONNECTED = 0,
        XCP_CONNECTED    = 1
    } Xcp_ConnectionStateType;

    typedef enum tagXcp_SlaveAccessType {
        XCP_ACC_PGM = 0x40,
        XCP_ACC_DAQ = 0x02,
        XCP_ACC_CAL = 0x01
    } Xcp_SlaveAccessType;

    typedef struct tagXcp_PDUType {
        uint16_t len;
        uint8_t *data;
    } Xcp_PduType;

    typedef struct tagXcp_GetIdType {
        uint16_t       len;
        uint8_t const *name;
    } Xcp_GetIdType;

    #if XCP_ENABLE_DAQ_COMMANDS == XCP_ON
    typedef struct tagXcpDaq_ODTEntryType {
        XcpDaq_MtaType mta;
        #if XCP_DAQ_ENABLE_BIT_OFFSET == XCP_ON
        uint8_t bitOffset;
        #endif /* XCP_DAQ_ENABLE_BIT_OFFSET */
        uint32_t length;
    } XcpDaq_ODTEntryType;

    typedef struct tagXcpDaq_ODTType {
        XcpDaq_ODTEntryIntegerType numOdtEntries;
        uint16_t                   firstOdtEntry;
    } XcpDaq_ODTType;

    typedef enum tagXcpDaq_DirectionType {
        XCP_DIRECTION_NONE,
        XCP_DIRECTION_DAQ,
        XCP_DIRECTION_STIM,
        XCP_DIRECTION_DAQ_STIM
    } XcpDaq_DirectionType;

    typedef struct tagXcpDaq_MeasurementVariable {
        volatile void *variable;
        uint32_t       length;
    } XcpDaq_MeasurementVariable;

    typedef struct tagXcpDaq_DynamicListType {
        XcpDaq_ODTIntegerType numOdts;
        uint16_t              firstOdt;
        uint8_t               mode;
        #if XCP_DAQ_ENABLE_PRESCALER == XCP_ON
        uint8_t prescaler;
        uint8_t counter;
        #endif /* XCP_DAQ_ENABLE_PRESCALER */
    } XcpDaq_DynamicListType;

    typedef struct tagXcpDaq_ListConfigurationType {
        XcpDaq_ODTIntegerType numOdts;
        uint16_t              firstOdt;
    } XcpDaq_ListConfigurationType;

    typedef struct tagXcpDaq_ListStateType {
        uint8_t mode;
        #if XCP_DAQ_ENABLE_PRESCALER == XCP_ON
        uint8_t prescaler;
        uint8_t counter;
        #endif /* XCP_DAQ_ENABLE_PRESCALER */
    } XcpDaq_ListStateType;

    typedef enum tagXcpDaq_EntityKindType {
        XCP_ENTITY_UNUSED,
        XCP_ENTITY_DAQ_LIST,
        XCP_ENTITY_ODT,
        XCP_ENTITY_ODT_ENTRY
    } XcpDaq_EntityKindType;

    typedef struct tagXcpDaq_EntityType {
        /*Xcp_DaqEntityKindType*/
        uint8_t kind;

        union {
            XcpDaq_ODTEntryType    odtEntry;
            XcpDaq_ODTType         odt;
            XcpDaq_DynamicListType daqList;
        } entity;
    } XcpDaq_EntityType;

    typedef struct tagXcpDaq_EventType {
        uint8_t const * const name;
        uint8_t               nameLen;
        uint8_t               properties;
        uint8_t               timeunit;
        uint8_t               cycle;
        /* unit8_t priority; */
    } XcpDaq_EventType;

    typedef struct tagXcpDaq_MessageType {
        uint8_t        dlc;
        uint8_t const *data;
    } XcpDaq_MessageType;

    #endif /* XCP_ENABLE_DAQ_COMMANDS */

    typedef enum tagXcp_MemoryAccessType {
        XCP_MEM_ACCESS_READ,
        XCP_MEM_ACCESS_WRITE
    } Xcp_MemoryAccessType;

    typedef enum tagXcp_MemoryMappingResultType {
        XCP_MEMORY_MAPPED,
        XCP_MEMORY_NOT_MAPPED,
        XCP_MEMORY_ADDRESS_INVALID
    } Xcp_MemoryMappingResultType;

    typedef void (*Xcp_SendCalloutType)(Xcp_PduType const *pdu);

    typedef void (*Xcp_ServerCommandType)(Xcp_PduType const * const pdu);

    typedef struct tagXcp_1DArrayType {
        uint8_t  length;
        uint8_t *data;
    } Xcp_1DArrayType;

    typedef struct tagXcp_OptionsType {
    #if XCP_TRANSPORT_LAYER == XCP_ON_ETHERNET
        bool     ipv6;
        bool     tcp;
        uint16_t port;
    #elif XCP_TRANSPORT_LAYER == XCP_ON_BTH
    char interface2[6];
    #elif XCP_TRANSPORT_LAYER == XCP_ON_CAN
    bool fd;
    char can_interf[64];
    #elif XCP_TRANSPORT_LAYER == XCP_ON_SXI
    bool dummy;
    #endif
    } Xcp_OptionsType;

    /*
    ** Global User Functions.
    */
    void Xcp_Init(void);

    void Xcp_MainFunction(void);

    /*
    ** Global Helper Functions.
    */
    extern Xcp_OptionsType Xcp_Options;

    void Xcp_DispatchCommand(Xcp_PduType const * const pdu);

    void Xcp_Disconnect(void);

    Xcp_ConnectionStateType Xcp_GetConnectionState(void);

    void Xcp_SetSendCallout(Xcp_SendCalloutType callout);

    Xcp_MtaType Xcp_GetNonPagedAddress(void const * const ptr);

    void Xcp_SetMta(Xcp_MtaType mta);

    void Xcp_SetBusy(bool enable);

    bool Xcp_IsBusy(void);

    void Xcp_UploadSingleBlock(void);

    Xcp_StateType *Xcp_GetState(void);

    #if XCP_ENABLE_DAQ_COMMANDS == XCP_ON

    /*
    ** DAQ Implementation Functions.
    */
    void XcpDaq_Init(void);

    Xcp_ReturnType XcpDaq_Free(void);

    Xcp_ReturnType XcpDaq_Alloc(XcpDaq_ListIntegerType daqCount);

    Xcp_ReturnType XcpDaq_AllocOdt(XcpDaq_ListIntegerType daqListNumber, XcpDaq_ODTIntegerType odtCount);

    Xcp_ReturnType XcpDaq_AllocOdtEntry(
        XcpDaq_ListIntegerType daqListNumber, XcpDaq_ODTIntegerType odtNumber, XcpDaq_ODTEntryIntegerType odtEntriesCount
    );

    XcpDaq_ListStateType *XcpDaq_GetListState(XcpDaq_ListIntegerType daqListNumber);

    XcpDaq_ListConfigurationType const *XcpDaq_GetListConfiguration(XcpDaq_ListIntegerType daqListNumber);

    XcpDaq_ODTType const *XcpDaq_GetOdt(XcpDaq_ListIntegerType daqListNumber, XcpDaq_ODTIntegerType odtNumber);

    XcpDaq_ODTEntryType *XcpDaq_GetOdtEntry(
        XcpDaq_ListIntegerType daqListNumber, XcpDaq_ODTIntegerType odtNumber, XcpDaq_ODTEntryIntegerType odtEntryNumber
    );

    bool XcpDaq_ValidateConfiguration(void);

    bool XcpDaq_ValidateList(XcpDaq_ListIntegerType daqListNumber);

    bool XcpDaq_ValidateOdtEntry(
        XcpDaq_ListIntegerType daqListNumber, XcpDaq_ODTIntegerType odtNumber, XcpDaq_ODTEntryIntegerType odtEntry
    );

    void XcpDaq_AddEventChannel(XcpDaq_ListIntegerType daqListNumber, uint16_t eventChannelNumber);

    void XcpDaq_CopyMemory(void *dst, void const *src, uint32_t len);

    XcpDaq_EventType const *XcpDaq_GetEventConfiguration(uint16_t eventChannelNumber);

    void XcpDaq_TriggerEvent(uint8_t eventChannelNumber);

    void XcpDaq_GetProperties(uint8_t *properties);

    XcpDaq_ListIntegerType XcpDaq_GetListCount(void);

    void XcpDaq_SetProcessorState(XcpDaq_ProcessorStateType state);

    void XcpDaq_StartSelectedLists(void);

    void XcpDaq_StopSelectedLists(void);

    void XcpDaq_StopAllLists(void);

    void XcpDaq_StartStopSingleList(XcpDaq_ListIntegerType daqListNumber, uint8_t mode);

    void XcpDaq_StartStopSynch(uint8_t mode);

    bool XcpDaq_GetFirstPid(XcpDaq_ListIntegerType daqListNumber, XcpDaq_ODTIntegerType *firstPID);

    void XcpDaq_SetPointer(
        XcpDaq_ListIntegerType daqListNumber, XcpDaq_ODTIntegerType odtNumber, XcpDaq_ODTEntryIntegerType odtEntryNumber
    );

    void XcpDaq_SetListMode(
        XcpDaq_ListIntegerType daqListNumber, uint8_t mode, uint16_t eventChannelNumber, uint8_t prescaler, uint8_t priority
    );

    void XcpDaq_WriteEntry(uint8_t bitOffset, uint8_t elemSize, uint8_t adddrExt, Xcp_PointerSizeType address);

    void XcpDaq_ReadEntry(uint8_t *bitOffset, uint8_t *elemSize, uint8_t *adddrExt, Xcp_PointerSizeType *address);

        /*
        **  Predefined DAQ constants.
        */
        #if XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_ON
    extern const XcpDaq_ODTEntryType          XcpDaq_PredefinedOdtEntries[];
    extern const XcpDaq_ODTType               XcpDaq_PredefinedOdts[];
    extern const XcpDaq_ListConfigurationType XcpDaq_PredefinedLists[];
    extern const XcpDaq_ListIntegerType       XcpDaq_PredefinedListCount;
    extern XcpDaq_ListStateType               XcpDaq_PredefinedListsState[];
    extern XcpDaq_ODTEntryType                XcpDaq_PredefinedMeasurementVariables[];
        #endif /* XCP_DAQ_ENABLE_PREDEFINED_LISTS */

    extern const XcpDaq_EventType XcpDaq_Events[];

    XCP_DAQ_ENTITY_TYPE XcpDaq_GetDynamicDaqEntityCount(void);

        #if XCP_DAQ_ENABLE_QUEUING == XCP_ON

    /*
    ** DAQ Queuing Support.
    */

    typedef struct tagXcpDaq_QueueType {
        uint8_t head;
        uint8_t tail;
        bool    overload;
    } XcpDaq_QueueType;

    void            XcpDaq_QueueGetVar(XcpDaq_QueueType *var);
    void            XcpDaq_QueueInit(void);
    XCP_STATIC bool XcpDaq_QueueFull(void);
    bool            XcpDaq_QueueEmpty(void);
    bool            XcpDaq_QueueDequeue(uint16_t *len, uint8_t *data);
    bool            XcpDaq_QueueEnqueue(uint16_t len, uint8_t const *data);

        #endif /* XCP_DAQ_ENABLE_QUEUING */

        /*
        ** Debugging / Testing interface.
        */
        #if XCP_BUILD_TYPE == XCP_DEBUG_BUILD

    void XcpDaq_GetCounts(XCP_DAQ_ENTITY_TYPE *entityCount, XCP_DAQ_ENTITY_TYPE *listCount, XCP_DAQ_ENTITY_TYPE *odtCount);

    uint16_t XcpDaq_TotalDynamicEntityCount(void);

    XcpDaq_EntityType *XcpDaq_GetDynamicEntities(void);

    XcpDaq_EntityType *XcpDaq_GetDynamicEntity(uint16_t num);

    uint8_t *XcpDaq_GetDtoBuffer(void);

        #endif  // XCP_BUILD_TYPE

    #endif /* XCP_ENABLE_DAQ_COMMANDS */

    /*
    ** PGM Functions.
    */
    #if XCP_ENABLE_PGM_COMMANDS == XCP_ON

    void XcpPgm_SetProcessorState(XcpPgm_ProcessorStateType state);

    #endif /* ENABLE_PGM_COMMANDS */

    /*
     * Checksum Methods.
     */
    #define XCP_CHECKSUM_METHOD_XCP_ADD_11       (1)
    #define XCP_CHECKSUM_METHOD_XCP_ADD_12       (2)
    #define XCP_CHECKSUM_METHOD_XCP_ADD_14       (3)
    #define XCP_CHECKSUM_METHOD_XCP_ADD_22       (4)
    #define XCP_CHECKSUM_METHOD_XCP_ADD_24       (5)
    #define XCP_CHECKSUM_METHOD_XCP_ADD_44       (6)
    #define XCP_CHECKSUM_METHOD_XCP_CRC_16       (7)
    #define XCP_CHECKSUM_METHOD_XCP_CRC_16_CITT  (8)
    #define XCP_CHECKSUM_METHOD_XCP_CRC_32       (9)
    #define XCP_CHECKSUM_METHOD_XCP_USER_DEFINED (0xff)

    #define XCP_DAQ_TIMESTAMP_UNIT_1NS   (0)
    #define XCP_DAQ_TIMESTAMP_UNIT_10NS  (1)
    #define XCP_DAQ_TIMESTAMP_UNIT_100NS (2)
    #define XCP_DAQ_TIMESTAMP_UNIT_1US   (3)
    #define XCP_DAQ_TIMESTAMP_UNIT_10US  (4)
    #define XCP_DAQ_TIMESTAMP_UNIT_100US (5)
    #define XCP_DAQ_TIMESTAMP_UNIT_1MS   (6)
    #define XCP_DAQ_TIMESTAMP_UNIT_10MS  (7)
    #define XCP_DAQ_TIMESTAMP_UNIT_100MS (8)
    #define XCP_DAQ_TIMESTAMP_UNIT_1S    (9)
    #define XCP_DAQ_TIMESTAMP_UNIT_1PS   (10)
    #define XCP_DAQ_TIMESTAMP_UNIT_10PS  (11)
    #define XCP_DAQ_TIMESTAMP_UNIT_100PS (12)

    #define XCP_DAQ_TIMESTAMP_SIZE_1 (1)
    #define XCP_DAQ_TIMESTAMP_SIZE_2 (2)
    #define XCP_DAQ_TIMESTAMP_SIZE_4 (4)

    /*
    **
    */
    void Xcp_SendCto(void);

    uint8_t *Xcp_GetCtoOutPtr(void);

    #if XCP_ENABLE_DAQ_COMMANDS == XCP_ON

    void Xcp_SendDto(void);

    uint8_t *Xcp_GetDtoOutPtr(void);

    void Xcp_SetDtoOutLen(uint16_t len);

    #endif /* XCP_ENABLE_DAQ_COMMANDS */

    void Xcp_SetCtoOutLen(uint16_t len);

    void Xcp_Send8(uint8_t len, uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5, uint8_t b6, uint8_t b7);

    #if XCP_ENABLE_EVENT_PACKET_API == XCP_ON

    void Xcp_SendEventPacket(uint8_t eventCode, uint8_t const * const eventInfo, uint8_t eventInfoLength);

    #endif /* XCP_ENABLE_EVENT_PACKET_API */
    #if XCP_ENABLE_SERVICE_REQUEST_API == XCP_ON
    void Xcp_SendServiceRequestPacket(
        uint8_t serviceRequestCode, uint8_t const * const serviceRequest, uint8_t serviceRequestLength
    );
    #endif /* XCP_ENABLE_SERVICE_REQUEST_API */

    /*
    **  Helpers.
    */
    void Xcp_DisplayInfo(void);

    void Xcp_CopyMemory(Xcp_MtaType dst, Xcp_MtaType src, uint32_t len);

    uint8_t Xcp_GetByte(Xcp_PduType const * const value, uint8_t offs);

    uint16_t Xcp_GetWord(Xcp_PduType const * const value, uint8_t offs);

    uint32_t Xcp_GetDWord(Xcp_PduType const * const value, uint8_t offs);

    void Xcp_SetByte(Xcp_PduType const * const pdu, uint8_t offs, uint8_t value);

    void Xcp_SetWord(Xcp_PduType const * const pdu, uint8_t offs, uint16_t value);

    void Xcp_SetDWord(Xcp_PduType const * const pdu, uint8_t offs, uint32_t value);

    /*
    **  Transport Layer Stuff.
    */
    #define XCP_COMM_BUFLEN                                                                                                        \
        ((XCP_MAX(XCP_MAX_CTO, XCP_MAX_DTO)) + XCP_TRANSPORT_LAYER_LENGTH_SIZE + XCP_TRANSPORT_LAYER_COUNTER_SIZE +                \
         XCP_TRANSPORT_LAYER_CHECKSUM_SIZE)

    void XcpTl_Init(void);

    void XcpTl_DeInit(void);

    int16_t XcpTl_FrameAvailable(uint32_t sec, uint32_t usec);

    void XcpTl_RxHandler(void);

    void XcpTl_Send(uint8_t const *buf, uint16_t len);

    void XcpTl_MainFunction(void);

    void XcpTl_SaveConnection(void);

    void XcpTl_ReleaseConnection(void);

    bool XcpTl_VerifyConnection(void);

    void XcpTl_FeedReceiver(uint8_t octet);

    void XcpTl_TransportLayerCmd_Res(Xcp_PduType const * const pdu);

    void XcpTl_PrintConnectionInformation(void);

    /*
    **  Customization Stuff.
    */
    bool Xcp_HookFunction_GetId(uint8_t id_type, uint8_t **result, uint32_t *result_length);

    bool Xcp_HookFunction_GetSeed(uint8_t resource, Xcp_1DArrayType *result);

    /*
    **  Hardware dependent stuff.
    */
    void XcpHw_Init(void);
    void XcpHw_PosixInit(void);
    void XcpHw_Deinit(void);

    uint32_t XcpHw_GetTimerCounter(void);

    uint32_t XcpHw_GetTimerCounterMS(void);

    void XcpHw_AcquireLock(uint8_t lockIdx);

    void XcpHw_ReleaseLock(uint8_t lockIdx);

    void XcpDaq_TransmitDtos(void);

    extern Xcp_PduType Xcp_CtoIn;
    extern Xcp_PduType Xcp_CtoOut;

    #if XCP_TRANSPORT_LAYER == XCP_ON_SXI
    /*
    ** Serial Port Function Prototypes.
    */
    void     Serial_Init(void);
    void     Serial_DeInit(void);
    uint32_t Serial_Available(void);
    bool     Serial_Read(uint8_t *in_byte);
    void     Serial_WriteByte(uint8_t out_byte);
    void     Serial_WriteBuffer(uint8_t const *out_bytes, uint32_t size);
    void     Serial_MainFunction(void);

    #endif

    #if (XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_CRC_16) || (XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_CRC_16_CITT)
    typedef uint16_t Xcp_ChecksumType;
    #elif XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_CRC_32
typedef uint32_t Xcp_ChecksumType;
    #elif XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_ADD_11
typedef uint8_t Xcp_ChecksumType;
    #elif (XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_ADD_12) || (XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_ADD_22)
typedef uint16_t Xcp_ChecksumType;
    #elif (XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_ADD_14) || (XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_ADD_24) ||    \
        (XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_ADD_44)
typedef uint32_t Xcp_ChecksumType;
    #endif /* XCP_CHECKSUM_METHOD */

    void Xcp_ChecksumInit(void);

    Xcp_ChecksumType Xcp_CalculateChecksum(uint8_t const *ptr, uint32_t length, Xcp_ChecksumType startValue, bool isFirstCall);

    void Xcp_ChecksumMainFunction(void);

    void Xcp_SendChecksumPositiveResponse(Xcp_ChecksumType checksum);

    void Xcp_SendChecksumOutOfRangeResponse(void);

    void Xcp_StartChecksumCalculation(uint8_t const *ptr, uint32_t size);

    #if XCP_ENABLE_EXTERN_C_GUARDS == XCP_ON
        #if defined(__cplusplus)
}
        #endif /* __cplusplus */
    #endif     /* XCP_EXTERN_C_GUARDS */

    /* Include time correlation module after all XCP types are defined */
    #if defined(XCP_ENABLE_TIME_CORRELATION) && (XCP_ENABLE_TIME_CORRELATION == XCP_ON)
        #include "xcp_timecorr.h"
    #endif

#endif /* __CXCP_H */

////////////////////////////////////////////////////////////////////////////////
//                                  xcp_hw.h                                  //
////////////////////////////////////////////////////////////////////////////////

/*
 * BlueParrot XCP
 *
 * (C) 2007-2025 by Christoph Schueler <github.com/Christoph2,
 *                                      cpu12.gems@googlemail.com>
 *
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * s. FLOSS-EXCEPTION.txt
 */

#if !defined(__XCP_HW_H)
#define __XCP_HW_H

#if XCP_ENABLE_EXTERN_C_GUARDS == XCP_ON
#if defined(__cplusplus)
extern "C" {



#endif /* __cplusplus */
#endif     /* XCP_EXTERN_C_GUARDS */


/*
**  Global Functions.
*/
void XcpHw_ParseCommandLineOptions(int argc, char **argv, Xcp_OptionsType *options);
void XcpHw_ErrorMsg(char *const function, int errorCode);
void XcpHw_Sleep(uint64_t usec);

#if XCP_ENABLE_EXTERN_C_GUARDS == XCP_ON
#if defined(__cplusplus)
}
#endif /* __cplusplus */
#endif     /* XCP_EXTERN_C_GUARDS */

#endif /* __XCP_HW_H */

////////////////////////////////////////////////////////////////////////////////
//                               xcp_timecorr.h                               //
////////////////////////////////////////////////////////////////////////////////

/*
 * BlueParrot XCP
 *
 * (C) 2026 by Christoph Schueler <github.com/Christoph2,
 *                                  cpu12.gems@googlemail.com>
 *
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * s. FLOSS-EXCEPTION.txt
 */

/*
 * XCP Advanced Time Correlation (spec chapters 7.5.6.1, 5.4, 7.7.8)
 *
 * Implements TIME_CORRELATION_PROPERTIES (0xC6) and GET_DAQ_CLOCK_MULTICAST
 * (transport layer cmd 0xF2/0xFA), plus the extended EV_TIME_SYNC event.
 *
 * External time sources (e.g. GPS/PPS modules) are plugged in via
 * XcpTimecorr_SetGrandmasterClockSource().
 */

#if !defined(__XCP_TIMECORR_H)
    #define __XCP_TIMECORR_H

    

    #if defined(XCP_ENABLE_TIME_CORRELATION) && (XCP_ENABLE_TIME_CORRELATION == XCP_ON)

    /*
    ** ============================================================
    **  Configuration defaults (override in xcp_config.h)
    ** ============================================================
    */

    /* Observable XCP slave clock type:
     *   0 = free-running, can be read randomly
     *   1 = may be syntonized/synchronized to grandmaster, readable
     *   2 = no local clock (DAQ timestamps from external source)
     */
        #if !defined(XCP_TIMECORR_XCP_SLV_CLK)
            #define XCP_TIMECORR_XCP_SLV_CLK (0)
        #endif

    /* Observable grandmaster clock type:
     *   0 = no dedicated grandmaster clock
     *   1 = dedicated grandmaster clock, readable randomly
     *   2 = dedicated grandmaster clock, NOT readable randomly; slave generates EV_TIME_SYNC autonomously
     */
        #if !defined(XCP_TIMECORR_GRANDM_CLK)
            #define XCP_TIMECORR_GRANDM_CLK (0)
        #endif

    /* Observable ECU clock type:
     *   0 = no ECU clock
     *   1 = ECU clock readable randomly
     *   2 = ECU clock not readable randomly; slave generates EV_TIME_SYNC autonomously
     *   3 = ECU timestamps reported, ECU clock not readable
     */
        #if !defined(XCP_TIMECORR_ECU_CLK)
            #define XCP_TIMECORR_ECU_CLK (0)
        #endif

    /* Default cluster affiliation (determines multicast group).
     * Maps to IPv4 239.255.UPPER_BYTE_LE.LOWER_BYTE_LE
     * 0x0001 → 239.255.0.1
     */
        #if !defined(XCP_TIMECORR_DEFAULT_CLUSTER_ID)
            #define XCP_TIMECORR_DEFAULT_CLUSTER_ID (0x0001u)
        #endif

    /* Native timestamp size for the XCP slave clock (4 or 8 bytes). */
        #if !defined(XCP_TIMECORR_SLV_NATIVE_TS_SIZE)
            #define XCP_TIMECORR_SLV_NATIVE_TS_SIZE (4u)
        #endif

    /* XCP slave clock UUID (EUI-64, 8 bytes, most significant byte first).
     * Should be unique per device. Override in xcp_config.h. */
        #if !defined(XCP_TIMECORR_SLV_UUID)
            #define XCP_TIMECORR_SLV_UUID { 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x01u }
        #endif

    /* XCP slave clock stratum level (255 = unknown). */
        #if !defined(XCP_TIMECORR_SLV_STRATUM)
            #define XCP_TIMECORR_SLV_STRATUM (255u)
        #endif

    /* Multicast UDP port (fixed by XCP spec). */
        #define XCP_TIMECORR_MULTICAST_PORT (5557u)

    /*
    ** ============================================================
    **  Constants
    ** ============================================================
    */

    /* Stratum level for unknown clock quality */
        #define XCP_TIMECORR_STRATUM_UNKNOWN (255u)

    /* Native timestamp size values */
        #define XCP_TIMECORR_NATIVE_TS_DWORD (4u)
        #define XCP_TIMECORR_NATIVE_TS_DLONG (8u)

    /* Epoch values (for grandmaster clock info) */
        #define XCP_TIMECORR_EPOCH_TAI       (0u)
        #define XCP_TIMECORR_EPOCH_UTC       (1u)
        #define XCP_TIMECORR_EPOCH_ARBITRARY (2u)

    /* TRIGGER_INITIATOR field values (bits [2:0] of TRIGGER_INFO) */
        #define XCP_TIMECORR_TRIG_HW          (0u) /* HW trigger / Vector Syncline */
        #define XCP_TIMECORR_TRIG_INDEPENDENT (1u) /* XCP-independent sync event (e.g. PPS) */
        #define XCP_TIMECORR_TRIG_MULTICAST   (2u) /* GET_DAQ_CLOCK_MULTICAST */
        #define XCP_TIMECORR_TRIG_BRIDGE      (3u) /* GET_DAQ_CLOCK_MULTICAST via Time Sync Bridge */
        #define XCP_TIMECORR_TRIG_SYNC_STATE  (4u) /* Syntonization/synchronization state change */
        #define XCP_TIMECORR_TRIG_LEAP_SECOND (5u) /* Leap second on grandmaster */
        #define XCP_TIMECORR_TRIG_ECU_RESET   (6u) /* Release of ECU reset */

    /* TIME_OF_TS_SAMPLING field values (bits [4:3] of TRIGGER_INFO) */
        #define XCP_TIMECORR_TS_SAMP_CMD (0u) /* During command processing */
        #define XCP_TIMECORR_TS_SAMP_ISR (1u) /* Low-jitter, high-priority interrupt */
        #define XCP_TIMECORR_TS_SAMP_TX  (2u) /* Upon physical transmission */
        #define XCP_TIMECORR_TS_SAMP_RX  (3u) /* Upon physical reception (recommended for multicast) */

    /* SLV_CLK_SYNC_STATE values (bits [2:0] of SYNC_STATE field) */
        #define XCP_TIMECORR_SLV_SYNC_IN_PROGRESS   (0u) /* Synchronization in progress */
        #define XCP_TIMECORR_SLV_SYNCED             (1u) /* Synchronized to grandmaster */
        #define XCP_TIMECORR_SLV_SYNTON_IN_PROGRESS (2u) /* Syntonization in progress */
        #define XCP_TIMECORR_SLV_SYNTONIZED         (3u) /* Syntonized to grandmaster */
        #define XCP_TIMECORR_SLV_NO_SYNC_SUPPORT    (7u) /* No sync/synton capability */

    /* GRANDM_CLK_SYNC_STATE values (bit [3] of SYNC_STATE field) */
        #define XCP_TIMECORR_GRANDM_NOT_SYNCED (0u)
        #define XCP_TIMECORR_GRANDM_SYNCED     (1u)

    /* ECU_CLK_SYNC_STATE values (bits [5:4] of SYNC_STATE field) */
        #define XCP_TIMECORR_ECU_NOT_SYNCED   (0u)
        #define XCP_TIMECORR_ECU_SYNCED       (1u)
        #define XCP_TIMECORR_ECU_SYNC_UNKNOWN (2u)

/*
** ============================================================
**  Types
** ============================================================
*/

/*
 * Clock identification and characteristics.
 * Used both in the upload data block and internally.
 */
typedef struct tagXcpTimecorr_ClockInfoType {
    uint8_t  uuid[8];       /* EUI-64 unique clock identifier (most significant byte first) */
    uint16_t ticks;         /* Timestamp ticks per unit */
    uint8_t  unit;          /* Timestamp unit (XCP_DAQ_TIMESTAMP_UNIT_*) */
    uint8_t  stratum;       /* Clock quality by stratum level; 255 = unknown */
    uint8_t  native_size;   /* Size of native counter: 4 (DWORD) or 8 (DLONG) */
    uint8_t  epoch;         /* Epoch: 0=TAI, 1=UTC, 2=arbitrary (only meaningful for grandmaster) */
    uint64_t max_timestamp; /* Last valid value before wrap-around */
} XcpTimecorr_ClockInfoType;

/*
 * External time source interface.
 *
 * Implement this struct to connect any external time source (GPS/PPS, IEEE 1588 PTP, etc.)
 * as a grandmaster clock observable by the XCP slave.
 *
 * Usage example (GPS module with 1PPS):
 *
 *   static uint32_t gps_get_ts(void) { return gps_read_counter(); }
 *   static uint8_t  gps_sync_state(void) { return gps_is_locked()
 *                                             ? XCP_TIMECORR_GRANDM_SYNCED
 *                                             : XCP_TIMECORR_GRANDM_NOT_SYNCED; }
 *   static const XcpTimecorr_TimeSourceType GPS_Source = {
 *       .info = {
 *           .uuid         = { 0xAA, 0xBB, 0xCC, 0xFF, 0xFE, 0x01, 0x02, 0x03 },
 *           .ticks        = 1,
 *           .unit         = XCP_DAQ_TIMESTAMP_UNIT_1US,
 *           .stratum      = 1,
 *           .native_size  = XCP_TIMECORR_NATIVE_TS_DWORD,
 *           .epoch        = XCP_TIMECORR_EPOCH_UTC,
 *           .max_timestamp = 0xFFFFFFFFULL,
 *       },
 *       .get_timestamp_u32 = gps_get_ts,
 *       .get_timestamp_u64 = NULL,
 *       .get_sync_state    = gps_sync_state,
 *   };
 *   XcpTimecorr_SetGrandmasterClockSource(&GPS_Source);
 */
typedef struct tagXcpTimecorr_TimeSourceType {
    /* Static clock characteristics (filled once at startup) */
    XcpTimecorr_ClockInfoType info;

    /* Read current 32-bit timestamp. Used when info.native_size == 4.
     * Must not be NULL when info.native_size == 4. */
    uint32_t (*get_timestamp_u32)(void);

    /* Read current 64-bit timestamp. Used when info.native_size == 8.
     * May be NULL when info.native_size == 4. */
    uint64_t (*get_timestamp_u64)(void);

    /* Return current synchronization state (XCP_TIMECORR_GRANDM_*).
     * May be NULL; treated as XCP_TIMECORR_GRANDM_NOT_SYNCED when NULL. */
    uint8_t (*get_sync_state)(void);
} XcpTimecorr_TimeSourceType;

/* Current response format (RESPONSE_FMT in SLAVE_CONFIG) */
typedef enum tagXcpTimecorr_ResponseFmtType {
    XCP_TIMECORR_FMT_LEGACY = 0, /* Legacy mode (after CONNECT, before master activates) */
    XCP_TIMECORR_FMT_EXT_1  = 1, /* Extended: EV_TIME_SYNC for TRIGGER_INITIATOR 0, 2, 3 only */
    XCP_TIMECORR_FMT_EXT_2  = 2, /* Extended: EV_TIME_SYNC for all trigger conditions */
} XcpTimecorr_ResponseFmtType;

/*
** ============================================================
**  Public API
** ============================================================
*/

/* Initialize time correlation module.
 * Called automatically from Xcp_Init() when XCP_ENABLE_TIME_CORRELATION is on. */
void XcpTimecorr_Init(void);

/* Register an external grandmaster clock source (e.g. GPS/PPS module).
 * Set source to NULL to remove grandmaster clock.
 * This maps to GRANDM_CLK in OBSERVABLE_CLOCKS (scenario 4a / 4b). */
void XcpTimecorr_SetGrandmasterClockSource(XcpTimecorr_TimeSourceType const *source);

/* Handle TIME_CORRELATION_PROPERTIES command (0xC6). Called from xcp.c. */
void XcpTimecorr_HandleTimeCorrelationProperties(Xcp_PduType const * const pdu);

/* Handle GET_DAQ_CLOCK_MULTICAST reception.
 * Builds and sends extended EV_TIME_SYNC event packet if XCP is connected
 * and the cluster_id matches the cluster affiliation (or cluster_affiliation == 0).
 * cluster_id : cluster identifier from multicast packet (Intel byte order)
 * counter    : counter byte copied from GET_DAQ_CLOCK_MULTICAST */
void XcpTimecorr_HandleMulticast(uint16_t cluster_id, uint8_t counter);

/* Send EV_TIME_SYNC for a non-multicast trigger (e.g. PPS signal, state change).
 * Only sent when RESPONSE_FMT == XCP_TIMECORR_FMT_EXT_2.
 * trigger_initiator: one of XCP_TIMECORR_TRIG_* */
void XcpTimecorr_SendTimeSyncEvent(uint8_t trigger_initiator);

/* Query current response format */
XcpTimecorr_ResponseFmtType XcpTimecorr_GetResponseFmt(void);

/* Query current cluster affiliation */
uint16_t XcpTimecorr_GetClusterAffiliation(void);

/* Notify the transport layer that the cluster affiliation has changed
 * so it can rejoin the correct multicast group.
 * Implemented in the transport layer (wineth.c / linuxeth.c). */
void XcpTl_UpdateMulticastGroup(uint16_t cluster_id);

    #endif /* XCP_ENABLE_TIME_CORRELATION */

#endif /* __XCP_TIMECORR_H */

////////////////////////////////////////////////////////////////////////////////
//                                   xcp.c                                    //
////////////////////////////////////////////////////////////////////////////////

/*
 * BlueParrot XCP
 *
 * (C) 2007-2022 by Christoph Schueler <github.com/Christoph2,
 *                                      cpu12.gems@googlemail.com>
 *
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * s. FLOSS-EXCEPTION.txt
 */


#if defined(XCP_ENABLE_TIME_CORRELATION) && (XCP_ENABLE_TIME_CORRELATION == XCP_ON)
    #include "xcp_timecorr.h"
#endif

#include <inttypes.h> /* for PRIxPTR */

// #include "common/tusb_debug.h"
/*
** Private Options.
*/
#define XCP_ENABLE_STD_COMMANDS     XCP_ON
#define XCP_ENABLE_INTERLEAVED_MODE XCP_OFF

#define XCP_DRIVER_VERSION (10)

/*
 *  Local Constants.
 */
XCP_STATIC const Xcp_GetIdType Xcp_GetId0 = XCP_SET_ID(XCP_GET_ID_0);
XCP_STATIC const Xcp_GetIdType Xcp_GetId1 = XCP_SET_ID(XCP_GET_ID_1);

/*
** Local Variables.
*/
XCP_STATIC Xcp_ConnectionStateType Xcp_ConnectionState = XCP_DISCONNECTED;
XCP_STATIC Xcp_StateType           Xcp_State;
XCP_STATIC Xcp_SendCalloutType     Xcp_SendCallout                                       = (Xcp_SendCalloutType)XCP_NULL;
static uint8_t                     Xcp_CtoOutBuffer[XCP_TRANSPORT_LAYER_CTO_BUFFER_SIZE] = { 0 };
static uint8_t                     Xcp_CtoInBuffer[XCP_TRANSPORT_LAYER_CTO_BUFFER_SIZE]  = { 0 };

/*
**  Global Variables.
*/
Xcp_PduType Xcp_CtoIn  = { 0, &Xcp_CtoInBuffer[0] };
Xcp_PduType Xcp_CtoOut = { 0, &Xcp_CtoOutBuffer[0] };

#if XCP_ENABLE_GET_ID_EPK == XCP_ON
extern const char Xcp_EEPROM_Kennung[];
#endif /* XCP_ENABLE_GET_ID_EPK */

#if XCP_ENABLE_DAQ_COMMANDS == XCP_ON
static uint8_t     Xcp_DtoOutBuffer[XCP_TRANSPORT_LAYER_DTO_BUFFER_SIZE] = { 0 };
static Xcp_PduType Xcp_DtoOut                                            = { 0, &Xcp_DtoOutBuffer[0] };
#endif /* XCP_ENABLE_DAQ_COMMANDS */

void Xcp_WriteMemory(void *dest, void *src, uint16_t count);

void Xcp_ReadMemory(void *dest, void *src, uint16_t count);

/*
** Local Macros.
*/
#define XCP_INCREMENT_MTA(i) Xcp_State.mta.address += UINT32((i))

#if XCP_ENABLE_DAQ_COMMANDS == XCP_ON
    #define XCP_ASSERT_DAQ_STOPPED()                                                                                               \
        do {                                                                                                                       \
            if (Xcp_State.daqProcessor.state == XCP_DAQ_STATE_RUNNING) {                                                           \
                Xcp_SendResult(ERR_DAQ_ACTIVE);                                                                                    \
                return;                                                                                                            \
            }                                                                                                                      \
        } while (0)
#else
    #define XCP_ASSERT_DAQ_STOPPED()
#endif /* ENABLE_DAQ_COMMANDS */

#if XCP_ENABLE_PGM_COMMANDS == XCP_ON
    #define XCP_ASSERT_PGM_IDLE()                                                                                                  \
        do {                                                                                                                       \
            if (Xcp_State.pgmProcessor.state == XCP_PGM_ACTIVE) {                                                                  \
                Xcp_SendResult(ERR_PGM_ACTIVE);                                                                                    \
                return;                                                                                                            \
            }                                                                                                                      \
        } while (0)
#else
    #define XCP_ASSERT_PGM_IDLE()
#endif /* ENABLE_PGM_COMMANDS */

#define XCP_ASSERT_PGM_ACTIVE()                                                                                                    \
    do {                                                                                                                           \
        if (Xcp_State.pgmProcessor.state != XCP_PGM_ACTIVE) {                                                                      \
            Xcp_SendResult(ERR_SEQUENCE);                                                                                          \
            return;                                                                                                                \
        }                                                                                                                          \
    } while (0)

#define XCP_ASSERT_UNLOCKED(r)                                                                                                     \
    do {                                                                                                                           \
        if (Xcp_IsProtected((r))) {                                                                                                \
            Xcp_SendResult(ERR_ACCESS_LOCKED);                                                                                     \
            return;                                                                                                                \
        }                                                                                                                          \
    } while (0)

#if XCP_ENABLE_CHECK_MEMORY_ACCESS == XCP_ON
    #define XCP_CHECK_MEMORY_ACCESS(m, l, a, p)                                                                                    \
        do {                                                                                                                       \
            if (!Xcp_HookFunction_CheckMemoryAccess((m), (l), (a), (p))) {                                                         \
                Xcp_SendResult(ERR_ACCESS_DENIED);                                                                                 \
                return;                                                                                                            \
            }                                                                                                                      \
        } while (0)
#else
    #define XCP_CHECK_MEMORY_ACCESS(m, l, a, p)
#endif /* XCP_ENABLE_CHECK_MEMORY_ACCESS */

/*
** Local Function Prototypes.
*/
XCP_STATIC Xcp_MemoryMappingResultType Xcp_MapMemory(Xcp_MtaType const *src, Xcp_MtaType *dst);

XCP_STATIC void Xcp_Download_Copy(Xcp_PointerSizeType address, uint8_t ext, uint32_t len);

XCP_STATIC void Xcp_PositiveResponse(void);

XCP_STATIC void Xcp_ErrorResponse(uint8_t errorCode);

XCP_STATIC void Xcp_BusyResponse(void);

#if (XCP_ENABLE_SERVICE_REQUEST_API == XCP_ON) || (XCP_ENABLE_EVENT_PACKET_API)

XCP_STATIC void Xcp_SendSpecialPacket(uint8_t packetType, uint8_t code, uint8_t const * const data, uint8_t dataLength);

#endif

XCP_STATIC bool Xcp_IsProtected(uint8_t resource);

XCP_STATIC void Xcp_DefaultResourceProtection(void);

#if XCP_ENABLE_SLAVE_BLOCKMODE == XCP_ON
XCP_STATIC bool Xcp_SlaveBlockTransferIsActive(void);
XCP_STATIC void Xcp_SlaveBlockTransferSetActive(bool onOff);
#endif /* XCP_ENABLE_SLAVE_BLOCKMODE */

XCP_STATIC void Xcp_SendResult(Xcp_ReturnType result);

XCP_STATIC void Xcp_CommandNotImplemented_Res(Xcp_PduType const * const pdu);

XCP_STATIC void Xcp_Connect_Res(Xcp_PduType const * const pdu);

XCP_STATIC void Xcp_Disconnect_Res(Xcp_PduType const * const pdu);

XCP_STATIC void Xcp_GetStatus_Res(Xcp_PduType const * const pdu);

XCP_STATIC void Xcp_Synch_Res(Xcp_PduType const * const pdu);

#if XCP_ENABLE_GET_COMM_MODE_INFO == XCP_ON

XCP_STATIC void Xcp_GetCommModeInfo_Res(Xcp_PduType const * const pdu);

#endif /* XCP_ENABLE_GET_COMM_MODE_INFO */
#if XCP_ENABLE_GET_ID == XCP_ON

XCP_STATIC void Xcp_GetId_Res(Xcp_PduType const * const pdu);

#endif /* XCP_ENABLE_GET_ID */
#if XCP_ENABLE_SET_REQUEST == XCP_ON
XCP_STATIC void Xcp_SetRequest_Res(Xcp_PduType const * const pdu);
#endif /* XCP_ENABLE_SET_REQUEST */
#if XCP_ENABLE_GET_SEED == XCP_ON
XCP_STATIC void Xcp_GetSeed_Res(Xcp_PduType const * const pdu);
#endif /* XCP_ENABLE_GET_SEED */
#if XCP_ENABLE_UNLOCK == XCP_ON
XCP_STATIC void Xcp_Unlock_Res(Xcp_PduType const * const pdu);
#endif /* XCP_ENABLE_UNLOCK */
#if XCP_ENABLE_SET_MTA == XCP_ON

XCP_STATIC void Xcp_SetMta_Res(Xcp_PduType const * const pdu);

#endif /* XCP_ENABLE_SET_MTA */
#if XCP_ENABLE_UPLOAD == XCP_ON

XCP_STATIC void Xcp_Upload_Res(Xcp_PduType const * const pdu);

#endif
#if XCP_ENABLE_SHORT_UPLOAD == XCP_ON

XCP_STATIC void Xcp_ShortUpload_Res(Xcp_PduType const * const pdu);

#endif /* XCP_ENABLE_SHORT_UPLOAD */
#if XCP_ENABLE_BUILD_CHECKSUM == XCP_ON

XCP_STATIC void Xcp_BuildChecksum_Res(Xcp_PduType const * const pdu);

#endif
#if XCP_ENABLE_TRANSPORT_LAYER_CMD == XCP_ON
XCP_STATIC void Xcp_TransportLayerCmd_Res(Xcp_PduType const * const pdu);
#endif /* XCP_ENABLE_TRANSPORT_LAYER_CMD */
#if XCP_ENABLE_USER_CMD == XCP_ON
XCP_STATIC void Xcp_UserCmd_Res(Xcp_PduType const * const pdu);
#endif /* XCP_ENABLE_USER_CMD */

#if XCP_ENABLE_CAL_COMMANDS == XCP_ON

XCP_STATIC void Xcp_Download_Res(Xcp_PduType const * const pdu);

    #if XCP_ENABLE_DOWNLOAD_NEXT == XCP_ON
XCP_STATIC void Xcp_DownloadNext_Res(Xcp_PduType const * const pdu);
    #endif /* XCP_ENABLE_DOWNLOAD_NEXT */
    #if XCP_ENABLE_DOWNLOAD_MAX == XCP_ON

XCP_STATIC void Xcp_DownloadMax_Res(Xcp_PduType const * const pdu);

    #endif /* XCP_ENABLE_DOWNLOAD_MAX */
    #if XCP_ENABLE_SHORT_DOWNLOAD == XCP_ON

XCP_STATIC void Xcp_ShortDownload_Res(Xcp_PduType const * const pdu);

    #endif /* XCP_ENABLE_SHORT_DOWNLOAD */
    #if XCP_ENABLE_MODIFY_BITS == XCP_ON

XCP_STATIC void Xcp_ModifyBits_Res(Xcp_PduType const * const pdu);

    #endif /* XCP_ENABLE_MODIFY_BITS */

XCP_STATIC void Xcp_SetCalPage_Res(Xcp_PduType const * const pdu);

XCP_STATIC void Xcp_GetCalPage_Res(Xcp_PduType const * const pdu);

#endif /* XCP_ENABLE_CAL_COMMANDS */

#if XCP_ENABLE_PAG_COMMANDS == XCP_ON
    #if XCP_ENABLE_GET_PAG_PROCESSOR_INFO == XCP_ON
XCP_STATIC void Xcp_GetPagProcessorInfo_Res(Xcp_PduType const * const pdu);
    #endif /* XCP_ENABLE_GET_PAG_PROCESSOR_INFO */
    #if XCP_ENABLE_GET_SEGMENT_INFO == XCP_ON
XCP_STATIC void Xcp_GetSegmentInfo_Res(Xcp_PduType const * const pdu);
    #endif /* XCP_ENABLE_GET_SEGMENT_INFO */
    #if XCP_ENABLE_GET_PAGE_INFO == XCP_ON
XCP_STATIC void Xcp_GetPageInfo_Res(Xcp_PduType const * const pdu);
    #endif
    #if XCP_ENABLE_SET_SEGMENT_MODE == XCP_ON
XCP_STATIC void Xcp_SetSegmentMode_Res(Xcp_PduType const * const pdu);
    #endif /* XCP_ENABLE_SET_SEGMENT_MODE */
    #if XCP_ENABLE_GET_SEGMENT_MODE == XCP_ON
XCP_STATIC void Xcp_GetSegmentMode_Res(Xcp_PduType const * const pdu);
    #endif /* XCP_ENABLE_GET_SEGMENT_MODE */
    #if XCP_ENABLE_COPY_CAL_PAGE
XCP_STATIC void Xcp_CopyCalPage_Res(Xcp_PduType const * const pdu);
    #endif /* XCP_ENABLE_COPY_CAL_PAGE */
#endif     /* XCP_ENABLE_PAG_COMMANDS */

#if XCP_ENABLE_DAQ_COMMANDS == XCP_ON

XCP_STATIC void Xcp_ClearDaqList_Res(Xcp_PduType const * const pdu);

XCP_STATIC void Xcp_SetDaqPtr_Res(Xcp_PduType const * const pdu);

XCP_STATIC void Xcp_WriteDaq_Res(Xcp_PduType const * const pdu);

XCP_STATIC void Xcp_SetDaqListMode_Res(Xcp_PduType const * const pdu);

XCP_STATIC void Xcp_GetDaqListMode_Res(Xcp_PduType const * const pdu);

XCP_STATIC void Xcp_StartStopDaqList_Res(Xcp_PduType const * const pdu);

XCP_STATIC void Xcp_StartStopSynch_Res(Xcp_PduType const * const pdu);

    #if XCP_ENABLE_GET_DAQ_CLOCK == XCP_ON

XCP_STATIC void Xcp_GetDaqClock_Res(Xcp_PduType const * const pdu);

    #endif /* XCP_ENABLE_GET_DAQ_CLOCK */

    #if defined(XCP_ENABLE_TIME_CORRELATION) && (XCP_ENABLE_TIME_CORRELATION == XCP_ON)
XCP_STATIC void Xcp_TimeCorrelationProperties_Res(Xcp_PduType const * const pdu);
    #endif /* XCP_ENABLE_TIME_CORRELATION */

    #if XCP_ENABLE_READ_DAQ == XCP_ON
XCP_STATIC void Xcp_ReadDaq_Res(Xcp_PduType const * const pdu);
    #endif /* XCP_ENABLE_READ_DAQ */
    #if XCP_ENABLE_GET_DAQ_PROCESSOR_INFO == XCP_ON

XCP_STATIC void Xcp_GetDaqProcessorInfo_Res(Xcp_PduType const * const pdu);

    #endif /* XCP_ENABLE_GET_DAQ_PROCESSOR_INFO */
    #if XCP_ENABLE_GET_DAQ_RESOLUTION_INFO == XCP_ON

XCP_STATIC void Xcp_GetDaqResolutionInfo_Res(Xcp_PduType const * const pdu);

    #endif /* XCP_ENABLE_GET_DAQ_RESOLUTION_INFO */
    #if XCP_ENABLE_GET_DAQ_LIST_INFO == XCP_ON

XCP_STATIC void Xcp_GetDaqListInfo_Res(Xcp_PduType const * const pdu);

    #endif /* XCP_ENABLE_GET_DAQ_LIST_INFO */
    #if XCP_ENABLE_GET_DAQ_EVENT_INFO == XCP_ON

XCP_STATIC void Xcp_GetDaqEventInfo_Res(Xcp_PduType const * const pdu);

    #endif /* XCP_ENABLE_GET_DAQ_EVENT_INFO */
    #if XCP_ENABLE_FREE_DAQ == XCP_ON

XCP_STATIC void Xcp_FreeDaq_Res(Xcp_PduType const * const pdu);

    #endif /* XCP_ENABLE_FREE_DAQ */
    #if XCP_ENABLE_ALLOC_DAQ == XCP_ON

XCP_STATIC void Xcp_AllocDaq_Res(Xcp_PduType const * const pdu);

    #endif /* XCP_ENABLE_ALLOC_DAQ */
    #if XCP_ENABLE_ALLOC_ODT == XCP_ON

XCP_STATIC void Xcp_AllocOdt_Res(Xcp_PduType const * const pdu);

    #endif /* XCP_ENABLE_ALLOC_ODT */
    #if XCP_ENABLE_ALLOC_ODT_ENTRY == XCP_ON

XCP_STATIC void Xcp_AllocOdtEntry_Res(Xcp_PduType const * const pdu);

    #endif /* XCP_ENABLE_ALLOC_ODT_ENTRY */
    #if XCP_ENABLE_WRITE_DAQ_MULTIPLE == XCP_ON
XCP_STATIC void Xcp_WriteDaqMultiple_Res(Xcp_PduType const * const pdu);
    #endif /* XCP_ENABLE_WRITE_DAQ_MULTIPLE */
#endif     /* XCP_ENABLE_DAQ_COMMANDS */

#if XCP_ENABLE_PGM_COMMANDS == XCP_ON

XCP_STATIC void Xcp_ProgramStart_Res(Xcp_PduType const * const pdu);

XCP_STATIC void Xcp_ProgramClear_Res(Xcp_PduType const * const pdu);

XCP_STATIC void Xcp_Program_Res(Xcp_PduType const * const pdu);

XCP_STATIC void Xcp_ProgramReset_Res(Xcp_PduType const * const pdu);

    #if XCP_ENABLE_GET_PGM_PROCESSOR_INFO == XCP_ON

XCP_STATIC void Xcp_GetPgmProcessorInfo_Res(Xcp_PduType const * const pdu);

    #endif /* XCP_ENABLE_GET_PGM_PROCESSOR_INFO */
    #if XCP_ENABLE_GET_SECTOR_INFO == XCP_ON

XCP_STATIC void Xcp_GetSectorInfo_Res(Xcp_PduType const * const pdu);

    #endif /* XCP_ENABLE_GET_SECTOR_INFO */
    #if XCP_ENABLE_PROGRAM_PREPARE == XCP_ON

XCP_STATIC void Xcp_ProgramPrepare_Res(Xcp_PduType const * const pdu);

    #endif /* XCP_ENABLE_PROGRAM_PREPARE */
    #if XCP_ENABLE_PROGRAM_FORMAT == XCP_ON
XCP_STATIC void Xcp_ProgramFormat_Res(Xcp_PduType const * const pdu);
    #endif /* XCP_ENABLE_PROGRAM_FORMAT */
    #if XCP_ENABLE_PROGRAM_NEXT == XCP_ON
XCP_STATIC void Xcp_ProgramNext_Res(Xcp_PduType const * const pdu);
    #endif /* XCP_ENABLE_PROGRAM_NEXT */
    #if XCP_ENABLE_PROGRAM_MAX == XCP_ON
XCP_STATIC void Xcp_ProgramMax_Res(Xcp_PduType const * const pdu);
    #endif /* XCP_ENABLE_PROGRAM_MAX */
    #if XCP_ENABLE_PROGRAM_VERIFY == XCP_ON
XCP_STATIC void Xcp_ProgramVerify_Res(Xcp_PduType const * const pdu);
    #endif /* XCP_ENABLE_PROGRAM_VERIFY */
#endif     /* XCP_ENABLE_PGM_COMMANDS */

/*
** Big, fat jump table.
*/

XCP_STATIC const Xcp_ServerCommandType Xcp_ServerCommands[] = {
/* lint -save -e632      Assignment to strong type 'Xcp_ServerCommandType'
 * considered harmless in this context. */
#if XCP_ENABLE_STD_COMMANDS == XCP_ON
    Xcp_Connect_Res,
    Xcp_Disconnect_Res,
    Xcp_GetStatus_Res,
    Xcp_Synch_Res,
    #if XCP_ENABLE_GET_COMM_MODE_INFO == XCP_ON
    Xcp_GetCommModeInfo_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_GET_ID == XCP_ON
    Xcp_GetId_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_SET_REQUEST == XCP_ON
    Xcp_SetRequest_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_GET_SEED == XCP_ON
    Xcp_GetSeed_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_UNLOCK == XCP_ON
    Xcp_Unlock_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_SET_MTA == XCP_ON
    Xcp_SetMta_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_UPLOAD == XCP_ON
    Xcp_Upload_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_SHORT_UPLOAD == XCP_ON
    Xcp_ShortUpload_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_BUILD_CHECKSUM == XCP_ON
    Xcp_BuildChecksum_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_TRANSPORT_LAYER_CMD == XCP_ON
    Xcp_TransportLayerCmd_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_USER_CMD == XCP_ON
    Xcp_UserCmd_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
#else
    Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res,
#endif

#if XCP_ENABLE_CAL_COMMANDS == XCP_ON
    Xcp_Download_Res,
    #if XCP_ENABLE_DOWNLOAD_NEXT == XCP_ON
    Xcp_DownloadNext_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_DOWNLOAD_MAX == XCP_ON
    Xcp_DownloadMax_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_SHORT_DOWNLOAD == XCP_ON
    Xcp_ShortDownload_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_MODIFY_BITS == XCP_ON
    Xcp_ModifyBits_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
#else
    Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res,
#endif

#if XCP_ENABLE_PAG_COMMANDS == XCP_ON
    Xcp_SetCalPage_Res,
    Xcp_GetCalPage_Res,
    #if XCP_ENABLE_GET_PAG_PROCESSOR_INFO == XCP_ON
    Xcp_GetPagProcessorInfo_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_GET_SEGMENT_INFO == XCP_ON
    Xcp_GetSegmentInfo_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_GET_PAGE_INFO == XCP_ON
    Xcp_GetPageInfo_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_SET_SEGMENT_MODE == XCP_ON
    Xcp_SetSegmentMode_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_GET_SEGMENT_MODE == XCP_ON
    Xcp_GetSegmentMode_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_COPY_CAL_PAGE == XCP_ON
    Xcp_CopyCalPage_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
#else
    Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res,
#endif

#if XCP_ENABLE_DAQ_COMMANDS == XCP_ON
    Xcp_ClearDaqList_Res,
    Xcp_SetDaqPtr_Res,
    Xcp_WriteDaq_Res,
    Xcp_SetDaqListMode_Res,
    Xcp_GetDaqListMode_Res,
    Xcp_StartStopDaqList_Res,
    Xcp_StartStopSynch_Res,
    #if XCP_ENABLE_GET_DAQ_CLOCK == XCP_ON
    Xcp_GetDaqClock_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_READ_DAQ == XCP_ON
    Xcp_ReadDaq_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_GET_DAQ_PROCESSOR_INFO == XCP_ON
    Xcp_GetDaqProcessorInfo_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_GET_DAQ_RESOLUTION_INFO == XCP_ON
    Xcp_GetDaqResolutionInfo_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_GET_DAQ_LIST_INFO == XCP_ON
    Xcp_GetDaqListInfo_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_GET_DAQ_EVENT_INFO == XCP_ON
    Xcp_GetDaqEventInfo_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_FREE_DAQ == XCP_ON
    Xcp_FreeDaq_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_ALLOC_DAQ == XCP_ON
    Xcp_AllocDaq_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_ALLOC_ODT == XCP_ON
    Xcp_AllocOdt_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_ALLOC_ODT_ENTRY == XCP_ON
    Xcp_AllocOdtEntry_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
#else
    Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res,
#endif

#if XCP_ENABLE_PGM_COMMANDS == XCP_ON
    Xcp_ProgramStart_Res,
    Xcp_ProgramClear_Res,
    Xcp_Program_Res,
    Xcp_ProgramReset_Res,
    #if XCP_ENABLE_GET_PGM_PROCESSOR_INFO == XCP_ON
    Xcp_GetPgmProcessorInfo_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_GET_SECTOR_INFO == XCP_ON
    Xcp_GetSectorInfo_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_PROGRAM_PREPARE == XCP_ON
    Xcp_ProgramPrepare_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_PROGRAM_FORMAT == XCP_ON
    Xcp_ProgramFormat_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_PROGRAM_NEXT == XCP_ON
    Xcp_ProgramNext_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_PROGRAM_MAX == XCP_ON
    Xcp_ProgramMax_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
    #if XCP_ENABLE_PROGRAM_VERIFY == XCP_ON
    Xcp_ProgramVerify_Res,
    #else
    Xcp_CommandNotImplemented_Res,
    #endif
#else
    Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res,
    Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res, Xcp_CommandNotImplemented_Res,
#endif
#if (XCP_ENABLE_DAQ_COMMANDS == XCP_ON) && (XCP_ENABLE_WRITE_DAQ_MULTIPLE == XCP_ON)
    Xcp_WriteDaqMultiple_Res,
#else
    Xcp_CommandNotImplemented_Res,
#endif
#if defined(XCP_ENABLE_TIME_CORRELATION) && (XCP_ENABLE_TIME_CORRELATION == XCP_ON)
    Xcp_TimeCorrelationProperties_Res,
#else
    Xcp_CommandNotImplemented_Res,
#endif
    /* lint -restore */
};

/*
**  Global Functions.
*/
void Xcp_Init(void) {
    Xcp_ConnectionState = XCP_DISCONNECTED;

    XcpHw_Init();
    XcpUtl_MemSet(&Xcp_State, UINT8(0), (uint32_t)sizeof(Xcp_StateType));
    Xcp_State.busy = (bool)XCP_FALSE;

    Xcp_DefaultResourceProtection();

#if XCP_ENABLE_SLAVE_BLOCKMODE == XCP_ON
    Xcp_State.slaveBlockModeState.blockTransferActive = (bool)XCP_FALSE;
    Xcp_State.slaveBlockModeState.remaining           = UINT8(0);
#endif /* XCP_ENABLE_SLAVE_BLOCKMODE */
#if XCP_ENABLE_MASTER_BLOCKMODE == XCP_ON
    Xcp_State.masterBlockModeState.blockTransferActive = (bool)XCP_FALSE;
    Xcp_State.masterBlockModeState.remaining           = UINT8(0);
#endif /* XCP_ENABLE_MASTER_BLOCKMODE */
#if XCP_ENABLE_DAQ_COMMANDS == XCP_ON
    XcpDaq_Init();
#endif /* XCP_ENABLE_DAQ_COMMANDS */
#if XCP_ENABLE_PGM_COMMANDS == XCP_ON
    Xcp_State.pgmProcessor.state = XCP_PGM_STATE_UNINIT;
#endif /* ENABLE_PGM_COMMANDS */
#if XCP_TRANSPORT_LAYER_COUNTER_SIZE != 0
    Xcp_State.counter = (uint16_t)0;
#endif /* XCP_TRANSPORT_LAYER_COUNTER_SIZE */

#if XCP_ENABLE_STATISTICS == XCP_ON
    Xcp_State.statistics.crosBusy     = UINT32(0);
    Xcp_State.statistics.crosSend     = UINT32(0);
    Xcp_State.statistics.ctosReceived = UINT32(0);
#endif /* XCP_ENABLE_STATISTICS */
    XcpTl_Init();

#if defined(XCP_ENABLE_TIME_CORRELATION) && (XCP_ENABLE_TIME_CORRELATION == XCP_ON)
    XcpTimecorr_Init();
#endif /* XCP_ENABLE_TIME_CORRELATION */

#if (XCP_ENABLE_BUILD_CHECKSUM == XCP_ON) && (XCP_CHECKSUM_CHUNKED_CALCULATION == XCP_ON)
    Xcp_ChecksumInit();
#endif /* XCP_ENABLE_BUILD_CHECKSUM */
}

XCP_STATIC void Xcp_DefaultResourceProtection(void) {
#if XCP_ENABLE_RESOURCE_PROTECTION == XCP_ON
    Xcp_State.resourceProtection = UINT8(0);
    Xcp_State.seedRequested      = UINT8(0);
    #if (XCP_PROTECT_CAL == XCP_ON) || (XCP_PROTECT_PAG == XCP_ON)
    Xcp_State.resourceProtection |= XCP_RESOURCE_CAL_PAG;
    #endif /* XCP_PROTECT_CAL */
    #if XCP_PROTECT_DAQ == XCP_ON
    Xcp_State.resourceProtection |= XCP_RESOURCE_DAQ;
    #endif /* XCP_PROTECT_DAQ */
    #if XCP_PROTECT_STIM == XCP_ON
    Xcp_State.resourceProtection |= XCP_RESOURCE_STIM;
    #endif /* XCP_PROTECT_STIM */
    #if XCP_PROTECT_PGM == XCP_ON
    Xcp_State.resourceProtection |= XCP_RESOURCE_PGM;
    #endif /* XCP_PROTECT_PGM */
#endif     /* XCP_ENABLE_RESOURCE_PROTECTION */
}

void Xcp_Disconnect(void) {
    XcpTl_ReleaseConnection();
    Xcp_DefaultResourceProtection();
#if XCP_ENABLE_DAQ_COMMANDS == XCP_ON
    XcpDaq_Init();
#endif /* XCP_ENABLE_DAQ_COMMANDS */
}

void Xcp_MainFunction(void) {
#if XCP_ENABLE_SLAVE_BLOCKMODE == XCP_ON
    if (Xcp_SlaveBlockTransferIsActive()) {
        Xcp_UploadSingleBlock();
    }
#endif /* XCP_ENABLE_SLAVE_BLOCKMODE */

#if (XCP_ENABLE_BUILD_CHECKSUM == XCP_ON) && (XCP_CHECKSUM_CHUNKED_CALCULATION == XCP_ON)
    Xcp_ChecksumMainFunction();
#endif /* (XCP_ENABLE_BUILD_CHECKSUM) && (XCP_CHECKSUM_CHUNKED_CALCULATION ==                                                      \
      XCP_ON) */
    // XcpHw_Sleep(XCP_MAIN_FUNCTION_PERIOD);
}

void Xcp_SetMta(Xcp_MtaType mta) {
    Xcp_State.mta = mta;
}

Xcp_MtaType Xcp_GetNonPagedAddress(void const * const ptr) {
    Xcp_MtaType mta;

    mta.ext     = (uint8_t)0;
    mta.address = (Xcp_PointerSizeType)ptr;
    return mta;
}

void Xcp_SendCto(void) {
#if XCP_TRANSPORT_LAYER_LENGTH_SIZE != 0
    const uint16_t len = Xcp_CtoOut.len;
#endif /* XCP_TRANSPORT_LAYER_LENGTH_SIZE */

#if XCP_TRANSPORT_LAYER_LENGTH_SIZE == 1
    Xcp_CtoOut.data[0] = XCP_LOBYTE(len);
#elif XCP_TRANSPORT_LAYER_LENGTH_SIZE == 2
    Xcp_CtoOut.data[0] = XCP_LOBYTE(len);
    Xcp_CtoOut.data[1] = XCP_HIBYTE(len);
#endif /* XCP_TRANSPORT_LAYER_LENGTH_SIZE */

#if XCP_TRANSPORT_LAYER_COUNTER_SIZE == 1
    Xcp_CtoOut.data[XCP_TRANSPORT_LAYER_LENGTH_SIZE] = XCP_LOBYTE(Xcp_State.counter);
    Xcp_State.counter++;
#elif XCP_TRANSPORT_LAYER_COUNTER_SIZE == 2
    Xcp_CtoOut.data[XCP_TRANSPORT_LAYER_LENGTH_SIZE]     = XCP_LOBYTE(Xcp_State.counter);
    Xcp_CtoOut.data[XCP_TRANSPORT_LAYER_LENGTH_SIZE + 1] = XCP_HIBYTE(Xcp_State.counter);
    Xcp_State.counter++;
#endif /* XCP_TRANSPORT_LAYER_COUNTER_SIZE */

#if XCP_ENABLE_STATISTICS == XCP_ON
    Xcp_State.statistics.crosSend++;
#endif /* XCP_ENABLE_STATISTICS */

    XcpTl_Send(Xcp_CtoOut.data, Xcp_CtoOut.len + (uint16_t)XCP_TRANSPORT_LAYER_BUFFER_OFFSET);
}

#if XCP_ENABLE_DAQ_COMMANDS == XCP_ON

void Xcp_SendDto(void) {
    #if XCP_TRANSPORT_LAYER_LENGTH_SIZE != 0
    const uint16_t len = Xcp_DtoOut.len;
    #endif /* XCP_TRANSPORT_LAYER_LENGTH_SIZE */

    #if XCP_TRANSPORT_LAYER_LENGTH_SIZE == 1
    Xcp_DtoOut.data[0] = XCP_LOBYTE(len);
    #elif XCP_TRANSPORT_LAYER_LENGTH_SIZE == 2
    Xcp_DtoOut.data[0] = XCP_LOBYTE(len);
    Xcp_DtoOut.data[1] = XCP_HIBYTE(len);
    #endif /* XCP_TRANSPORT_LAYER_LENGTH_SIZE */

    #if XCP_TRANSPORT_LAYER_COUNTER_SIZE == 1
    Xcp_DtoOut.data[XCP_TRANSPORT_LAYER_LENGTH_SIZE] = XCP_LOBYTE(Xcp_State.counter);
    Xcp_State.counter++;
    #elif XCP_TRANSPORT_LAYER_COUNTER_SIZE == 2
    Xcp_DtoOut.data[XCP_TRANSPORT_LAYER_LENGTH_SIZE]     = XCP_LOBYTE(Xcp_State.counter);
    Xcp_DtoOut.data[XCP_TRANSPORT_LAYER_LENGTH_SIZE + 1] = XCP_HIBYTE(Xcp_State.counter);
    Xcp_State.counter++;
    #endif /* XCP_TRANSPORT_LAYER_COUNTER_SIZE */

    #if XCP_ENABLE_STATISTICS == XCP_ON
    Xcp_State.statistics.crosSend++;
    #endif /* XCP_ENABLE_STATISTICS */

    XcpTl_Send(Xcp_DtoOut.data, Xcp_DtoOut.len + (uint16_t)XCP_TRANSPORT_LAYER_BUFFER_OFFSET);
}

uint8_t *Xcp_GetDtoOutPtr(void) {
    return &(Xcp_DtoOut.data[XCP_TRANSPORT_LAYER_BUFFER_OFFSET]);
}

void Xcp_SetDtoOutLen(uint16_t len) {
    Xcp_DtoOut.len = len;
}

#endif /* XCP_ENABLE_DAQ_COMMANDS */

uint8_t *Xcp_GetCtoOutPtr(void) {
    return &(Xcp_CtoOut.data[XCP_TRANSPORT_LAYER_BUFFER_OFFSET]);
}

void Xcp_SetCtoOutLen(uint16_t len) {
    Xcp_CtoOut.len = len;
}

void Xcp_Send8(uint8_t len, uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5, uint8_t b6, uint8_t b7) {
    uint8_t *dataOut = Xcp_GetCtoOutPtr();

#if XCP_ON_CAN_MAX_DLC_REQUIRED == XCP_ON
    len = XCP_MAX_CTO;
#endif /* XCP_ON_CAN_MAX_DLC_REQUIRED */

    Xcp_SetCtoOutLen(UINT16(len));

    /* MISRA 2004 violation Rule 15.2               */
    /* Controlled fall-through (copy optimization)  */
    switch (len) {
        case 8:
            dataOut[7] = b7;
        /*lint -fallthrough */
        //[[fallthrough]];
        case 7:
            dataOut[6] = b6;
        /*lint -fallthrough */
        //[[fallthrough]];
        case 6:
            dataOut[5] = b5;
        /*lint -fallthrough */
        //[[fallthrough]];
        case 5:
            dataOut[4] = b4;
        /*lint -fallthrough */
        //[[fallthrough]];
        case 4:
            dataOut[3] = b3;
        /*lint -fallthrough */
        //[[fallthrough]];
        case 3:
            dataOut[2] = b2;
        /*lint -fallthrough */
        //[[fallthrough]];
        case 2:
            dataOut[1] = b1;
        /*lint -fallthrough */
        //[[fallthrough]];
        case 1:
            dataOut[0] = b0;
            break;
        default:
            break;
    }

    Xcp_SendCto();
}

#if XCP_ENABLE_SLAVE_BLOCKMODE == XCP_ON
XCP_STATIC bool Xcp_SlaveBlockTransferIsActive(void) {
    return Xcp_State.slaveBlockModeState.blockTransferActive;
}

XCP_STATIC void Xcp_SlaveBlockTransferSetActive(bool onOff) {
    XCP_ENTER_CRITICAL();
    Xcp_SetBusy(onOff); /* Active slave block-mode also means command processor is
                           busy. */
    Xcp_State.slaveBlockModeState.blockTransferActive = onOff;
    XCP_LEAVE_CRITICAL();
}

void Xcp_UploadSingleBlock(void) {
    uint8_t    *dataOut = XCP_NULL;
    uint8_t     length  = UINT8(0x00);
    Xcp_MtaType dst     = { 0, 0 };

    if (!Xcp_SlaveBlockTransferIsActive()) {
        return;
    }
    dataOut     = Xcp_GetCtoOutPtr();
    dataOut[0]  = (uint8_t)ERR_SUCCESS;
    dst.address = (Xcp_PointerSizeType)(dataOut + 1);
    dst.ext     = (uint8_t)0;

    if (Xcp_State.slaveBlockModeState.remaining <= (XCP_MAX_CTO - 1)) {
        length = Xcp_State.slaveBlockModeState.remaining;

    #if XCP_ON_CAN_MAX_DLC_REQUIRED == XCP_ON
        Xcp_SetCtoOutLen(UINT16(XCP_MAX_CTO));
    #else
        Xcp_SetCtoOutLen(length + UINT16(1));
    #endif /* XCP_ON_CAN_MAX_DLC_REQUIRED */

        // printf("PART BLOCK: %08x LEN% 02x\n", Xcp_State.mta.address, length);
        Xcp_CopyMemory(dst, Xcp_State.mta, (uint32_t)length);
        XCP_INCREMENT_MTA(length);
        Xcp_State.slaveBlockModeState.remaining -= length;
    } else {
        Xcp_SetCtoOutLen(UINT16(XCP_MAX_CTO));
        // printf("FULL BLOCK: %08x\n", Xcp_State.mta.address);
        Xcp_CopyMemory(dst, Xcp_State.mta, (uint32_t)(XCP_MAX_CTO - 1));
        XCP_INCREMENT_MTA((XCP_MAX_CTO - 1));
        Xcp_State.slaveBlockModeState.remaining -= (XCP_MAX_CTO - 1);
    }

    Xcp_SendCto();
    if (Xcp_State.slaveBlockModeState.remaining == UINT8(0)) {
        Xcp_SlaveBlockTransferSetActive((bool)XCP_FALSE);
        // printf("FINISHED.\n");
        // printf("----------------------------------------\n");
    }
}
#endif /* XCP_ENABLE_SLAVE_BLOCKMODE */

XCP_STATIC void Xcp_Upload(uint8_t len) {
    uint8_t *dataOut = Xcp_GetCtoOutPtr();
#if XCP_ENABLE_SLAVE_BLOCKMODE == XCP_OFF
    Xcp_MtaType dst = { 0 };
    dataOut[0]      = (uint8_t)ERR_SUCCESS;
    dst.address     = (Xcp_PointerSizeType)(dataOut + 1);
    dst.ext         = (uint8_t)0;

    Xcp_CopyMemory(dst, Xcp_State.mta, (uint32_t)len);
    XCP_INCREMENT_MTA(len);
    #if XCP_ON_CAN_MAX_DLC_REQUIRED == XCP_ON
    Xcp_SetCtoOutLen(UINT16(XCP_MAX_CTO));
    #else
    Xcp_SetCtoOutLen(len + UINT16(1));
    #endif /* XCP_ON_CAN_MAX_DLC_REQUIRED */
    Xcp_SendCto();
#else
    Xcp_State.slaveBlockModeState.remaining = len;

    // printf("----------------------------------------\n");
    Xcp_SlaveBlockTransferSetActive((bool)XCP_TRUE);
    Xcp_UploadSingleBlock();
#endif /* XCP_ENABLE_SLAVE_BLOCKMODE */
}

/**
 * Entry point, needs to be "wired" to CAN-Rx interrupt.
 *
 * @param pdu
 */

void Xcp_DispatchCommand(Xcp_PduType const * const pdu) {
    const uint8_t cmd = pdu->data[0];
    DBG_TRACE("<- ");

    if (Xcp_State.connected == (bool)XCP_TRUE) {
        /*DBG_PRINT2("CMD: [%02X]\n\r", cmd); */

        if (Xcp_IsBusy()) {
            Xcp_BusyResponse();
            return;
        } else {
#if XCP_ENABLE_STATISTICS == XCP_ON
            Xcp_State.statistics.ctosReceived++;
#endif /* XCP_ENABLE_STATISTICS */
            if (cmd < 0xc6) {
                Xcp_CommandNotImplemented_Res(pdu);
            } else {
                Xcp_ServerCommands[UINT8(0xff) - cmd](pdu);
            }
        }
    } else {
/* not connected. */
#if XCP_ENABLE_STATISTICS == XCP_ON
        Xcp_State.statistics.ctosReceived++;
#endif /* XCP_ENABLE_STATISTICS */
        if (pdu->data[0] == UINT8(XCP_CONNECT)) {
            Xcp_Connect_Res(pdu);
        } else {
        }
    }
#if defined(_MSC_VER)
    fflush(stdout);
#endif
}

void Xcp_SetSendCallout(Xcp_SendCalloutType callout) {
    Xcp_SendCallout = callout;
}

/*
**
** Global Helper Functions.
**
** Note: These functions are only useful for unit-testing and debugging.
**
*/
Xcp_ConnectionStateType Xcp_GetConnectionState(void) {
    return Xcp_ConnectionState;
}

/*
**
** Local Functions.
**
*/
XCP_STATIC void Xcp_CommandNotImplemented_Res(Xcp_PduType const * const pdu) {
    DBG_TRACE("Command not implemented [%02X].\n\r", pdu->data[0]);
    Xcp_ErrorResponse(UINT8(ERR_CMD_UNKNOWN));
}

XCP_STATIC void Xcp_Connect_Res(Xcp_PduType const * const pdu) {
    uint8_t resource      = UINT8(0x00);
    uint8_t commModeBasic = UINT8(0x00);

    DBG_TRACE("CONNECT\n\r");

    if (Xcp_State.connected == (bool)XCP_FALSE) {
        Xcp_State.connected = (bool)XCP_TRUE;
        /* TODO: Init stuff */
    }

#if XCP_ENABLE_PGM_COMMANDS == XCP_ON
    resource |= XCP_RESOURCE_PGM;
#endif /* XCP_ENABLE_PGM_COMMANDS */
#if XCP_ENABLE_DAQ_COMMANDS == XCP_ON
    resource |= XCP_RESOURCE_DAQ;
#endif /* XCP_ENABLE_DAQ_COMMANDS */
#if (XCP_ENABLE_CAL_COMMANDS == XCP_ON) || (XCP_ENABLE_PAG_COMMANDS == XCP_ON)
    resource |= XCP_RESOURCE_CAL_PAG;
#endif
#if XCP_ENABLE_STIM == XCP_ON
    resource |= XCP_RESOURCE_STIM;
#endif /* XCP_ENABLE_STIM */

    commModeBasic |= XCP_BYTE_ORDER;
    commModeBasic |= XCP_ADDRESS_GRANULARITY;
#if XCP_ENABLE_SLAVE_BLOCKMODE == XCP_ON
    commModeBasic |= XCP_SLAVE_BLOCK_MODE;
#endif /* XCP_ENABLE_SLAVE_BLOCKMODE */
#if XCP_ENABLE_GET_COMM_MODE_INFO
    commModeBasic |= XCP_OPTIONAL_COMM_MODE;
#endif /* XCP_ENABLE_GET_COMM_MODE_INFO */

    XcpTl_SaveConnection();

    Xcp_Send8(
        UINT8(8), UINT8(XCP_PACKET_IDENTIFIER_RES), UINT8(resource), UINT8(commModeBasic), UINT8(XCP_MAX_CTO),
        XCP_LOBYTE(XCP_MAX_DTO), XCP_HIBYTE(XCP_MAX_DTO), UINT8(XCP_PROTOCOL_VERSION_MAJOR),
        UINT8(XCP_TRANSPORT_LAYER_VERSION_MAJOR)
    );
    /*DBG_PRINT("MAX-DTO: %04X H: %02X L: %02X\n\r", XCP_MAX_DTO,
     * HIBYTE(XCP_MAX_DTO), LOBYTE(XCP_MAX_DTO)); */
}

XCP_STATIC void Xcp_Disconnect_Res(Xcp_PduType const * const pdu) {
    DBG_TRACE("DISCONNECT\n\r\n\r");

    XCP_ASSERT_PGM_IDLE();
    Xcp_PositiveResponse();
    Xcp_Disconnect();
}

XCP_STATIC void Xcp_GetStatus_Res(Xcp_PduType const * const pdu) {
    DBG_TRACE("GET_STATUS\n\r");

    Xcp_Send8(
        UINT8(6), UINT8(XCP_PACKET_IDENTIFIER_RES), UINT8(0), /* Current session status */
#if XCP_ENABLE_RESOURCE_PROTECTION == XCP_ON
        Xcp_State.resourceProtection, /* Current resource protection status */
#else
        UINT8(0x00), /* Everything is unprotected. */
#endif               /* XCP_ENABLE_RESOURCE_PROTECTION */
        UINT8(0x00), /* Reserved */
        UINT8(0),    /* Session configuration id */
        UINT8(0),    /* "                      " */
        UINT8(0), UINT8(0)
    );
}

XCP_STATIC void Xcp_Synch_Res(Xcp_PduType const * const pdu) {
    DBG_TRACE("SYNCH\n\r");
    Xcp_ErrorResponse(UINT8(ERR_CMD_SYNCH));
}

#if XCP_ENABLE_GET_COMM_MODE_INFO == XCP_ON

XCP_STATIC void Xcp_GetCommModeInfo_Res(Xcp_PduType const * const pdu) {
    uint8_t commModeOptional = UINT8(0);

    DBG_TRACE("GET_COMM_MODE_INFO\n\r");

    #if XCP_ENABLE_MASTER_BLOCKMODE == XCP_ON
    commModeOptional |= XCP_MASTER_BLOCK_MODE;
    #endif /* XCP_ENABLE_MASTER_BLOCKMODE */

    #if XCP_ENABLE_INTERLEAVED_MODE == XCP_ON
    commModeOptional |= XCP_INTERLEAVED_MODE;
    #endif /* XCP_ENABLE_INTERLEAVED_MODE */

    Xcp_Send8(
        UINT8(8), UINT8(XCP_PACKET_IDENTIFIER_RES), UINT8(0), /* Reserved */
        commModeOptional, UINT8(0),                           /* Reserved */
        UINT8(XCP_MAX_BS), UINT8(XCP_MIN_ST), UINT8(XCP_QUEUE_SIZE), UINT8(XCP_DRIVER_VERSION)
    );
}

#endif /* XCP_ENABLE_GET_COMM_MODE_INFO */

#if XCP_ENABLE_GET_ID == XCP_ON

XCP_STATIC void Xcp_GetId_Res(Xcp_PduType const * const pdu) {
    uint8_t               idType       = Xcp_GetByte(pdu, UINT8(1));
    static uint8_t const *response     = XCP_NULL;
    uint32_t              response_len = UINT32(0);
    bool                  valid        = XCP_TRUE;

    DBG_TRACE("GET_ID [type: 0x%02x]\n\r", idType);

    switch (idType) {
        case 0:
            response     = Xcp_GetId0.name;
            response_len = Xcp_GetId0.len;
            break;
        case 1:
            response     = Xcp_GetId1.name;
            response_len = Xcp_GetId1.len;
            break;
    #if XCP_ENABLE_GET_ID_EPK == XCP_ON
        case 5:
            response     = (uint8_t const *)&Xcp_EEPROM_Kennung[0];
            response_len = XcpUtl_StrLen(Xcp_EEPROM_Kennung);
            break;
    #endif /* XCP_ENABLE_GET_ID_EPK */
        default:
    #if XCP_ENABLE_GET_ID_HOOK == XCP_ON
            if (!Xcp_HookFunction_GetId(idType, &response, &response_len)) {
                response_len = 0;
                valid        = XCP_FALSE;
            }
    #else
            response_len = 0;
            valid        = XCP_FALSE;
    #endif /* XCP_ENABLE_GET_ID_HOOK */
            break;
    }
    if (valid) {
        Xcp_SetMta(Xcp_GetNonPagedAddress(response));
    }
    Xcp_Send8(
        UINT8(8), UINT8(XCP_PACKET_IDENTIFIER_RES), UINT8(0), UINT8(0), UINT8(0), XCP_LOBYTE(XCP_LOWORD(response_len)),
        XCP_HIBYTE(XCP_LOWORD(response_len)), XCP_LOBYTE(XCP_HIWORD(response_len)), XCP_HIBYTE(XCP_HIWORD(response_len))
    );
}

#endif /* XCP_ENABLE_GET_ID */

#if XCP_ENABLE_GET_SEED == XCP_ON
XCP_STATIC void Xcp_GetSeed_Res(Xcp_PduType const * const pdu) {
    uint8_t         mode     = Xcp_GetByte(pdu, UINT8(1));
    uint8_t         resource = Xcp_GetByte(pdu, UINT8(2));
    Xcp_1DArrayType seed     = { 0 };
    uint8_t         length   = UINT8(0);
    uint8_t        *dataOut  = Xcp_GetCtoOutPtr();

    DBG_TRACE("GET_SEED [mode: %02x resource: %02x]\n\r", mode, resource);

    XCP_ASSERT_PGM_IDLE();
    switch (resource) {
        case XCP_RESOURCE_PGM:
    #if XCP_ENABLE_PGM_COMMANDS == XCP_OFF
            Xcp_ErrorResponse(UINT8(ERR_OUT_OF_RANGE));
            return;
    #else
            break;
    #endif /* XCP_ENABLE_PGM_COMMANDS */
        case XCP_RESOURCE_STIM:

    #if XCP_ENABLE_STIM == XCP_OFF
            Xcp_ErrorResponse(UINT8(ERR_OUT_OF_RANGE));
            return;
    #else
            break;
    #endif /* XCP_ENABLE_STIM */
        case XCP_RESOURCE_DAQ:

    #if XCP_ENABLE_DAQ_COMMANDS == XCP_OFF
            Xcp_ErrorResponse(UINT8(ERR_OUT_OF_RANGE));
            return;
    #else
            break;
    #endif /* XCP_ENABLE_DAQ_COMMANDS */
        case XCP_RESOURCE_CAL_PAG:

    #if (XCP_ENABLE_CAL_COMMANDS == XCP_OFF) && (XCP_ENABLE_PAG_COMMANDS == XCP_OFF)
            Xcp_ErrorResponse(UINT8(ERR_OUT_OF_RANGE));
            return;
    #else
            break;
    #endif /* XCP_ENABLE_CAL_COMMANDS */
        default:
            Xcp_ErrorResponse(UINT8(ERR_OUT_OF_RANGE));
            return;
    }

    if (Xcp_IsProtected(resource)) {
        Xcp_HookFunction_GetSeed(resource, &seed); /* User supplied callout. */
        length = XCP_MIN(XCP_MAX_CTO - UINT16(2), seed.length);
        XCP_ASSERT_LE(length, XCP_TRANSPORT_LAYER_CTO_BUFFER_SIZE - 2);
        XcpUtl_MemCopy(dataOut + UINT16(2), seed.data, (uint32_t)length);
    } else {
        /* Resource already unlocked. */
        length = UINT8(0);
    }
    Xcp_State.seedRequested |= resource;
    dataOut[0] = UINT8(ERR_SUCCESS);
    dataOut[1] = length;
    #if XCP_ON_CAN_MAX_DLC_REQUIRED == XCP_ON
    Xcp_SetCtoOutLen(UINT16(XCP_MAX_CTO));
    #else
    Xcp_SetCtoOutLen(length + UINT16(2));
    #endif /* XCP_ON_CAN_MAX_DLC_REQUIRED */
    Xcp_SendCto();
}
#endif /* XCP_ENABLE_GET_SEED */

#if XCP_ENABLE_UNLOCK == XCP_ON
XCP_STATIC void Xcp_Unlock_Res(Xcp_PduType const * const pdu) {
    uint8_t         length = Xcp_GetByte(pdu, UINT8(1));
    Xcp_1DArrayType key    = { 0 };

    DBG_TRACE("UNLOCK [length: %u]\n\r", length);

    XCP_ASSERT_PGM_IDLE();
    if (Xcp_State.seedRequested == UINT8(0)) {
        Xcp_ErrorResponse(UINT8(ERR_SEQUENCE));
        return;
    }

    key.length = length;
    key.data   = pdu->data + 2;

    if (Xcp_HookFunction_Unlock(Xcp_State.seedRequested, &key)) {
        /* User supplied callout. */
        Xcp_State.resourceProtection &= UINT8(~(Xcp_State.seedRequested)); /* OK, unlock. */
        Xcp_Send8(
            UINT8(2), UINT8(XCP_PACKET_IDENTIFIER_RES), Xcp_State.resourceProtection, /* Current resource protection status. */
            UINT8(0), UINT8(0), UINT8(0), UINT8(0), UINT8(0), UINT8(0)
        );
    } else {
        Xcp_ErrorResponse(UINT8(ERR_ACCESS_LOCKED));
        Xcp_Disconnect();
    }
    Xcp_State.seedRequested = UINT8(0x00);
}
#endif /* XCP_ENABLE_UNLOCK */

#if XCP_ENABLE_UPLOAD == XCP_ON

XCP_STATIC void Xcp_Upload_Res(Xcp_PduType const * const pdu) {
    uint8_t len = Xcp_GetByte(pdu, UINT8(1));

    DBG_TRACE("UPLOAD [len: %u]\n\r", len);

    XCP_ASSERT_PGM_IDLE();
    XCP_CHECK_MEMORY_ACCESS(Xcp_State.mta, len, XCP_MEM_ACCESS_READ, XCP_FALSE);
    #if XCP_ENABLE_SLAVE_BLOCKMODE == XCP_OFF
    if (len > UINT8(XCP_MAX_CTO - 1)) {
        Xcp_ErrorResponse(UINT8(ERR_OUT_OF_RANGE));
        return;
    }
    #else

    #endif /* XCP_ENABLE_SLAVE_BLOCKMODE */

    Xcp_Upload(len);
}

#endif /* XCP_ENABLE_UPLOAD */

#if XCP_ENABLE_SHORT_UPLOAD == XCP_ON

XCP_STATIC void Xcp_ShortUpload_Res(Xcp_PduType const * const pdu) {
    uint8_t len = Xcp_GetByte(pdu, UINT8(1));

    DBG_TRACE("SHORT-UPLOAD [len: %u]\n\r", len);

    XCP_ASSERT_PGM_IDLE();
    XCP_CHECK_MEMORY_ACCESS(Xcp_State.mta, len, XCP_MEM_ACCESS_READ, (bool)XCP_FALSE);
    if (len > UINT8(XCP_MAX_CTO - 1)) {
        Xcp_ErrorResponse(UINT8(ERR_OUT_OF_RANGE));
        return;
    }

    Xcp_State.mta.ext     = Xcp_GetByte(pdu, UINT8(3));
    Xcp_State.mta.address = Xcp_GetDWord(pdu, UINT8(4));
    Xcp_Upload(len);
}

#endif /* XCP_ENABLE_SHORT_UPLOAD */

#if XCP_ENABLE_SET_MTA == XCP_ON

XCP_STATIC void Xcp_SetMta_Res(Xcp_PduType const * const pdu) {
    Xcp_State.mta.ext     = Xcp_GetByte(pdu, UINT8(3));
    Xcp_State.mta.address = Xcp_GetDWord(pdu, UINT8(4));

    DBG_TRACE("SET_MTA [address: 0x%" PRIxPTR " ext: 0x%02x]\n\r", (uintptr_t)Xcp_State.mta.address, Xcp_State.mta.ext);

    Xcp_PositiveResponse();
}

#endif /* XCP_ENABLE_SET_MTA */

#if XCP_ENABLE_BUILD_CHECKSUM == XCP_ON

void Xcp_SendChecksumPositiveResponse(Xcp_ChecksumType checksum) {
    Xcp_Send8(
        UINT8(8), UINT8(XCP_PACKET_IDENTIFIER_RES), UINT8(XCP_CHECKSUM_METHOD), UINT8(0), UINT8(0),
        XCP_LOBYTE(XCP_LOWORD(checksum)), XCP_HIBYTE(XCP_LOWORD(checksum)), XCP_LOBYTE(XCP_HIWORD(checksum)),
        XCP_HIBYTE(XCP_HIWORD(checksum))
    );
}

void Xcp_SendChecksumOutOfRangeResponse(void) {
    Xcp_Send8(
        UINT8(8), UINT8(XCP_PACKET_IDENTIFIER_ERR), UINT8(ERR_OUT_OF_RANGE), UINT8(0), UINT8(0),
        XCP_LOBYTE(XCP_LOWORD(XCP_CHECKSUM_MAXIMUM_BLOCK_SIZE)), XCP_HIBYTE(XCP_LOWORD(XCP_CHECKSUM_MAXIMUM_BLOCK_SIZE)),
        XCP_LOBYTE(XCP_HIWORD(XCP_CHECKSUM_MAXIMUM_BLOCK_SIZE)), XCP_HIBYTE(XCP_HIWORD(XCP_CHECKSUM_MAXIMUM_BLOCK_SIZE))
    );
}

XCP_STATIC void Xcp_BuildChecksum_Res(Xcp_PduType const * const pdu) {
    uint32_t         blockSize = Xcp_GetDWord(pdu, UINT8(4));
    Xcp_ChecksumType checksum  = (Xcp_ChecksumType)0;
    uint8_t const   *ptr       = XCP_NULL;

    DBG_TRACE("BUILD_CHECKSUM [blocksize: %u]\n\r", blockSize);

    XCP_ASSERT_PGM_IDLE();
    XCP_CHECK_MEMORY_ACCESS(Xcp_State.mta, blockSize, XCP_MEM_ACCESS_READ, (bool)XCP_FALSE);
    #if XCP_CHECKSUM_MAXIMUM_BLOCK_SIZE > 0
    /* We need to range check. */
    if (blockSize > UINT32(XCP_CHECKSUM_MAXIMUM_BLOCK_SIZE)) {
        Xcp_SendChecksumOutOfRangeResponse();
        return;
    }
    #endif

    ptr = (uint8_t const *)Xcp_State.mta.address;
    /* The MTA will be post-incremented by the block size. */

    #if XCP_CHECKSUM_CHUNKED_CALCULATION == XCP_OFF
    checksum = Xcp_CalculateChecksum(ptr, blockSize, (Xcp_ChecksumType)0, XCP_TRUE);
    Xcp_SendChecksumPositiveResponse(checksum);
    #else
    if (blockSize <= UINT32(XCP_CHECKSUM_CHUNK_SIZE)) {
        checksum = Xcp_CalculateChecksum(ptr, blockSize, (Xcp_ChecksumType)0, (bool)XCP_TRUE);
        Xcp_SendChecksumPositiveResponse(checksum);
    } else {
        Xcp_StartChecksumCalculation(ptr, blockSize);
    }
    #endif /* XCP_CHECKSUM_CHUNKED_CALCULATION */
}

#endif /* XCP_ENABLE_BUILD_CHECKSUM */

#if XCP_ENABLE_TRANSPORT_LAYER_CMD
XCP_STATIC void Xcp_TransportLayerCmd_Res(Xcp_PduType const * const pdu) {
    XCP_ASSERT_PGM_IDLE();
    XcpTl_TransportLayerCmd_Res(pdu);
}
#endif /* XCP_ENABLE_TRANSPORT_LAYER_CMD */

#if XCP_ENABLE_USER_CMD
XCP_STATIC void Xcp_UserCmd_Res(Xcp_PduType const * const pdu) {
    XCP_ASSERT_PGM_IDLE();
}
#endif /* XCP_ENABLE_USER_CMD */

/*
**
**  CAL Commands.
**
*/
#if XCP_ENABLE_CAL_COMMANDS == XCP_ON

XCP_STATIC void Xcp_Download_Res(Xcp_PduType const * const pdu) {
    uint8_t        len   = Xcp_GetByte(pdu, UINT8(1));
    const uint16_t LIMIT = XCP_MAX_BS * XCP_DOWNLOAD_PAYLOAD_LENGTH;
    DBG_TRACE("DOWNLOAD [len: %u]\n\r", len);

    XCP_ASSERT_PGM_IDLE();
    XCP_CHECK_MEMORY_ACCESS(Xcp_State.mta, len, XCP_MEM_ACCESS_WRITE, XCP_FALSE);

    #if XCP_ENABLE_MASTER_BLOCKMODE == XCP_ON
    Xcp_State.masterBlockModeState.blockTransferActive = (bool)XCP_FALSE;
    if (len > LIMIT) {
        Xcp_ErrorResponse(ERR_OUT_OF_RANGE); /* Request exceeds max. block size. */
        return;
    } else if (len > XCP_DOWNLOAD_PAYLOAD_LENGTH) {
        /* OK, regular first-frame transfer. */
        Xcp_State.masterBlockModeState.blockTransferActive = (bool)XCP_TRUE;
        Xcp_State.masterBlockModeState.remaining           = len - XCP_DOWNLOAD_PAYLOAD_LENGTH;
        Xcp_Download_Copy((Xcp_PointerSizeType)(uintptr_t)(pdu->data + 2), UINT8(0), UINT32(XCP_DOWNLOAD_PAYLOAD_LENGTH));
        return;
    }
    #endif /* XCP_ENABLE_MASTER_BLOCKMODE */

    #if XCP_ENABLE_MASTER_BLOCKMODE == XCP_OFF
    if (len > XCP_DOWNLOAD_PAYLOAD_LENGTH) {
        Xcp_ErrorResponse(ERR_OUT_OF_RANGE); /* Request exceeds max. payload size. */
        return;
    }
    #endif
    Xcp_Download_Copy((Xcp_PointerSizeType)(uintptr_t)(pdu->data + 2), UINT8(0), UINT32(len));
    Xcp_PositiveResponse();
}

    #if XCP_ENABLE_DOWNLOAD_NEXT == XCP_ON
        #if XCP_ENABLE_MASTER_BLOCKMODE == XCP_ON
XCP_STATIC void Xcp_DownloadNext_Res(Xcp_PduType const * const pdu) {
    uint8_t     remaining = Xcp_GetByte(pdu, UINT8(1));
    uint8_t     len;
    Xcp_MtaType src = { 0 };

    DBG_TRACE("DOWNLOAD_NEXT [remaining: %u]\n\r", remaining);

    XCP_ASSERT_PGM_IDLE();
    if (!Xcp_State.masterBlockModeState.blockTransferActive) {
        Xcp_ErrorResponse(ERR_SEQUENCE); /* Check: Is it really necessary to start a block-mode
                                            transfer with Xcp_Download_Res? */
        return;
    }
    len = XCP_MIN(remaining, XCP_DOWNLOAD_PAYLOAD_LENGTH);
    Xcp_Download_Copy((Xcp_PointerSizeType)(uintptr_t)(pdu->data + 2), UINT8(0), UINT32(len));
    Xcp_State.masterBlockModeState.remaining -= len;
    if (Xcp_State.masterBlockModeState.remaining == UINT8(0)) {
        Xcp_State.masterBlockModeState.blockTransferActive = (bool)XCP_FALSE;
        Xcp_PositiveResponse();
    }
}
        #endif /* XCP_ENABLE_MASTER_BLOCKMODE */
    #endif     /* XCP_ENABLE_DOWNLOAD_NEXT */

    #if XCP_ENABLE_DOWNLOAD_MAX == XCP_ON

XCP_STATIC void Xcp_DownloadMax_Res(Xcp_PduType const * const pdu) {
    Xcp_MtaType src = { 0 };

    DBG_TRACE("DOWNLOAD_MAX\n\r");

    XCP_ASSERT_PGM_IDLE();
    Xcp_Download_Copy((Xcp_PointerSizeType)(uintptr_t)(pdu->data + 1), UINT8(0), UINT32(XCP_DOWNLOAD_PAYLOAD_LENGTH + 1));
    Xcp_PositiveResponse();
}

    #endif /* XCP_ENABLE_DOWNLOAD_MAX */

    #if XCP_ENABLE_SHORT_DOWNLOAD == XCP_ON

XCP_STATIC void Xcp_ShortDownload_Res(Xcp_PduType const * const pdu) {
    uint8_t     len     = Xcp_GetByte(pdu, UINT8(1));
    uint8_t     addrExt = Xcp_GetByte(pdu, UINT8(3));
    uint32_t    address = Xcp_GetDWord(pdu, UINT8(4));
    Xcp_MtaType src     = { 0 };
    Xcp_MtaType dst     = { 0 };

    DBG_TRACE("SHORT-DOWNLOAD [len: %u address: 0x%08x ext: 0x%02x]\n\r", len, address, addrExt);
    dst.address = address;
    dst.ext     = addrExt;

    XCP_ASSERT_PGM_IDLE();
    XCP_CHECK_MEMORY_ACCESS(dst, len, XCP_MEM_ACCESS_WRITE, (bool)XCP_FALSE);
    if (len > (XCP_MAX_CTO - UINT8(8))) {
        Xcp_ErrorResponse(UINT8(ERR_OUT_OF_RANGE));
        return;
    }

    src.address = (Xcp_PointerSizeType)pdu->data + 8;
    src.ext     = UINT8(0);
    Xcp_CopyMemory(dst, src, (uint32_t)len);

    XCP_INCREMENT_MTA(len);

    Xcp_PositiveResponse();
}

    #endif /* XCP_ENABLE_SHORT_DOWNLOAD */

    #if XCP_ENABLE_MODIFY_BITS == XCP_ON

XCP_STATIC void Xcp_ModifyBits_Res(Xcp_PduType const * const pdu) {
    uint8_t   shiftValue = Xcp_GetByte(pdu, UINT8(1));
    uint16_t  andMask    = Xcp_GetWord(pdu, UINT8(2));
    uint16_t  xorMask    = Xcp_GetWord(pdu, UINT8(4));
    uint32_t *vp         = XCP_NULL;

    DBG_TRACE(
        "MODIFY-BITS [shiftValue: 0x%02X andMask: 0x%04x ext: xorMask: "
        "0x%04x]\n\r",
        shiftValue, andMask, xorMask
    );
    XCP_CHECK_MEMORY_ACCESS(Xcp_State.mta, 2, XCP_MEM_ACCESS_WRITE, (bool)XCP_FALSE);
    vp  = (uint32_t *)Xcp_State.mta.address;
    *vp = ((*vp) & ((~((uint32_t)(((uint16_t)~andMask) << shiftValue))) ^ ((uint32_t)(xorMask << shiftValue))));

    Xcp_PositiveResponse();
}

    #endif /* XCP_ENABLE_MODIFY_BITS */

#endif /* XCP_ENABLE_CAL_COMMANDS */

/*
**
**  DAQ Commands.
**
*/
#if XCP_ENABLE_DAQ_COMMANDS == XCP_ON

XCP_STATIC void Xcp_ClearDaqList_Res(Xcp_PduType const * const pdu) {
    XcpDaq_ListIntegerType daqListNumber = (XcpDaq_ListIntegerType)0;

    daqListNumber = (XcpDaq_ListIntegerType)Xcp_GetWord(pdu, UINT8(2));

    DBG_TRACE("CLEAR_DAQ_LIST [daq: %u] \n\r", daqListNumber);

    XCP_ASSERT_PGM_IDLE();
    XCP_ASSERT_UNLOCKED(XCP_RESOURCE_DAQ);
    Xcp_PositiveResponse();
}

XCP_STATIC void Xcp_SetDaqPtr_Res(Xcp_PduType const * const pdu) {
    XcpDaq_ListIntegerType     daqList  = (XcpDaq_ListIntegerType)0;
    XcpDaq_ODTIntegerType      odt      = (XcpDaq_ODTIntegerType)0;
    XcpDaq_ODTEntryIntegerType odtEntry = (XcpDaq_ODTEntryIntegerType)0;

    XCP_ASSERT_DAQ_STOPPED();
    XCP_ASSERT_PGM_IDLE();
    XCP_ASSERT_UNLOCKED(XCP_RESOURCE_DAQ);
    daqList  = (XcpDaq_ListIntegerType)Xcp_GetWord(pdu, UINT8(2));
    odt      = Xcp_GetByte(pdu, UINT8(4));
    odtEntry = Xcp_GetByte(pdu, UINT8(5));

    /* TODO: Check for Predef. lists only */

    if (!XcpDaq_ValidateOdtEntry(daqList, odt, odtEntry)) {
        /* If the specified list is not available, ERR_OUT_OF_RANGE will be
         * returned. */
        Xcp_ErrorResponse(UINT8(ERR_OUT_OF_RANGE));
        return;
    }

    XcpDaq_SetPointer(daqList, odt, odtEntry);

    DBG_TRACE(
        "SET_DAQ_PTR [daq: %u odt: %u odtEntry: %u]\n\r", Xcp_State.daqPointer.daqList, Xcp_State.daqPointer.odt,
        Xcp_State.daqPointer.odtEntry
    );

    Xcp_PositiveResponse();
}

    #if XCP_ENABLE_READ_DAQ == XCP_ON
XCP_STATIC void Xcp_ReadDaq_Res(Xcp_PduType const * const pdu) {
    uint8_t             bitOffset;
    uint8_t             elemSize;
    uint8_t             adddrExt;
    Xcp_PointerSizeType address;

    XcpDaq_ReadEntry(&bitOffset, &elemSize, &adddrExt, &address);

    Xcp_Send8(
        UINT8(8), UINT8(XCP_PACKET_IDENTIFIER_RES), UINT8(bitOffset), UINT8(elemSize), UINT8(adddrExt),
        XCP_LOBYTE(XCP_LOWORD(address)), XCP_HIBYTE(XCP_LOWORD(address)), XCP_LOBYTE(XCP_HIWORD(address)),
        XCP_HIBYTE(XCP_HIWORD(address))
    );
}
    #endif /* XCP_ENABLE_READ_DAQ */

XCP_STATIC void Xcp_WriteDaq_Res(Xcp_PduType const * const pdu) {
    const uint8_t  bitOffset = Xcp_GetByte(pdu, UINT8(1));
    const uint8_t  elemSize  = Xcp_GetByte(pdu, UINT8(2));
    const uint8_t  adddrExt  = Xcp_GetByte(pdu, UINT8(3));
    const uint32_t address   = Xcp_GetDWord(pdu, UINT8(4));

    DBG_TRACE("WRITE_DAQ\n\r");

    XCP_ASSERT_DAQ_STOPPED();
    XCP_ASSERT_PGM_IDLE();
    XCP_ASSERT_UNLOCKED(XCP_RESOURCE_DAQ);
    #if XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_ON
    /* WRITE_DAQ is only possible for elements in configurable DAQ lists. */
    if (Xcp_State.daqPointer.daqList < XcpDaq_PredefinedListCount) {
        Xcp_SendResult(ERR_WRITE_PROTECTED);
        return;
    }
    #endif /* XCP_DAQ_ENABLE_PREDEFINED_LISTS */
    XcpDaq_WriteEntry(bitOffset, elemSize, adddrExt, address);
    Xcp_PositiveResponse();
}

    #if XCP_ENABLE_WRITE_DAQ_MULTIPLE == XCP_ON
XCP_STATIC void Xcp_WriteDaqMultiple_Res(Xcp_PduType const * const pdu) {
    uint8_t  numElements = Xcp_GetByte(pdu, UINT8(1));
    uint8_t  bitOffset   = UINT8(0);
    uint8_t  elemSize    = UINT8(0);
    uint8_t  adddrExt    = UINT8(0);
    uint32_t address     = UINT32(0);
    uint8_t  daq_offset  = UINT8(0);
    uint8_t  idx         = UINT8(0);

    if (((numElements * UINT8(8)) + UINT8(2)) > XCP_MAX_CTO) {
        // To many ODT entries to fit into one XCP frame.
        Xcp_SendResult(ERR_OUT_OF_RANGE);
        return;
    }
    DBG_TRACE("WRITE_DAQ_MULTIPLE [numElements: 0x%08x]\n\r", numElements);
    XCP_ASSERT_DAQ_STOPPED();
    XCP_ASSERT_PGM_IDLE();
    XCP_ASSERT_UNLOCKED(XCP_RESOURCE_DAQ);
        #if XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_ON
    /* WRITE_DAQ is only possible for elements in configurable DAQ lists. */
    if (Xcp_State.daqPointer.daqList < XcpDaq_PredefinedListCount) {
        Xcp_SendResult(ERR_WRITE_PROTECTED);
        return;
    }
        #endif /* XCP_DAQ_ENABLE_PREDEFINED_LISTS */
    for (idx = UINT8(0); idx < numElements; ++idx) {
        daq_offset = (idx * UINT8(8)) + UINT8(2);
        bitOffset  = Xcp_GetByte(pdu, daq_offset + UINT8(0));
        elemSize   = Xcp_GetByte(pdu, daq_offset + UINT8(1));
        address    = Xcp_GetDWord(pdu, daq_offset + UINT8(2));
        adddrExt   = Xcp_GetByte(pdu, daq_offset + UINT8(6));
        XcpDaq_WriteEntry(bitOffset, elemSize, adddrExt, address);
    }
    Xcp_PositiveResponse();
}
    #endif /* XCP_ENABLE_WRITE_DAQ_MULTIPLE */

XCP_STATIC void Xcp_SetDaqListMode_Res(Xcp_PduType const * const pdu) {
    const uint8_t                mode               = Xcp_GetByte(pdu, UINT8(1));
    const XcpDaq_ListIntegerType daqListNumber      = (XcpDaq_ListIntegerType)Xcp_GetWord(pdu, UINT8(2));
    const uint16_t               eventChannelNumber = Xcp_GetWord(pdu, UINT8(4));
    const uint8_t                prescaler          = Xcp_GetByte(pdu, UINT8(6));
    const uint8_t                priority           = Xcp_GetByte(pdu, UINT8(7));

    DBG_TRACE(
        "SET_DAQ_LIST_MODE [mode: 0x%02x daq: %03u event: %03u prescaler: "
        "%03u priority: %03u]\n\r",
        mode, daqListNumber, eventChannelNumber, prescaler, priority
    );
    XCP_ASSERT_DAQ_STOPPED();
    XCP_ASSERT_PGM_IDLE();
    XCP_ASSERT_UNLOCKED(XCP_RESOURCE_DAQ);

    /* Validate DAQ list index against configured lists to avoid config-dependent faults */
    if (daqListNumber >= XcpDaq_GetListCount()) {
        Xcp_ErrorResponse(UINT8(ERR_OUT_OF_RANGE));
        return;
    }

    #if XCP_ENABLE_STIM == XCP_OFF
    if ((mode & XCP_DAQ_LIST_MODE_DIRECTION) == XCP_DAQ_LIST_MODE_DIRECTION) {
        Xcp_ErrorResponse(UINT8(ERR_CMD_SYNTAX));
        return;
    }
    #endif /* XCP_ENABLE_STIM */
    /*
    The master is not allowed to set the ALTERNATING flag and the TIMESTAMP flag at
    the same time.
    */
    #if XCP_DAQ_ALTERNATING_SUPPORTED == XCP_OFF
    if ((mode & XCP_DAQ_LIST_MODE_ALTERNATING) == XCP_DAQ_LIST_MODE_ALTERNATING) {
        Xcp_ErrorResponse(UINT8(ERR_CMD_SYNTAX));
        return;
    }
    #endif /* XCP_DAQ_ENABLE_ALTERNATING */
    #if XCP_DAQ_PRIORITIZATION_SUPPORTED == XCP_OFF
        /* Needs to be 0 */

        #if 0  // Ignored for now.
    if (priority > UINT8(0)) {
        Xcp_ErrorResponse(UINT8(ERR_OUT_OF_RANGE));
        return;
    }
        #endif

    #endif /* XCP_DAQ_ENABLE_PRIORITIZATION */
    #if XCP_DAQ_PRESCALER_SUPPORTED == XCP_OFF
    /* Needs to be 1 */
    if (prescaler > UINT8(1)) {
        Xcp_ErrorResponse(UINT8(ERR_OUT_OF_RANGE));
        return;
    }
    #endif /* XCP_DAQ_ENABLE_PRESCALER */

    XcpDaq_SetListMode(daqListNumber, mode, eventChannelNumber, prescaler, priority);

    Xcp_PositiveResponse();
}

XCP_STATIC void Xcp_GetDaqListMode_Res(Xcp_PduType const * const pdu) {
    XcpDaq_ListStateType        *entry         = XCP_NULL;
    const XcpDaq_ListIntegerType daqListNumber = (XcpDaq_ListIntegerType)Xcp_GetWord(pdu, UINT8(2));

    DBG_TRACE("GET_DAQ_LIST_MODE\n\r");
    XCP_ASSERT_PGM_IDLE();
    XCP_ASSERT_UNLOCKED(XCP_RESOURCE_DAQ);

    if (daqListNumber >= XcpDaq_GetListCount()) {
        Xcp_ErrorResponse(UINT8(ERR_OUT_OF_RANGE));
        return;
    }

    entry = XcpDaq_GetListState(daqListNumber);

    Xcp_Send8(
        UINT8(8), UINT8(XCP_PACKET_IDENTIFIER_RES), UINT8(entry->mode), UINT8(0), UINT8(0),

        // XCP_LOBYTE(UINT16(XCP_DAQ_MAX_EVENT_CHANNEL)), XCP_HIBYTE(UINT16(XCP_DAQ_MAX_EVENT_CHANNEL)),

        UINT8(0),  // WORD    Current Event Channel Number
        UINT8(0),
        UINT8(1),  // BYTE    Current Prescaler
        UINT8(0)   // BYTE    Current DAQ list Priority
    );
}

XCP_STATIC void Xcp_StartStopDaqList_Res(Xcp_PduType const * const pdu) {
    const uint8_t                mode          = Xcp_GetByte(pdu, UINT8(1));
    XcpDaq_ODTIntegerType        firstPid      = (XcpDaq_ODTIntegerType)0;
    const XcpDaq_ListIntegerType daqListNumber = (XcpDaq_ListIntegerType)Xcp_GetWord(pdu, UINT8(2));

    DBG_TRACE("START_STOP_DAQ_LIST [mode: 0x%02x daq: %03u]\n\r", mode, daqListNumber);
    XCP_ASSERT_PGM_IDLE();
    XCP_ASSERT_UNLOCKED(XCP_RESOURCE_DAQ);

    if (daqListNumber >= XcpDaq_GetListCount()) {
        Xcp_ErrorResponse(UINT8(ERR_OUT_OF_RANGE));
        return;
    }

    if (mode > 2) {
        Xcp_ErrorResponse(UINT8(ERR_MODE_NOT_VALID));
        return;
    }

    XcpDaq_StartStopSingleList(daqListNumber, mode);
    XcpDaq_GetFirstPid(daqListNumber, &firstPid);

    Xcp_Send8(UINT8(8), UINT8(XCP_PACKET_IDENTIFIER_RES), firstPid, UINT8(0), UINT8(0), UINT8(0), UINT8(0), UINT8(0), UINT8(0));
}

XCP_STATIC void Xcp_StartStopSynch_Res(Xcp_PduType const * const pdu) {
    const uint8_t mode = Xcp_GetByte(pdu, UINT8(1));

    DBG_TRACE("START_STOP_SYNCH [mode: 0x%02x]\n\r", mode);
    XCP_ASSERT_PGM_IDLE();
    XCP_ASSERT_UNLOCKED(XCP_RESOURCE_DAQ);

    if (mode > 3) {
        Xcp_ErrorResponse(UINT8(ERR_MODE_NOT_VALID));
        return;
    }
    if ((mode == 3) || (mode == 2)) {
        XcpDaq_StartStopSynch(2);
    } else {
        XcpDaq_StartStopSynch(mode);
    }

    Xcp_PositiveResponse();
}

    #if XCP_ENABLE_GET_DAQ_PROCESSOR_INFO == XCP_ON

XCP_STATIC void Xcp_GetDaqProcessorInfo_Res(Xcp_PduType const * const pdu) {
    uint8_t                properties = UINT8(0);
    XcpDaq_ListIntegerType listCount  = (XcpDaq_ListIntegerType)0;

    DBG_TRACE("GET_DAQ_PROCESSOR_INFO\n\r");
    XCP_ASSERT_PGM_IDLE();
    XCP_ASSERT_UNLOCKED(XCP_RESOURCE_DAQ);

    XcpDaq_GetProperties(&properties);
    listCount = XcpDaq_GetListCount();

    Xcp_Send8(
        UINT8(8), UINT8(XCP_PACKET_IDENTIFIER_RES), properties, XCP_LOBYTE(listCount), XCP_HIBYTE(listCount),
        XCP_LOBYTE(UINT16(XCP_DAQ_MAX_EVENT_CHANNEL)), XCP_HIBYTE(UINT16(XCP_DAQ_MAX_EVENT_CHANNEL)),
        #if XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_ON
        XcpDaq_PredefinedListCount,
        #else
        UINT8(0),
        #endif /* XCP_DAQ_ENABLE_PREDEFINED_LISTS */
        UINT8(0)
    );
}

    #endif /* XCP_ENABLE_GET_DAQ_PROCESSOR_INFO */

    #if XCP_ENABLE_GET_DAQ_LIST_INFO == XCP_ON

XCP_STATIC void Xcp_GetDaqListInfo_Res(Xcp_PduType const * const pdu) {
    const XcpDaq_ListIntegerType daqListNumber = (XcpDaq_ListIntegerType)Xcp_GetWord(pdu, UINT8(2));
        #if XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_ON
    XcpDaq_ListConfigurationType const *listConf = XCP_NULL;
        #endif
    /*    XcpDaq_ListStateType * listState = XCP_NULL;*/
    uint8_t properties = UINT8(0x00);

    DBG_TRACE("GET_DAQ_LIST_INFO [daq: %u] \n\r", daqListNumber);
    XCP_ASSERT_PGM_IDLE();
    XCP_ASSERT_UNLOCKED(XCP_RESOURCE_DAQ);

    if (daqListNumber >= XcpDaq_GetListCount()) {
        Xcp_ErrorResponse(UINT8(ERR_OUT_OF_RANGE));
    }
        #if XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_ON
    listConf = XcpDaq_GetListConfiguration(daqListNumber);
        #endif

    properties |= DAQ_LIST_PROPERTY_PREDEFINED;  /* Hardcoded for now. */
    properties |= DAQ_LIST_PROPERTY_EVENT_FIXED; /* "                " */
    properties |= DAQ_LIST_PROPERTY_DAQ;         /* "                " */

        #if XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_ON
    Xcp_Send8(
        UINT8(8), UINT8(XCP_PACKET_IDENTIFIER_RES), properties, listConf->numOdts, 0, /* Hardcoded for now. */
        0,                                                                            /* Hardcoded for now. */
        UINT8(0), UINT8(0), UINT8(0)
    );
        #else
    Xcp_ErrorResponse(UINT8(ERR_CMD_SYNTAX));
        #endif /* XCP_DAQ_ENABLE_PREDEFINED_LISTS */
}

    #endif /* XCP_ENABLE_GET_DAQ_LIST_INFO */

    #if XCP_ENABLE_FREE_DAQ == XCP_ON

XCP_STATIC void Xcp_FreeDaq_Res(Xcp_PduType const * const pdu) {
    DBG_TRACE("FREE_DAQ\n\r");
    XCP_ASSERT_UNLOCKED(XCP_RESOURCE_DAQ);
    XCP_ASSERT_PGM_IDLE();
    Xcp_SendResult(XcpDaq_Free());
}

    #endif /* XCP_ENABLE_FREE_DAQ */

    #if XCP_ENABLE_ALLOC_DAQ == XCP_ON

XCP_STATIC void Xcp_AllocDaq_Res(Xcp_PduType const * const pdu) {
    XcpDaq_ListIntegerType daqCount = (XcpDaq_ListIntegerType)0;

    XCP_ASSERT_PGM_IDLE();
    XCP_ASSERT_UNLOCKED(XCP_RESOURCE_DAQ);
    daqCount = (XcpDaq_ListIntegerType)Xcp_GetWord(pdu, UINT8(2));
    DBG_TRACE("ALLOC_DAQ [count: %u] \n\r", daqCount);
    Xcp_SendResult(XcpDaq_Alloc(daqCount));
}

    #endif

    #if XCP_ENABLE_ALLOC_ODT == XCP_ON

XCP_STATIC void Xcp_AllocOdt_Res(Xcp_PduType const * const pdu) {
    XcpDaq_ListIntegerType daqListNumber = (XcpDaq_ListIntegerType)0;
    XcpDaq_ODTIntegerType  odtCount      = (XcpDaq_ODTIntegerType)0;

    XCP_ASSERT_PGM_IDLE();
    XCP_ASSERT_UNLOCKED(XCP_RESOURCE_DAQ);
    daqListNumber = (XcpDaq_ListIntegerType)Xcp_GetWord(pdu, UINT8(2));
    odtCount      = Xcp_GetByte(pdu, UINT8(4));
    DBG_TRACE("ALLOC_ODT [daq: %u count: %u] \n\r", daqListNumber, odtCount);
    Xcp_SendResult(XcpDaq_AllocOdt(daqListNumber, odtCount));
}

    #endif /* XCP_ENABLE_ALLOC_ODT */

    #if XCP_ENABLE_ALLOC_ODT_ENTRY == XCP_ON

XCP_STATIC void Xcp_AllocOdtEntry_Res(Xcp_PduType const * const pdu) {
    XcpDaq_ListIntegerType     daqListNumber   = (XcpDaq_ListIntegerType)0;
    XcpDaq_ODTIntegerType      odtNumber       = (XcpDaq_ODTIntegerType)0;
    XcpDaq_ODTEntryIntegerType odtEntriesCount = (XcpDaq_ODTEntryIntegerType)0;

    XCP_ASSERT_PGM_IDLE();
    XCP_ASSERT_UNLOCKED(XCP_RESOURCE_DAQ);
    daqListNumber   = (XcpDaq_ListIntegerType)Xcp_GetWord(pdu, UINT8(2));
    odtNumber       = Xcp_GetByte(pdu, UINT8(4));
    odtEntriesCount = Xcp_GetByte(pdu, UINT8(5));
    DBG_TRACE("ALLOC_ODT_ENTRY: [daq: %u odt: %u count: %u]\n\r", daqListNumber, odtNumber, odtEntriesCount);
    Xcp_SendResult(XcpDaq_AllocOdtEntry(daqListNumber, odtNumber, odtEntriesCount));
}

    #endif /* XCP_ENABLE_ALLOC_ODT_ENTRY */

    #if XCP_ENABLE_GET_DAQ_CLOCK == XCP_ON

XCP_STATIC void Xcp_GetDaqClock_Res(Xcp_PduType const * const pdu) {
    uint32_t timestamp = UINT32(0);

    XCP_ASSERT_PGM_IDLE();
        #if XCP_DAQ_CLOCK_ACCESS_ALWAYS_SUPPORTED == XCP_OFF
    XCP_ASSERT_UNLOCKED(XCP_RESOURCE_DAQ);
        #endif /* XCP_DAQ_ENABLE_CLOCK_ACCESS_ALWAYS */

    timestamp = XcpHw_GetTimerCounter();
    DBG_TRACE("GET_DAQ_CLOCK [timestamp: %u]\n\r", timestamp);

        #if defined(XCP_ENABLE_TIME_CORRELATION) && (XCP_ENABLE_TIME_CORRELATION == XCP_ON)
    if (XcpTimecorr_GetResponseFmt() >= XCP_TIMECORR_FMT_EXT_1) {
        /* Extended mode: send EV_TIME_SYNC event packet instead of legacy response */
        XcpTimecorr_SendTimeSyncEvent(XCP_TIMECORR_TRIG_HW);
        return;
    }
        #endif /* XCP_ENABLE_TIME_CORRELATION */

    /* Legacy mode response */
    Xcp_Send8(
        UINT8(8), UINT8(XCP_PACKET_IDENTIFIER_RES), UINT8(0), UINT8(0), UINT8(0), XCP_LOBYTE(XCP_LOWORD(timestamp)),
        XCP_HIBYTE(XCP_LOWORD(timestamp)), XCP_LOBYTE(XCP_HIWORD(timestamp)), XCP_HIBYTE(XCP_HIWORD(timestamp))
    );
}

    #endif /* XCP_ENABLE_GET_DAQ_CLOCK */

    #if defined(XCP_ENABLE_TIME_CORRELATION) && (XCP_ENABLE_TIME_CORRELATION == XCP_ON)
XCP_STATIC void Xcp_TimeCorrelationProperties_Res(Xcp_PduType const * const pdu) {
    XcpTimecorr_HandleTimeCorrelationProperties(pdu);
}
    #endif /* XCP_ENABLE_TIME_CORRELATION */

    #if XCP_ENABLE_GET_DAQ_RESOLUTION_INFO == XCP_ON

XCP_STATIC void Xcp_GetDaqResolutionInfo_Res(Xcp_PduType const * const pdu) {
    DBG_TRACE("GET_DAQ_RESOLUTION_INFO\n\r");
    XCP_ASSERT_PGM_IDLE();
    XCP_ASSERT_UNLOCKED(XCP_RESOURCE_DAQ);

    Xcp_Send8(
        UINT8(8), UINT8(XCP_PACKET_IDENTIFIER_RES), UINT8(1), /* Granularity for size of ODT entry (DIRECTION = DAQ) */
        UINT8(XCP_DAQ_MAX_ODT_ENTRY_SIZE),                    /* Maximum size of ODT entry (DIRECTION = DAQ) */
        UINT8(1),                                             /* Granularity for size of ODT entry (DIRECTION = STIM) */
        UINT8(XCP_DAQ_MAX_ODT_ENTRY_SIZE),                    /* Maximum size of ODT entry (DIRECTION = STIM) */
        UINT8(0x34),                                          /* Timestamp unit and size */
        UINT8(1),                                             /* Timestamp ticks per unit (WORD) */
        UINT8(0)
    );
}

    #endif /* XCP_ENABLE_GET_DAQ_RESOLUTION_INFO */

    #if XCP_ENABLE_GET_DAQ_EVENT_INFO == XCP_ON

XCP_STATIC void Xcp_GetDaqEventInfo_Res(Xcp_PduType const * const pdu) {
    uint16_t                eventChannel = Xcp_GetWord(pdu, UINT8(2));
    uint8_t                 nameLen      = UINT8(0);
    XcpDaq_EventType const *event        = XCP_NULL;

    DBG_TRACE("GET_DAQ_EVENT_INFO [eventChannel: %d]\n\r", eventChannel);
    XCP_ASSERT_UNLOCKED(XCP_RESOURCE_DAQ);

    if (eventChannel >= UINT8(XCP_DAQ_MAX_EVENT_CHANNEL)) {
        Xcp_SendResult(ERR_OUT_OF_RANGE);
        return;
    }
    event = XcpDaq_GetEventConfiguration(eventChannel);
    if ((event->name == XCP_NULL) || (event->nameLen == UINT8(0))) {
        nameLen = UINT8(0);
    } else {
        nameLen = event->nameLen;
        Xcp_SetMta(Xcp_GetNonPagedAddress(event->name));
    }

    Xcp_Send8(
        UINT8(7), UINT8(XCP_PACKET_IDENTIFIER_RES), UINT8(event->properties), /* DAQ_EVENT_PROPERTIES */
        UINT8(1),               /* maximum number of DAQ lists in this event channel */
        UINT8(nameLen),         /* EVENT_CHANNEL_NAME_LENGTH in bytes 0 – If not
                                         available */
        UINT8(event->cycle),    /* EVENT_CHANNEL_TIME_CYCLE 0 – Not cyclic */
        UINT8(event->timeunit), /* EVENT_CHANNEL_TIME_UNIT don’t care if
                                 Event channel time cycle = 0 */
        UINT8(0),               /* EVENT_CHANNEL_PRIORITY (FF highest) */
        UINT8(0)
    );
}

    #endif /* XCP_ENABLE_GET_DAQ_EVENT_INFO */

#endif /* XCP_ENABLE_DAQ_COMMANDS */

/*
**
** PGM Services.
**
*/
#if XCP_ENABLE_PGM_COMMANDS == XCP_ON

XCP_STATIC void Xcp_ProgramStart_Res(Xcp_PduType const * const pdu) {
    DBG_TRACE("PROGRAM_START\n\r");
    XCP_ASSERT_UNLOCKED(XCP_RESOURCE_PGM);
    XCP_ASSERT_DAQ_STOPPED();
    XcpPgm_SetProcessorState(XCP_PGM_ACTIVE);

    Xcp_Send8(
        UINT8(7), UINT8(XCP_PACKET_IDENTIFIER_RES), UINT8(0), /* Reserved */
        UINT8(0),                                             /* COMM_MODE_PGM */
        UINT8(XCP_MAX_CTO),                                   /* MAX_CTO_PGM [BYTES]Maximum CTO size for PGM */
        UINT8(XCP_MAX_BS_PGM),                                /* MAX_BS_PGM */
        UINT8(XCP_MIN_ST_PGM),                                /* MIN_ST_PGM */
        UINT8(0),                                             /* QUEUE_SIZE_PGM */
        UINT8(0)
    );
}

XCP_STATIC void Xcp_ProgramClear_Res(Xcp_PduType const * const pdu) {
    uint8_t  mode       = Xcp_GetByte(pdu, UINT8(1));
    uint32_t clearRange = Xcp_GetDWord(pdu, UINT8(4));

    DBG_TRACE("PROGRAM_CLEAR [mode: %d clearRange: 0x%08x]\n\r", mode, clearRange);
    XCP_ASSERT_PGM_ACTIVE();

    Xcp_PositiveResponse();
}

XCP_STATIC void Xcp_Program_Res(Xcp_PduType const * const pdu) {
    uint8_t     len = Xcp_GetByte(pdu, UINT8(1));
    Xcp_MtaType src = { 0 };

    DBG_TRACE("PROGRAM [len: %u]\n\r", len);

    XCP_ASSERT_PGM_ACTIVE();
    XCP_CHECK_MEMORY_ACCESS(Xcp_State.mta, len, XCP_MEM_ACCESS_WRITE, (bool)XCP_TRUE);
    src.address = (Xcp_PointerSizeType)pdu->data + 2;
    src.ext     = UINT8(0);
    //    Xcp_CopyMemory(Xcp_State.mta, src, (uint32_t)len);

    XCP_INCREMENT_MTA(len);

    Xcp_PositiveResponse();
}

XCP_STATIC void Xcp_ProgramReset_Res(Xcp_PduType const * const pdu) {
    DBG_TRACE("PROGRAM_RESET\n\r");
    XCP_ASSERT_PGM_ACTIVE();

    XcpPgm_SetProcessorState(XCP_PGM_IDLE);
    Xcp_PositiveResponse();
}

    #if XCP_ENABLE_GET_PGM_PROCESSOR_INFO == XCP_ON

XCP_STATIC void Xcp_GetPgmProcessorInfo_Res(Xcp_PduType const * const pdu) {
    DBG_TRACE("GET_PGM_PROCESSOR_INFO\n\r");
    /* XCP_ASSERT_UNLOCKED(XCP_RESOURCE_PGM); */

    Xcp_Send8(
        UINT8(3), UINT8(XCP_PACKET_IDENTIFIER_RES), XCP_PGM_PROPERIES, /* PGM_PROPERTIES */
        XCP_MAX_SECTOR_PGM,                                            /* MAX_SECTOR */
        UINT8(0), UINT8(0), UINT8(0), UINT8(0), UINT8(0)
    );
}

    #endif /* XCP_ENABLE_GET_PGM_PROCESSOR_INFO */

    #if XCP_ENABLE_GET_SECTOR_INFO == XCP_ON

XCP_STATIC void Xcp_GetSectorInfo_Res(Xcp_PduType const * const pdu) {
    uint8_t mode         = Xcp_GetByte(pdu, UINT8(1));
    uint8_t sectorNumber = Xcp_GetByte(pdu, UINT8(2));

    DBG_TRACE("GET_SECTOR_INFO [sectorNumber: %d mode :%d]\n\r", sectorNumber, mode);

    /* XCP_ASSERT_UNLOCKED(XCP_RESOURCE_PGM); */

    if (sectorNumber >= XCP_MAX_SECTOR_PGM) {
        Xcp_SendResult(ERR_SEGMENT_NOT_VALID);
        return;
    }

    if (mode == UINT8(0)) {
        /* Start-Address */
    } else if (mode == UINT8(1)) {
        /* Length */
    } else {
        Xcp_SendResult(ERR_MODE_NOT_VALID);
        return;
    }

    Xcp_Send8(
        UINT8(8), UINT8(XCP_PACKET_IDENTIFIER_RES), UINT8(sectorNumber * 2), /* Clear Sequence Number */
        UINT8((sectorNumber * 2) + 1),                                       /* Program Sequence Number */
        UINT8(0),                                                            /* Programming method */
        UINT8(0),                                                            /* SECTOR_INFO - DWORD*/
        UINT8(0), UINT8(0), UINT8(0)
    );
}

    #endif /* XCP_ENABLE_GET_SECTOR_INFO */

    #if XCP_ENABLE_PROGRAM_PREPARE == XCP_ON

XCP_STATIC void Xcp_ProgramPrepare_Res(Xcp_PduType const * const pdu) {
    uint16_t codeSize = Xcp_GetWord(pdu, UINT8(2));

    DBG_TRACE("PROGRAM_PREPARE [codeSize: %d]\n\r", codeSize);
    XCP_ASSERT_PGM_ACTIVE();

    Xcp_PositiveResponse();
}

    #endif /* XCP_ENABLE_PROGRAM_PREPARE */

    #if XCP_ENABLE_PROGRAM_FORMAT == XCP_ON
XCP_STATIC void Xcp_ProgramFormat_Res(Xcp_PduType const * const pdu) {
    uint8_t compressionMethod = Xcp_GetByte(pdu, UINT8(1));
    uint8_t encryptionMethod  = Xcp_GetByte(pdu, UINT8(2));
    uint8_t programmingMethod = Xcp_GetByte(pdu, UINT8(3));
    uint8_t accessMethod      = Xcp_GetByte(pdu, UINT8(4));

    DBG_TRACE(
        "PROGRAM_FORMAT [compression: %d encryption: %d programming: %d "
        "access: %d]\n\r",
        compressionMethod, encryptionMethod, programmingMethod, accessMethod
    );
    XCP_ASSERT_PGM_ACTIVE();

    Xcp_PositiveResponse();
}
    #endif /* XCP_ENABLE_PROGRAM_FORMAT */

    #if XCP_ENABLE_PROGRAM_NEXT == XCP_ON
XCP_STATIC void Xcp_ProgramNext_Res(Xcp_PduType const * const pdu) {
    DBG_TRACE("PROGRAM_NEXT\n\r");
    XCP_ASSERT_PGM_ACTIVE();

    Xcp_PositiveResponse();
}
    #endif /* XCP_ENABLE_PROGRAM_NEXT */

    #if XCP_ENABLE_PROGRAM_MAX == XCP_ON
XCP_STATIC void Xcp_ProgramMax_Res(Xcp_PduType const * const pdu) {
    DBG_TRACE("PROGRAM_MAX\n\r");
    XCP_ASSERT_PGM_ACTIVE();

    Xcp_PositiveResponse();
}
    #endif /* XCP_ENABLE_PROGRAM_MAX */

    #if XCP_ENABLE_PROGRAM_VERIFY == XCP_ON
XCP_STATIC void Xcp_ProgramVerify_Res(Xcp_PduType const * const pdu) {
    DBG_TRACE("PROGRAM_VERIFY\n\r");
    XCP_ASSERT_PGM_ACTIVE();

    Xcp_PositiveResponse();
}
    #endif /* XCP_ENABLE_PROGRAM_VERIFY */

#endif /* XCP_ENABLE_PGM_COMMANDS */

/*
**  Helpers.
*/
XCP_STATIC void Xcp_SendResult(Xcp_ReturnType result) {
    if (result == ERR_SUCCESS) {
        Xcp_PositiveResponse();
    } else {
        Xcp_ErrorResponse(UINT8(result));
    }
}

#if 0
void Xcp_WriteMemory(void *dest, void *src, uint16_t count) {
    XcpUtl_MemCopy(dest, src, UINT32(count));
}
#endif

#if XCP_REPLACE_STD_COPY_MEMORY == XCP_OFF

void Xcp_CopyMemory(Xcp_MtaType dst, Xcp_MtaType src, uint32_t len) {
    #if XCP_ENABLE_ADDRESS_MAPPER == XCP_ON
    Xcp_MtaType                 tmpD = { 0 };
    Xcp_MtaType                 tmpS = { 0 };
    Xcp_MemoryMappingResultType res  = XCP_MEMORY_ADDRESS_INVALID;

    tmpD.address = dst.address;
    tmpS.address = src.address;

    res = Xcp_HookFunction_AddressMapper(&tmpD, &dst);
    if (res == XCP_MEMORY_MAPPED) {
    } else if (res == XCP_MEMORY_NOT_MAPPED) {
    } else if (res == XCP_MEMORY_ADDRESS_INVALID) {
    } else {
    }
    res = Xcp_HookFunction_AddressMapper(&tmpS, &src);
    if (res == XCP_MEMORY_MAPPED) {
    } else if (res == XCP_MEMORY_NOT_MAPPED) {
    } else if (res == XCP_MEMORY_ADDRESS_INVALID) {
    } else {
    }
    XcpUtl_MemCopy((void *)tmpD.address, (void *)tmpS.address, len);
    #else

    /* Without address-mapper we don't know how to handle address extensions. */
    XcpUtl_MemCopy((void *)dst.address, (void *)src.address, len);
    #endif /* XCP_ENABLE_ADDRESS_MAPPER */
}

#endif /* XCP_REPLACE_STD_COPY_MEMORY */

INLINE uint8_t Xcp_GetByte(Xcp_PduType const * const pdu, uint8_t offs) {
    return (*(pdu->data + offs));
}

INLINE uint16_t Xcp_GetWord(Xcp_PduType const * const pdu, uint8_t offs) {
#if XCP_BYTE_ORDER == XCP_BYTE_ORDER_INTEL
    return ((*(pdu->data + offs)) & UINT8(0xff)) | (((*(pdu->data + UINT8(1) + offs)) & UINT8(0xff)) << UINT8(8));
#elif XCP_BYTE_ORDER == XCP_BYTE_ORDER_MOTOROLA
    return (((*(pdu->data + offs)) & UINT8(0xff)) << UINT8(8)) | ((*(pdu->data + UINT8(1) + offs)) & UINT8(0xff));
#endif
}

INLINE uint32_t Xcp_GetDWord(Xcp_PduType const * const pdu, uint8_t offs) {
    uint16_t h = UINT16(0);
    uint16_t l = UINT16(0);

#if XCP_BYTE_ORDER == XCP_BYTE_ORDER_INTEL
    l = Xcp_GetWord(pdu, offs + 0);
    h = Xcp_GetWord(pdu, offs + 2);
#elif XCP_BYTE_ORDER == XCP_BYTE_ORDER_MOTOROLA
    h = Xcp_GetWord(pdu, offs + 0);
    l = Xcp_GetWord(pdu, offs + 2);
#endif
    return (uint32_t)(h * 0x10000) + l;
}

INLINE void Xcp_SetByte(Xcp_PduType const * const pdu, uint8_t offs, uint8_t value) {
    (*(pdu->data + offs)) = value;
}

INLINE void Xcp_SetWord(Xcp_PduType const * const pdu, uint8_t offs, uint16_t value) {
#if XCP_BYTE_ORDER == XCP_BYTE_ORDER_INTEL
    (*(pdu->data + offs))            = value & UINT8(0xff);
    (*(pdu->data + UINT8(1) + offs)) = (value & UINT16(0xff00)) >> UINT8(8);
#elif XCP_BYTE_ORDER == XCP_BYTE_ORDER_MOTOROLA
    (*(pdu->data + offs))            = (value & UINT16(0xff00)) >> UINT8(8);
    (*(pdu->data + UINT8(1) + offs)) = value & UINT8(0xff);
#endif
}

INLINE void Xcp_SetDWord(Xcp_PduType const * const pdu, uint8_t offs, uint32_t value) {
#if XCP_BYTE_ORDER == XCP_BYTE_ORDER_INTEL
    Xcp_SetWord(pdu, offs + 2, (value & UINT32(0xffff0000)) >> UINT8(16));
    Xcp_SetWord(pdu, offs + 0, value & UINT32(0x0000ffff));
#elif XCP_BYTE_ORDER == XCP_BYTE_ORDER_MOTOROLA
    Xcp_SetWord(pdu, offs + 0, (value & UINT32(0xffff0000)) >> UINT8(16));
    Xcp_SetWord(pdu, offs + 2, value & UINT32(0x0000ffff));
#endif
}

#if XCP_ENABLE_RESOURCE_PROTECTION == XCP_ON
XCP_STATIC bool Xcp_IsProtected(uint8_t resource) {
    return ((Xcp_State.resourceProtection & resource) == resource);
}
#else

XCP_STATIC bool Xcp_IsProtected(uint8_t resource) {
    return XCP_FALSE;
}

#endif /* XCP_ENABLE_RESOURCE_PROTECTION */

void Xcp_SetBusy(bool enable) {
    XCP_ENTER_CRITICAL();
    Xcp_State.busy = enable;
    XCP_LEAVE_CRITICAL();
}

bool Xcp_IsBusy(void) {
    return Xcp_State.busy;
}

Xcp_StateType *Xcp_GetState(void) {
    Xcp_StateType *tState = XCP_NULL;

    XCP_DAQ_ENTER_CRITICAL();
    tState = &Xcp_State;
    XCP_DAQ_LEAVE_CRITICAL();

    return tState;
}

XCP_STATIC void Xcp_PositiveResponse(void) {
    Xcp_Send8(UINT8(1), UINT8(XCP_PACKET_IDENTIFIER_RES), UINT8(0), UINT8(0), UINT8(0), UINT8(0), UINT8(0), UINT8(0), UINT8(0));
}

XCP_STATIC void Xcp_ErrorResponse(uint8_t errorCode) {
    Xcp_Send8(
        UINT8(2), UINT8(XCP_PACKET_IDENTIFIER_ERR), UINT8(errorCode), UINT8(0), UINT8(0), UINT8(0), UINT8(0), UINT8(0), UINT8(0)
    );
}

XCP_STATIC void Xcp_BusyResponse(void) {
#if XCP_ENABLE_STATISTICS == XCP_ON
    Xcp_State.statistics.crosBusy++;
#endif /* XCP_ENABLE_STATISTICS */
    Xcp_ErrorResponse(ERR_CMD_BUSY);
}

#if (XCP_ENABLE_SERVICE_REQUEST_API == XCP_ON) || (XCP_ENABLE_EVENT_PACKET_API)

XCP_STATIC void Xcp_SendSpecialPacket(uint8_t packetType, uint8_t code, uint8_t const * const data, uint8_t dataLength) {
    uint8_t *dataOut     = Xcp_GetCtoOutPtr();
    uint8_t  frameLength = UINT8(0);
    uint8_t  idx         = UINT8(0);

    dataLength = XCP_MIN(XCP_MAX_CTO - 2, dataLength); /* Silently truncate. */

    #if XCP_ON_CAN_MAX_DLC_REQUIRED == XCP_ON
    frameLength = XCP_MAX_CTO;
    #else
    frameLength = dataLength + 2;
    #endif /* XCP_ON_CAN_MAX_DLC_REQUIRED */
    Xcp_SetCtoOutLen(UINT16(frameLength));
    dataOut[0] = packetType;
    dataOut[1] = code;
    if (dataLength) {
        for (idx = UINT8(0); idx < dataLength; ++idx) {
            dataOut[2 + idx] = data[idx];
        }
    }
    Xcp_SendCto();
}

#endif

#if XCP_ENABLE_EVENT_PACKET_API == XCP_ON

void Xcp_SendEventPacket(uint8_t eventCode, uint8_t const * const eventInfo, uint8_t eventInfoLength) {
    Xcp_SendSpecialPacket(XCP_PACKET_IDENTIFIER_EV, eventCode, eventInfo, eventInfoLength);
}

#endif /* XCP_ENABLE_EVENT_PACKET_API */

#if XCP_ENABLE_SERVICE_REQUEST_API == XCP_ON
void Xcp_SendServiceRequestPacket(uint8_t serviceRequestCode, uint8_t const * const serviceRequest, uint8_t serviceRequestLength) {
    Xcp_SendSpecialPacket(XCP_PACKET_IDENTIFIER_SERV, serviceRequestCode, serviceRequest, serviceRequestLength);
}
#endif /* XCP_ENABLE_SERVICE_REQUEST_API */

#if 0
XCP_STATIC Xcp_MemoryMappingResultType Xcp_MapMemory(Xcp_MtaType const *src, Xcp_MtaType *dst) {
    Xcp_MemoryMappingResultType mappingResult;

    /*    FlsEmu_MemoryMapper(tmpAddr); */

    Xcp_MtaType dest;
    printf("addr: %x ext: %d\n", mta->address, mta->ext);
    if ((mta->address >= 0x4000) && (mta->address < 0x5000)) {
        mta->address = (0x00A90000 - 0x4000) + mta->address;
    }
    printf("MAPPED: addr: %x ext: %d\n", mta->address, mta->ext);
}
#endif

XCP_STATIC void Xcp_Download_Copy(Xcp_PointerSizeType address, uint8_t ext, uint32_t len) {
    Xcp_MtaType src = { 0 };

    src.address = address;
    src.ext     = ext;
    Xcp_CopyMemory(Xcp_State.mta, src, len);
    XCP_INCREMENT_MTA(len);
}

#if XCP_ENABLE_PGM_COMMANDS == XCP_ON

void XcpPgm_SetProcessorState(XcpPgm_ProcessorStateType state) {
    Xcp_StateType *tmpState = { 0 };

    tmpState = Xcp_GetState();
    XCP_PGM_ENTER_CRITICAL();
    tmpState->pgmProcessor.state = state;
    XCP_PGM_LEAVE_CRITICAL();
}

#endif /* ENABLE_PGM_COMMANDS */

#if XCP_ENABLE_DAQ_COMMANDS == XCP_ON

#endif /* XCP_ENABLE_DAQ_COMMANDS */

////////////////////////////////////////////////////////////////////////////////
//                               xcp_checksum.c                               //
////////////////////////////////////////////////////////////////////////////////

/*
 * BlueParrot XCP
 *
 * (C) 2007-2022 by Christoph Schueler <github.com/Christoph2,
 *                                      cpu12.gems@googlemail.com>
 *
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * s. FLOSS-EXCEPTION.txt
 */

/** CRC calculation based on code by Michael Barr: **/
/**********************************************************************
 * Copyright (c) 2000 by Michael Barr.  This software is placed into
 * the public domain and may be used for any purpose.  However, this
 * notice must not be changed or removed and no warranty is either
 * expressed or implied by its publication or distribution.
 **********************************************************************/


/* Place CRC lookup tables into Flash/ROM on AVR (Arduino Uno) and
 * provide accessor macros to read from PROGMEM.
 */
#if defined(__AVR__)
    #include <avr/pgmspace.h>
    #define XCP_PROGMEM     PROGMEM
    #define XCP_READ16(idx) pgm_read_word(&CRC_TAB[(idx)])
    #define XCP_READ32(idx) pgm_read_dword(&CRC_TAB[(idx)])
#else
    #define XCP_PROGMEM
    #define XCP_READ16(idx) (CRC_TAB[(idx)])
    #define XCP_READ32(idx) (CRC_TAB[(idx)])
#endif

#if XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_CRC_32
    #define READ_CRC_TAB(i) XCP_READ32((i))
#else
    #define READ_CRC_TAB(i) XCP_READ16((i))
#endif

/*
** Local Types
*/
typedef enum tagXcp_ChecksumJobStateType {
    XCP_CHECKSUM_STATE_IDLE,
    XCP_CHECKSUM_STATE_RUNNING_INITIAL,
    XCP_CHECKSUM_STATE_RUNNING_REMAINING,
    XCP_CHECKSUM_STATE_RUNNING_FINAL
} Xcp_ChecksumJobStateType;

typedef struct tagXcp_ChecksumJobType {
    Xcp_ChecksumJobStateType state;
    Xcp_MtaType              mta;
    uint32_t                 size;
    Xcp_ChecksumType         interimChecksum;
} Xcp_ChecksumJobType;

#if XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_ADD_11

#elif XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_ADD_12

#elif XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_ADD_14

#elif XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_ADD_22

#elif XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_ADD_24

#elif XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_ADD_44

#elif XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_CRC_16

    #define XCP_CRC_NAME            "CRC-16"
    #define XCP_CRC_POLYNOMIAL      ((uint16_t)0x8005)
    #define XCP_CRC_INITIAL_VALUE   ((uint16_t)0x0000)
    #define XCP_CRC_FINAL_XOR_VALUE ((uint16_t)0x0000)
    #define REFLECT_DATA            XCP_TRUE
    #define REFLECT_REMAINDER       XCP_TRUE
    #define CHECK_VALUE             ((uint16_t)0xBB3D)

static const uint16_t CRC_TAB[] XCP_PROGMEM = {
    (uint16_t)0x0000, (uint16_t)0x8005, (uint16_t)0x800F, (uint16_t)0x000A, (uint16_t)0x801B, (uint16_t)0x001E, (uint16_t)0x0014,
    (uint16_t)0x8011, (uint16_t)0x8033, (uint16_t)0x0036, (uint16_t)0x003C, (uint16_t)0x8039, (uint16_t)0x0028, (uint16_t)0x802D,
    (uint16_t)0x8027, (uint16_t)0x0022, (uint16_t)0x8063, (uint16_t)0x0066, (uint16_t)0x006C, (uint16_t)0x8069, (uint16_t)0x0078,
    (uint16_t)0x807D, (uint16_t)0x8077, (uint16_t)0x0072, (uint16_t)0x0050, (uint16_t)0x8055, (uint16_t)0x805F, (uint16_t)0x005A,
    (uint16_t)0x804B, (uint16_t)0x004E, (uint16_t)0x0044, (uint16_t)0x8041, (uint16_t)0x80C3, (uint16_t)0x00C6, (uint16_t)0x00CC,
    (uint16_t)0x80C9, (uint16_t)0x00D8, (uint16_t)0x80DD, (uint16_t)0x80D7, (uint16_t)0x00D2, (uint16_t)0x00F0, (uint16_t)0x80F5,
    (uint16_t)0x80FF, (uint16_t)0x00FA, (uint16_t)0x80EB, (uint16_t)0x00EE, (uint16_t)0x00E4, (uint16_t)0x80E1, (uint16_t)0x00A0,
    (uint16_t)0x80A5, (uint16_t)0x80AF, (uint16_t)0x00AA, (uint16_t)0x80BB, (uint16_t)0x00BE, (uint16_t)0x00B4, (uint16_t)0x80B1,
    (uint16_t)0x8093, (uint16_t)0x0096, (uint16_t)0x009C, (uint16_t)0x8099, (uint16_t)0x0088, (uint16_t)0x808D, (uint16_t)0x8087,
    (uint16_t)0x0082, (uint16_t)0x8183, (uint16_t)0x0186, (uint16_t)0x018C, (uint16_t)0x8189, (uint16_t)0x0198, (uint16_t)0x819D,
    (uint16_t)0x8197, (uint16_t)0x0192, (uint16_t)0x01B0, (uint16_t)0x81B5, (uint16_t)0x81BF, (uint16_t)0x01BA, (uint16_t)0x81AB,
    (uint16_t)0x01AE, (uint16_t)0x01A4, (uint16_t)0x81A1, (uint16_t)0x01E0, (uint16_t)0x81E5, (uint16_t)0x81EF, (uint16_t)0x01EA,
    (uint16_t)0x81FB, (uint16_t)0x01FE, (uint16_t)0x01F4, (uint16_t)0x81F1, (uint16_t)0x81D3, (uint16_t)0x01D6, (uint16_t)0x01DC,
    (uint16_t)0x81D9, (uint16_t)0x01C8, (uint16_t)0x81CD, (uint16_t)0x81C7, (uint16_t)0x01C2, (uint16_t)0x0140, (uint16_t)0x8145,
    (uint16_t)0x814F, (uint16_t)0x014A, (uint16_t)0x815B, (uint16_t)0x015E, (uint16_t)0x0154, (uint16_t)0x8151, (uint16_t)0x8173,
    (uint16_t)0x0176, (uint16_t)0x017C, (uint16_t)0x8179, (uint16_t)0x0168, (uint16_t)0x816D, (uint16_t)0x8167, (uint16_t)0x0162,
    (uint16_t)0x8123, (uint16_t)0x0126, (uint16_t)0x012C, (uint16_t)0x8129, (uint16_t)0x0138, (uint16_t)0x813D, (uint16_t)0x8137,
    (uint16_t)0x0132, (uint16_t)0x0110, (uint16_t)0x8115, (uint16_t)0x811F, (uint16_t)0x011A, (uint16_t)0x810B, (uint16_t)0x010E,
    (uint16_t)0x0104, (uint16_t)0x8101, (uint16_t)0x8303, (uint16_t)0x0306, (uint16_t)0x030C, (uint16_t)0x8309, (uint16_t)0x0318,
    (uint16_t)0x831D, (uint16_t)0x8317, (uint16_t)0x0312, (uint16_t)0x0330, (uint16_t)0x8335, (uint16_t)0x833F, (uint16_t)0x033A,
    (uint16_t)0x832B, (uint16_t)0x032E, (uint16_t)0x0324, (uint16_t)0x8321, (uint16_t)0x0360, (uint16_t)0x8365, (uint16_t)0x836F,
    (uint16_t)0x036A, (uint16_t)0x837B, (uint16_t)0x037E, (uint16_t)0x0374, (uint16_t)0x8371, (uint16_t)0x8353, (uint16_t)0x0356,
    (uint16_t)0x035C, (uint16_t)0x8359, (uint16_t)0x0348, (uint16_t)0x834D, (uint16_t)0x8347, (uint16_t)0x0342, (uint16_t)0x03C0,
    (uint16_t)0x83C5, (uint16_t)0x83CF, (uint16_t)0x03CA, (uint16_t)0x83DB, (uint16_t)0x03DE, (uint16_t)0x03D4, (uint16_t)0x83D1,
    (uint16_t)0x83F3, (uint16_t)0x03F6, (uint16_t)0x03FC, (uint16_t)0x83F9, (uint16_t)0x03E8, (uint16_t)0x83ED, (uint16_t)0x83E7,
    (uint16_t)0x03E2, (uint16_t)0x83A3, (uint16_t)0x03A6, (uint16_t)0x03AC, (uint16_t)0x83A9, (uint16_t)0x03B8, (uint16_t)0x83BD,
    (uint16_t)0x83B7, (uint16_t)0x03B2, (uint16_t)0x0390, (uint16_t)0x8395, (uint16_t)0x839F, (uint16_t)0x039A, (uint16_t)0x838B,
    (uint16_t)0x038E, (uint16_t)0x0384, (uint16_t)0x8381, (uint16_t)0x0280, (uint16_t)0x8285, (uint16_t)0x828F, (uint16_t)0x028A,
    (uint16_t)0x829B, (uint16_t)0x029E, (uint16_t)0x0294, (uint16_t)0x8291, (uint16_t)0x82B3, (uint16_t)0x02B6, (uint16_t)0x02BC,
    (uint16_t)0x82B9, (uint16_t)0x02A8, (uint16_t)0x82AD, (uint16_t)0x82A7, (uint16_t)0x02A2, (uint16_t)0x82E3, (uint16_t)0x02E6,
    (uint16_t)0x02EC, (uint16_t)0x82E9, (uint16_t)0x02F8, (uint16_t)0x82FD, (uint16_t)0x82F7, (uint16_t)0x02F2, (uint16_t)0x02D0,
    (uint16_t)0x82D5, (uint16_t)0x82DF, (uint16_t)0x02DA, (uint16_t)0x82CB, (uint16_t)0x02CE, (uint16_t)0x02C4, (uint16_t)0x82C1,
    (uint16_t)0x8243, (uint16_t)0x0246, (uint16_t)0x024C, (uint16_t)0x8249, (uint16_t)0x0258, (uint16_t)0x825D, (uint16_t)0x8257,
    (uint16_t)0x0252, (uint16_t)0x0270, (uint16_t)0x8275, (uint16_t)0x827F, (uint16_t)0x027A, (uint16_t)0x826B, (uint16_t)0x026E,
    (uint16_t)0x0264, (uint16_t)0x8261, (uint16_t)0x0220, (uint16_t)0x8225, (uint16_t)0x822F, (uint16_t)0x022A, (uint16_t)0x823B,
    (uint16_t)0x023E, (uint16_t)0x0234, (uint16_t)0x8231, (uint16_t)0x8213, (uint16_t)0x0216, (uint16_t)0x021C, (uint16_t)0x8219,
    (uint16_t)0x0208, (uint16_t)0x820D, (uint16_t)0x8207, (uint16_t)0x0202,
};

#elif XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_CRC_16_CITT

    #define XCP_CRC_NAME            "CRC-CCITT"
    #define XCP_CRC_POLYNOMIAL      ((uint16_t)0x1021)
    #define XCP_CRC_INITIAL_VALUE   ((uint16_t)0xFFFF)
    #define XCP_CRC_FINAL_XOR_VALUE ((uint16_t)0x0000)
    #define REFLECT_DATA            XCP_FALSE
    #define REFLECT_REMAINDER       XCP_FALSE
    #define CHECK_VALUE             ((uint16_t)0x29B1)

static const uint16_t CRC_TAB[] XCP_PROGMEM = {
    (uint16_t)0x0000, (uint16_t)0x1021, (uint16_t)0x2042, (uint16_t)0x3063, (uint16_t)0x4084, (uint16_t)0x50A5, (uint16_t)0x60C6,
    (uint16_t)0x70E7, (uint16_t)0x8108, (uint16_t)0x9129, (uint16_t)0xA14A, (uint16_t)0xB16B, (uint16_t)0xC18C, (uint16_t)0xD1AD,
    (uint16_t)0xE1CE, (uint16_t)0xF1EF, (uint16_t)0x1231, (uint16_t)0x0210, (uint16_t)0x3273, (uint16_t)0x2252, (uint16_t)0x52B5,
    (uint16_t)0x4294, (uint16_t)0x72F7, (uint16_t)0x62D6, (uint16_t)0x9339, (uint16_t)0x8318, (uint16_t)0xB37B, (uint16_t)0xA35A,
    (uint16_t)0xD3BD, (uint16_t)0xC39C, (uint16_t)0xF3FF, (uint16_t)0xE3DE, (uint16_t)0x2462, (uint16_t)0x3443, (uint16_t)0x0420,
    (uint16_t)0x1401, (uint16_t)0x64E6, (uint16_t)0x74C7, (uint16_t)0x44A4, (uint16_t)0x5485, (uint16_t)0xA56A, (uint16_t)0xB54B,
    (uint16_t)0x8528, (uint16_t)0x9509, (uint16_t)0xE5EE, (uint16_t)0xF5CF, (uint16_t)0xC5AC, (uint16_t)0xD58D, (uint16_t)0x3653,
    (uint16_t)0x2672, (uint16_t)0x1611, (uint16_t)0x0630, (uint16_t)0x76D7, (uint16_t)0x66F6, (uint16_t)0x5695, (uint16_t)0x46B4,
    (uint16_t)0xB75B, (uint16_t)0xA77A, (uint16_t)0x9719, (uint16_t)0x8738, (uint16_t)0xF7DF, (uint16_t)0xE7FE, (uint16_t)0xD79D,
    (uint16_t)0xC7BC, (uint16_t)0x48C4, (uint16_t)0x58E5, (uint16_t)0x6886, (uint16_t)0x78A7, (uint16_t)0x0840, (uint16_t)0x1861,
    (uint16_t)0x2802, (uint16_t)0x3823, (uint16_t)0xC9CC, (uint16_t)0xD9ED, (uint16_t)0xE98E, (uint16_t)0xF9AF, (uint16_t)0x8948,
    (uint16_t)0x9969, (uint16_t)0xA90A, (uint16_t)0xB92B, (uint16_t)0x5AF5, (uint16_t)0x4AD4, (uint16_t)0x7AB7, (uint16_t)0x6A96,
    (uint16_t)0x1A71, (uint16_t)0x0A50, (uint16_t)0x3A33, (uint16_t)0x2A12, (uint16_t)0xDBFD, (uint16_t)0xCBDC, (uint16_t)0xFBBF,
    (uint16_t)0xEB9E, (uint16_t)0x9B79, (uint16_t)0x8B58, (uint16_t)0xBB3B, (uint16_t)0xAB1A, (uint16_t)0x6CA6, (uint16_t)0x7C87,
    (uint16_t)0x4CE4, (uint16_t)0x5CC5, (uint16_t)0x2C22, (uint16_t)0x3C03, (uint16_t)0x0C60, (uint16_t)0x1C41, (uint16_t)0xEDAE,
    (uint16_t)0xFD8F, (uint16_t)0xCDEC, (uint16_t)0xDDCD, (uint16_t)0xAD2A, (uint16_t)0xBD0B, (uint16_t)0x8D68, (uint16_t)0x9D49,
    (uint16_t)0x7E97, (uint16_t)0x6EB6, (uint16_t)0x5ED5, (uint16_t)0x4EF4, (uint16_t)0x3E13, (uint16_t)0x2E32, (uint16_t)0x1E51,
    (uint16_t)0x0E70, (uint16_t)0xFF9F, (uint16_t)0xEFBE, (uint16_t)0xDFDD, (uint16_t)0xCFFC, (uint16_t)0xBF1B, (uint16_t)0xAF3A,
    (uint16_t)0x9F59, (uint16_t)0x8F78, (uint16_t)0x9188, (uint16_t)0x81A9, (uint16_t)0xB1CA, (uint16_t)0xA1EB, (uint16_t)0xD10C,
    (uint16_t)0xC12D, (uint16_t)0xF14E, (uint16_t)0xE16F, (uint16_t)0x1080, (uint16_t)0x00A1, (uint16_t)0x30C2, (uint16_t)0x20E3,
    (uint16_t)0x5004, (uint16_t)0x4025, (uint16_t)0x7046, (uint16_t)0x6067, (uint16_t)0x83B9, (uint16_t)0x9398, (uint16_t)0xA3FB,
    (uint16_t)0xB3DA, (uint16_t)0xC33D, (uint16_t)0xD31C, (uint16_t)0xE37F, (uint16_t)0xF35E, (uint16_t)0x02B1, (uint16_t)0x1290,
    (uint16_t)0x22F3, (uint16_t)0x32D2, (uint16_t)0x4235, (uint16_t)0x5214, (uint16_t)0x6277, (uint16_t)0x7256, (uint16_t)0xB5EA,
    (uint16_t)0xA5CB, (uint16_t)0x95A8, (uint16_t)0x8589, (uint16_t)0xF56E, (uint16_t)0xE54F, (uint16_t)0xD52C, (uint16_t)0xC50D,
    (uint16_t)0x34E2, (uint16_t)0x24C3, (uint16_t)0x14A0, (uint16_t)0x0481, (uint16_t)0x7466, (uint16_t)0x6447, (uint16_t)0x5424,
    (uint16_t)0x4405, (uint16_t)0xA7DB, (uint16_t)0xB7FA, (uint16_t)0x8799, (uint16_t)0x97B8, (uint16_t)0xE75F, (uint16_t)0xF77E,
    (uint16_t)0xC71D, (uint16_t)0xD73C, (uint16_t)0x26D3, (uint16_t)0x36F2, (uint16_t)0x0691, (uint16_t)0x16B0, (uint16_t)0x6657,
    (uint16_t)0x7676, (uint16_t)0x4615, (uint16_t)0x5634, (uint16_t)0xD94C, (uint16_t)0xC96D, (uint16_t)0xF90E, (uint16_t)0xE92F,
    (uint16_t)0x99C8, (uint16_t)0x89E9, (uint16_t)0xB98A, (uint16_t)0xA9AB, (uint16_t)0x5844, (uint16_t)0x4865, (uint16_t)0x7806,
    (uint16_t)0x6827, (uint16_t)0x18C0, (uint16_t)0x08E1, (uint16_t)0x3882, (uint16_t)0x28A3, (uint16_t)0xCB7D, (uint16_t)0xDB5C,
    (uint16_t)0xEB3F, (uint16_t)0xFB1E, (uint16_t)0x8BF9, (uint16_t)0x9BD8, (uint16_t)0xABBB, (uint16_t)0xBB9A, (uint16_t)0x4A75,
    (uint16_t)0x5A54, (uint16_t)0x6A37, (uint16_t)0x7A16, (uint16_t)0x0AF1, (uint16_t)0x1AD0, (uint16_t)0x2AB3, (uint16_t)0x3A92,
    (uint16_t)0xFD2E, (uint16_t)0xED0F, (uint16_t)0xDD6C, (uint16_t)0xCD4D, (uint16_t)0xBDAA, (uint16_t)0xAD8B, (uint16_t)0x9DE8,
    (uint16_t)0x8DC9, (uint16_t)0x7C26, (uint16_t)0x6C07, (uint16_t)0x5C64, (uint16_t)0x4C45, (uint16_t)0x3CA2, (uint16_t)0x2C83,
    (uint16_t)0x1CE0, (uint16_t)0x0CC1, (uint16_t)0xEF1F, (uint16_t)0xFF3E, (uint16_t)0xCF5D, (uint16_t)0xDF7C, (uint16_t)0xAF9B,
    (uint16_t)0xBFBA, (uint16_t)0x8FD9, (uint16_t)0x9FF8, (uint16_t)0x6E17, (uint16_t)0x7E36, (uint16_t)0x4E55, (uint16_t)0x5E74,
    (uint16_t)0x2E93, (uint16_t)0x3EB2, (uint16_t)0x0ED1, (uint16_t)0x1EF0,
};

#elif XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_CRC_32

    #define XCP_CRC_NAME            "CRC-32"
    #define XCP_CRC_POLYNOMIAL      ((uint32_t)0x04C11DB7)
    #define XCP_CRC_INITIAL_VALUE   ((uint32_t)0xFFFFFFFF)
    #define XCP_CRC_FINAL_XOR_VALUE ((uint32_t)0xFFFFFFFF)
    #define REFLECT_DATA            XCP_TRUE
    #define REFLECT_REMAINDER       XCP_TRUE
    #define CHECK_VALUE             ((uint32_t)0xCBF43926)

static const uint32_t CRC_TAB[] XCP_PROGMEM = {
    (uint32_t)0x00000000, (uint32_t)0x77073096, (uint32_t)0xee0e612c, (uint32_t)0x990951ba, (uint32_t)0x076dc419,
    (uint32_t)0x706af48f, (uint32_t)0xe963a535, (uint32_t)0x9e6495a3, (uint32_t)0x0edb8832, (uint32_t)0x79dcb8a4,
    (uint32_t)0xe0d5e91e, (uint32_t)0x97d2d988, (uint32_t)0x09b64c2b, (uint32_t)0x7eb17cbd, (uint32_t)0xe7b82d07,
    (uint32_t)0x90bf1d91, (uint32_t)0x1db71064, (uint32_t)0x6ab020f2, (uint32_t)0xf3b97148, (uint32_t)0x84be41de,
    (uint32_t)0x1adad47d, (uint32_t)0x6ddde4eb, (uint32_t)0xf4d4b551, (uint32_t)0x83d385c7, (uint32_t)0x136c9856,
    (uint32_t)0x646ba8c0, (uint32_t)0xfd62f97a, (uint32_t)0x8a65c9ec, (uint32_t)0x14015c4f, (uint32_t)0x63066cd9,
    (uint32_t)0xfa0f3d63, (uint32_t)0x8d080df5, (uint32_t)0x3b6e20c8, (uint32_t)0x4c69105e, (uint32_t)0xd56041e4,
    (uint32_t)0xa2677172, (uint32_t)0x3c03e4d1, (uint32_t)0x4b04d447, (uint32_t)0xd20d85fd, (uint32_t)0xa50ab56b,
    (uint32_t)0x35b5a8fa, (uint32_t)0x42b2986c, (uint32_t)0xdbbbc9d6, (uint32_t)0xacbcf940, (uint32_t)0x32d86ce3,
    (uint32_t)0x45df5c75, (uint32_t)0xdcd60dcf, (uint32_t)0xabd13d59, (uint32_t)0x26d930ac, (uint32_t)0x51de003a,
    (uint32_t)0xc8d75180, (uint32_t)0xbfd06116, (uint32_t)0x21b4f4b5, (uint32_t)0x56b3c423, (uint32_t)0xcfba9599,
    (uint32_t)0xb8bda50f, (uint32_t)0x2802b89e, (uint32_t)0x5f058808, (uint32_t)0xc60cd9b2, (uint32_t)0xb10be924,
    (uint32_t)0x2f6f7c87, (uint32_t)0x58684c11, (uint32_t)0xc1611dab, (uint32_t)0xb6662d3d, (uint32_t)0x76dc4190,
    (uint32_t)0x01db7106, (uint32_t)0x98d220bc, (uint32_t)0xefd5102a, (uint32_t)0x71b18589, (uint32_t)0x06b6b51f,
    (uint32_t)0x9fbfe4a5, (uint32_t)0xe8b8d433, (uint32_t)0x7807c9a2, (uint32_t)0x0f00f934, (uint32_t)0x9609a88e,
    (uint32_t)0xe10e9818, (uint32_t)0x7f6a0dbb, (uint32_t)0x086d3d2d, (uint32_t)0x91646c97, (uint32_t)0xe6635c01,
    (uint32_t)0x6b6b51f4, (uint32_t)0x1c6c6162, (uint32_t)0x856530d8, (uint32_t)0xf262004e, (uint32_t)0x6c0695ed,
    (uint32_t)0x1b01a57b, (uint32_t)0x8208f4c1, (uint32_t)0xf50fc457, (uint32_t)0x65b0d9c6, (uint32_t)0x12b7e950,
    (uint32_t)0x8bbeb8ea, (uint32_t)0xfcb9887c, (uint32_t)0x62dd1ddf, (uint32_t)0x15da2d49, (uint32_t)0x8cd37cf3,
    (uint32_t)0xfbd44c65, (uint32_t)0x4db26158, (uint32_t)0x3ab551ce, (uint32_t)0xa3bc0074, (uint32_t)0xd4bb30e2,
    (uint32_t)0x4adfa541, (uint32_t)0x3dd895d7, (uint32_t)0xa4d1c46d, (uint32_t)0xd3d6f4fb, (uint32_t)0x4369e96a,
    (uint32_t)0x346ed9fc, (uint32_t)0xad678846, (uint32_t)0xda60b8d0, (uint32_t)0x44042d73, (uint32_t)0x33031de5,
    (uint32_t)0xaa0a4c5f, (uint32_t)0xdd0d7cc9, (uint32_t)0x5005713c, (uint32_t)0x270241aa, (uint32_t)0xbe0b1010,
    (uint32_t)0xc90c2086, (uint32_t)0x5768b525, (uint32_t)0x206f85b3, (uint32_t)0xb966d409, (uint32_t)0xce61e49f,
    (uint32_t)0x5edef90e, (uint32_t)0x29d9c998, (uint32_t)0xb0d09822, (uint32_t)0xc7d7a8b4, (uint32_t)0x59b33d17,
    (uint32_t)0x2eb40d81, (uint32_t)0xb7bd5c3b, (uint32_t)0xc0ba6cad, (uint32_t)0xedb88320, (uint32_t)0x9abfb3b6,
    (uint32_t)0x03b6e20c, (uint32_t)0x74b1d29a, (uint32_t)0xead54739, (uint32_t)0x9dd277af, (uint32_t)0x04db2615,
    (uint32_t)0x73dc1683, (uint32_t)0xe3630b12, (uint32_t)0x94643b84, (uint32_t)0x0d6d6a3e, (uint32_t)0x7a6a5aa8,
    (uint32_t)0xe40ecf0b, (uint32_t)0x9309ff9d, (uint32_t)0x0a00ae27, (uint32_t)0x7d079eb1, (uint32_t)0xf00f9344,
    (uint32_t)0x8708a3d2, (uint32_t)0x1e01f268, (uint32_t)0x6906c2fe, (uint32_t)0xf762575d, (uint32_t)0x806567cb,
    (uint32_t)0x196c3671, (uint32_t)0x6e6b06e7, (uint32_t)0xfed41b76, (uint32_t)0x89d32be0, (uint32_t)0x10da7a5a,
    (uint32_t)0x67dd4acc, (uint32_t)0xf9b9df6f, (uint32_t)0x8ebeeff9, (uint32_t)0x17b7be43, (uint32_t)0x60b08ed5,
    (uint32_t)0xd6d6a3e8, (uint32_t)0xa1d1937e, (uint32_t)0x38d8c2c4, (uint32_t)0x4fdff252, (uint32_t)0xd1bb67f1,
    (uint32_t)0xa6bc5767, (uint32_t)0x3fb506dd, (uint32_t)0x48b2364b, (uint32_t)0xd80d2bda, (uint32_t)0xaf0a1b4c,
    (uint32_t)0x36034af6, (uint32_t)0x41047a60, (uint32_t)0xdf60efc3, (uint32_t)0xa867df55, (uint32_t)0x316e8eef,
    (uint32_t)0x4669be79, (uint32_t)0xcb61b38c, (uint32_t)0xbc66831a, (uint32_t)0x256fd2a0, (uint32_t)0x5268e236,
    (uint32_t)0xcc0c7795, (uint32_t)0xbb0b4703, (uint32_t)0x220216b9, (uint32_t)0x5505262f, (uint32_t)0xc5ba3bbe,
    (uint32_t)0xb2bd0b28, (uint32_t)0x2bb45a92, (uint32_t)0x5cb36a04, (uint32_t)0xc2d7ffa7, (uint32_t)0xb5d0cf31,
    (uint32_t)0x2cd99e8b, (uint32_t)0x5bdeae1d, (uint32_t)0x9b64c2b0, (uint32_t)0xec63f226, (uint32_t)0x756aa39c,
    (uint32_t)0x026d930a, (uint32_t)0x9c0906a9, (uint32_t)0xeb0e363f, (uint32_t)0x72076785, (uint32_t)0x05005713,
    (uint32_t)0x95bf4a82, (uint32_t)0xe2b87a14, (uint32_t)0x7bb12bae, (uint32_t)0x0cb61b38, (uint32_t)0x92d28e9b,
    (uint32_t)0xe5d5be0d, (uint32_t)0x7cdcefb7, (uint32_t)0x0bdbdf21, (uint32_t)0x86d3d2d4, (uint32_t)0xf1d4e242,
    (uint32_t)0x68ddb3f8, (uint32_t)0x1fda836e, (uint32_t)0x81be16cd, (uint32_t)0xf6b9265b, (uint32_t)0x6fb077e1,
    (uint32_t)0x18b74777, (uint32_t)0x88085ae6, (uint32_t)0xff0f6a70, (uint32_t)0x66063bca, (uint32_t)0x11010b5c,
    (uint32_t)0x8f659eff, (uint32_t)0xf862ae69, (uint32_t)0x616bffd3, (uint32_t)0x166ccf45, (uint32_t)0xa00ae278,
    (uint32_t)0xd70dd2ee, (uint32_t)0x4e048354, (uint32_t)0x3903b3c2, (uint32_t)0xa7672661, (uint32_t)0xd06016f7,
    (uint32_t)0x4969474d, (uint32_t)0x3e6e77db, (uint32_t)0xaed16a4a, (uint32_t)0xd9d65adc, (uint32_t)0x40df0b66,
    (uint32_t)0x37d83bf0, (uint32_t)0xa9bcae53, (uint32_t)0xdebb9ec5, (uint32_t)0x47b2cf7f, (uint32_t)0x30b5ffe9,
    (uint32_t)0xbdbdf21c, (uint32_t)0xcabac28a, (uint32_t)0x53b39330, (uint32_t)0x24b4a3a6, (uint32_t)0xbad03605,
    (uint32_t)0xcdd70693, (uint32_t)0x54de5729, (uint32_t)0x23d967bf, (uint32_t)0xb3667a2e, (uint32_t)0xc4614ab8,
    (uint32_t)0x5d681b02, (uint32_t)0x2a6f2b94, (uint32_t)0xb40bbe37, (uint32_t)0xc30c8ea1, (uint32_t)0x5a05df1b,
    (uint32_t)0x2d02ef8d
};

#endif /* XCP_CHECKSUM_METHOD */

#define WIDTH  ((uint16_t)(8U * sizeof(Xcp_ChecksumType)))
#define TOPBIT (1 << (WIDTH - 1))

#if (REFLECT_DATA == XCP_TRUE)
    #define CRC_REFLECT_DATA(X) ((uint8_t)reflect((X), 8))
#else
    #define CRC_REFLECT_DATA(X) (X)
#endif

#if (REFLECT_REMAINDER == XCP_TRUE)
    #define CRC_REFLECT_REMAINDER(X) ((Xcp_ChecksumType)reflect((X), WIDTH))
#else
    #define CRC_REFLECT_REMAINDER(X) (X)
#endif

#if (REFLECT_DATA == XCP_TRUE) || (REFLECT_REMAINDER == XCP_TRUE)
static uint32_t reflect(uint32_t data, uint8_t nBits) {
    uint32_t reflection = 0x00000000;
    uint8_t  bit;
    /*
     * Reflect the data about the center bit.
     */
    for (bit = 0; bit < nBits; ++bit) {
        /*
         * If the LSB bit is set, set the reflection of it.
         */
        if (data & 0x01) {
            reflection |= (1 << ((nBits - 1) - bit));
        }
        data = (data >> 1);
    }
    return reflection;
}
#endif /* (REFLECT_DATA == TRUE) || (REFLECT_REMAINDER == TRUE) */

Xcp_ChecksumType Xcp_CalculateChecksum(uint8_t const *ptr, uint32_t length, Xcp_ChecksumType startValue, bool isFirstCall) {
    Xcp_ChecksumType result = 0;
    uint32_t         idx    = 0UL;
#if (XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_CRC_16) || (XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_CRC_16_CITT) ||     \
    (XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_CRC_32)
    uint8_t data = 0;

    if (isFirstCall) {
        result = XCP_CRC_INITIAL_VALUE;
    } else {
        result = startValue;
    }

    for (idx = (uint32_t)0UL; idx < length; ++idx) {
        data   = CRC_REFLECT_DATA(ptr[idx]) ^ UINT8(result >> (WIDTH - UINT8(8)));
        result = READ_CRC_TAB(data) ^ UINT16(result << 8);
    }
    return CRC_REFLECT_REMAINDER(result) ^ XCP_CRC_FINAL_XOR_VALUE;
#elif (XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_ADD_11) || (XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_ADD_12) ||        \
    (XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_ADD_14)
    if (isFirstCall) {
        result = (Xcp_ChecksumType)0;
    } else {
        result = startValue;
    }

    for (idx = (uint32_t)0UL; idx < length; ++idx) {
        result += ptr[idx];
    }

    return result;
#elif (XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_ADD_22) || (XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_ADD_24) ||        \
    (XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_ADD_44)

    #if (XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_ADD_22) || (XCP_CHECKSUM_METHOD == XCP_CHECKSUM_METHOD_XCP_ADD_24)
    uint16_t const *data = (uint16_t const *)ptr; /* Undefined behaviour -  See note above */

    length >>= 1;
    #else
    uint32_t const *data = (uint32_t const *)ptr; /* Undefined behaviour -  See note above */

    length >>= 2;
    #endif /* XCP_CHECKSUM_METHOD */
    if (isFirstCall) {
        result = (Xcp_ChecksumType)0;
    } else {
        result = startValue;
    }

    for (idx = (uint32_t)0UL; idx < length; ++idx) {
        result += data[idx];
    }

    return result;
#endif     /* XCP_CHECKSUM_METHOD */
}

#if XCP_ENABLE_BUILD_CHECKSUM == XCP_ON && XCP_CHECKSUM_CHUNKED_CALCULATION == XCP_ON
static Xcp_ChecksumJobType Xcp_ChecksumJob;

void Xcp_ChecksumInit(void) {
    Xcp_ChecksumJob.mta.address     = UINT32(0ul);
    Xcp_ChecksumJob.mta.ext         = UINT8(0);
    Xcp_ChecksumJob.interimChecksum = (Xcp_ChecksumType)0UL;
    Xcp_ChecksumJob.size            = UINT32(0ul);
    Xcp_ChecksumJob.state           = XCP_CHECKSUM_STATE_IDLE;
}

void Xcp_StartChecksumCalculation(uint8_t const *ptr, uint32_t size) {
    XCP_ENTER_CRITICAL();
    if ((Xcp_ChecksumJob.state != XCP_CHECKSUM_STATE_IDLE) || Xcp_IsBusy()) {
        XCP_LEAVE_CRITICAL();
        return;
    }
    Xcp_SetBusy(XCP_TRUE);
    Xcp_ChecksumJob.state = XCP_CHECKSUM_STATE_RUNNING_INITIAL;
    /* printf("S-Address: %p Size: %u\n", ptr, size); */
    Xcp_ChecksumJob.mta.address = (Xcp_PointerSizeType)ptr;
    Xcp_ChecksumJob.size        = size;
    XCP_LEAVE_CRITICAL();
}

/** @brief Do lengthy checksum/CRC calculations in the background.
 *
 *
 */
void Xcp_ChecksumMainFunction(void) {
    if (Xcp_ChecksumJob.state == XCP_CHECKSUM_STATE_IDLE) {
        return;
    } else if (Xcp_ChecksumJob.state == XCP_CHECKSUM_STATE_RUNNING_INITIAL) {
        Xcp_ChecksumJob.interimChecksum = Xcp_CalculateChecksum(
            (uint8_t const *)Xcp_ChecksumJob.mta.address, XCP_CHECKSUM_CHUNK_SIZE, (Xcp_ChecksumType)0, XCP_TRUE
        );
        Xcp_ChecksumJob.size -= XCP_CHECKSUM_CHUNK_SIZE;
        Xcp_ChecksumJob.mta.address += XCP_CHECKSUM_CHUNK_SIZE;
        Xcp_ChecksumJob.state = XCP_CHECKSUM_STATE_RUNNING_REMAINING;
    } else if (Xcp_ChecksumJob.state == XCP_CHECKSUM_STATE_RUNNING_REMAINING) {
        Xcp_ChecksumJob.interimChecksum = Xcp_CalculateChecksum(
            (uint8_t const *)Xcp_ChecksumJob.mta.address, XCP_CHECKSUM_CHUNK_SIZE, Xcp_ChecksumJob.interimChecksum, XCP_FALSE
        );
        /* printf("N-Address: %x Size: %u CS: %x\n", Xcp_ChecksumJob.mta.address,
         * Xcp_ChecksumJob.size, Xcp_ChecksumJob.interimChecksum); */
        Xcp_ChecksumJob.size -= XCP_CHECKSUM_CHUNK_SIZE;
        Xcp_ChecksumJob.mta.address += XCP_CHECKSUM_CHUNK_SIZE;
        if (Xcp_ChecksumJob.size <= XCP_CHECKSUM_CHUNK_SIZE) {
            Xcp_ChecksumJob.state = XCP_CHECKSUM_STATE_RUNNING_FINAL;
        }
    } else if (Xcp_ChecksumJob.state == XCP_CHECKSUM_STATE_RUNNING_FINAL) {
        /* printf("F-Address: %x Size: %u CS: %x\n", Xcp_ChecksumJob.mta.address,
         * Xcp_ChecksumJob.size, Xcp_ChecksumJob.interimChecksum); */
        Xcp_ChecksumJob.interimChecksum = Xcp_CalculateChecksum(
            (uint8_t const *)Xcp_ChecksumJob.mta.address, Xcp_ChecksumJob.size, Xcp_ChecksumJob.interimChecksum, XCP_FALSE
        );
        /* printf("FINAL-VALUE %x\n", Xcp_ChecksumJob.interimChecksum); */
        Xcp_SetBusy(XCP_FALSE);
        Xcp_SendChecksumPositiveResponse(Xcp_ChecksumJob.interimChecksum);
        Xcp_ChecksumJob.state = XCP_CHECKSUM_STATE_IDLE;
    }
}
#endif /* XCP_ENABLE_BUILD_CHECKSUM */

////////////////////////////////////////////////////////////////////////////////
//                                 xcp_daq.c                                  //
////////////////////////////////////////////////////////////////////////////////

/*
 * BlueParrot XCP
 *
 * (C) 2007-2025 by Christoph Schueler <github.com/Christoph2,
 *                                      cpu12.gems@googlemail.com>
 *
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * s. FLOSS-EXCEPTION.txt
 */

/*
 *
 * TODO: Cleanup DAQ configuration
 *
 */

#if defined(_WIN32)
    #include <stdio.h>
#endif /* _WIN32 */
#include <inttypes.h>


/*
** Private Parameters for now.
*/

/*
** Local Types.
*/
typedef struct {
    uint16_t len;
    uint8_t  data[XCP_MAX_DTO];
} XcpDaq_OdtType;

typedef enum tagXcpDaq_AllocResultType {
    DAQ_ALLOC_OK,
    DAQ_ALLOC_ERR
} XcpDaq_AllocResultType;

typedef enum tagXcpDaq_AllocStateType {
    XCP_ALLOC_IDLE,
    XCP_AFTER_FREE_DAQ,
    XCP_AFTER_ALLOC_DAQ,
    XCP_AFTER_ALLOC_ODT,
    XCP_AFTER_ALLOC_ODT_ENTRY
} XcpDaq_AllocStateType;

typedef enum tagXcpDaq_AllocTransitionType {
    XCP_CALL_FREE_DAQ,
    XCP_CALL_ALLOC_DAQ,
    XCP_CALL_ALLOC_ODT,
    XCP_CALL_ALLOC_ODT_ENTRY
} XcpDaq_AllocTransitionType;

typedef enum tagXcpDaq_ListTransitionType {
    DAQ_LIST_TRANSITION_START,
    DAQ_LIST_TRANSITION_STOP
} XcpDaq_ListTransitionType;

/*
** Local Function-like Macros.
*/
#define XCP_DAQ_MESSAGE_SIZE(msg) UINT16((((msg)->dlc) + sizeof(uint8_t)))

/*
** Local Function Prototypes.
*/
void            XcpDaq_PrintDAQDetails(void);
XCP_STATIC void XcpDaq_StartStopLists(XcpDaq_ListTransitionType transition);
XCP_STATIC void XcpDaq_InitMessageQueue(void);
#if XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON
XCP_STATIC bool                   XcpDaq_AllocValidateTransition(XcpDaq_AllocTransitionType transition);
XCP_STATIC XcpDaq_ListIntegerType XcpDaq_GetDynamicListCount(void);
#endif /* XCP_DAQ_ENABLE_DYNAMIC_LISTS */

/*
** Local Constants.
*/
#if XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON
XCP_STATIC const uint8_t XcpDaq_AllocTransitionTable[5][4] = {
    /* FREE_DAQ           ALLOC_DAQ             ALLOC_ODT ALLOC_ODT_ENTRY */
    /* ALLOC_IDLE*/ { UINT8(DAQ_ALLOC_OK), UINT8(DAQ_ALLOC_ERR), UINT8(DAQ_ALLOC_ERR), UINT8(DAQ_ALLOC_ERR) },
    /* AFTER_FREE_DAQ  */
    { UINT8(DAQ_ALLOC_OK), UINT8(DAQ_ALLOC_OK),  UINT8(DAQ_ALLOC_ERR), UINT8(DAQ_ALLOC_ERR) },
    /* AFTER_ALLOC_DAQ */
    { UINT8(DAQ_ALLOC_OK), UINT8(DAQ_ALLOC_OK),  UINT8(DAQ_ALLOC_OK),  UINT8(DAQ_ALLOC_ERR) },
    /* AFTER_ALLOC_ODT */
    { UINT8(DAQ_ALLOC_OK), UINT8(DAQ_ALLOC_ERR), UINT8(DAQ_ALLOC_OK),  UINT8(DAQ_ALLOC_OK)  },
    /* AFTER_ALLOC_ODT_ENTRY */
    { UINT8(DAQ_ALLOC_OK), UINT8(DAQ_ALLOC_ERR), UINT8(DAQ_ALLOC_ERR), UINT8(DAQ_ALLOC_OK)  },
};
#endif /* XCP_DAQ_ENABLE_DYNAMIC_LISTS */

/*
** Local Variables.
*/

XCP_STATIC XcpDaq_EntityType XcpDaq_Entities[XCP_DAQ_MAX_DYNAMIC_ENTITIES];

#if XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON
XCP_STATIC XcpDaq_AllocStateType        XcpDaq_AllocState;
XCP_STATIC XcpDaq_ListStateType         XcpDaq_ListState;
XCP_STATIC XcpDaq_ListConfigurationType XcpDaq_ListConfiguration;
XCP_STATIC XCP_DAQ_ENTITY_TYPE          XcpDaq_EntityCount = (XCP_DAQ_ENTITY_TYPE)0;
XCP_STATIC XCP_DAQ_ENTITY_TYPE          XcpDaq_ListCount   = (XCP_DAQ_ENTITY_TYPE)0;
XCP_STATIC XCP_DAQ_ENTITY_TYPE          XcpDaq_OdtCount    = (XCP_DAQ_ENTITY_TYPE)0;
#endif /* XCP_DAQ_ENABLE_DYNAMIC_LISTS */

#if XCP_DAQ_ENABLE_QUEUING == XCP_ON
XCP_STATIC XcpDaq_QueueType XcpDaq_Queue                         = { 0 };
XCP_STATIC XcpDaq_OdtType   XcpDaq_QueueDTOs[XCP_DAQ_QUEUE_SIZE] = { 0 };
#endif /* XCP_DAQ_ENABLE_QUEUING */

#if XCP_DAQ_ENABLE_MULTIPLE_DAQ_LISTS_PER_EVENT == XCP_OFF
XCP_STATIC uint8_t XcpDaq_ListForEvent[XCP_DAQ_MAX_EVENT_CHANNEL];
#else
    #error XCP_DAQ_ENABLE_MULTIPLE_DAQ_LISTS_PER_EVENT option currently not supported
#endif /* XCP_DAQ_ENABLE_MULTIPLE_DAQ_LISTS_PER_EVENT */

/*
**
** Global Variables.
**
*/

/*
**
** Global Functions.
**
*/

/*
** Dynamic DAQ related functions.
*/
#if XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON
Xcp_ReturnType XcpDaq_Free(void) {
    Xcp_ReturnType result = ERR_SUCCESS;

    XcpDaq_EntityCount = (XCP_DAQ_ENTITY_TYPE)0;
    XcpDaq_ListCount   = (XCP_DAQ_ENTITY_TYPE)0;
    XcpDaq_OdtCount    = (XCP_DAQ_ENTITY_TYPE)0;

    #if XCP_DAQ_ENABLE_MULTIPLE_DAQ_LISTS_PER_EVENT == XCP_OFF
    XcpUtl_MemSet(XcpDaq_ListForEvent, UINT8(0), UINT32(sizeof(XcpDaq_ListForEvent[0]) * UINT8(XCP_DAQ_MAX_EVENT_CHANNEL)));
    #endif /* XCP_DAQ_ENABLE_MULTIPLE_DAQ_LISTS_PER_EVENT */

    if (XcpDaq_AllocValidateTransition(XCP_CALL_FREE_DAQ)) {
        XcpUtl_MemSet(
            XcpDaq_Entities, UINT8(0), UINT32(sizeof(XcpDaq_EntityType) * (XCP_DAQ_ENTITY_TYPE)XCP_DAQ_MAX_DYNAMIC_ENTITIES)
        );
        XcpDaq_AllocState = XCP_AFTER_FREE_DAQ;
    } else {
        result = ERR_SEQUENCE; /* Never touched; function always succeeds. */
    }
    return result;
}

Xcp_ReturnType XcpDaq_Alloc(XcpDaq_ListIntegerType daqCount) {
    XCP_DAQ_ENTITY_TYPE idx;
    Xcp_ReturnType      result = ERR_SUCCESS;

    if (!XcpDaq_AllocValidateTransition(XCP_CALL_ALLOC_DAQ)) {
    #if XCP_DAQ_ENABLE_RESET_DYN_DAQ_CONFIG_ON_SEQUENCE_ERROR == XCP_ON
        XcpDaq_Init();
    #endif /* XCP_DAQ_ENABLE_RESET_DYN_DAQ_CONFIG_ON_SEQUENCE_ERROR */
        result = ERR_SEQUENCE;
    } else {
        if ((XcpDaq_EntityCount + daqCount) <= (XCP_DAQ_ENTITY_TYPE)XCP_DAQ_MAX_DYNAMIC_ENTITIES) {
            XcpDaq_AllocState = XCP_AFTER_ALLOC_DAQ;
            for (idx = XcpDaq_EntityCount; idx < (XcpDaq_EntityCount + daqCount); ++idx) {
                XcpDaq_Entities[idx].kind                   = UINT8(XCP_ENTITY_DAQ_LIST);
                XcpDaq_Entities[idx].entity.daqList.numOdts = (XcpDaq_ODTIntegerType)0;
            }
            XcpDaq_ListCount += daqCount;
            XcpDaq_EntityCount += daqCount;
        } else {
            result = ERR_MEMORY_OVERFLOW;
        }
    }
    return result;
}

Xcp_ReturnType XcpDaq_AllocOdt(XcpDaq_ListIntegerType daqListNumber, XcpDaq_ODTIntegerType odtCount) {
    XCP_DAQ_ENTITY_TYPE idx;
    Xcp_ReturnType      result = ERR_SUCCESS;

    if (!XcpDaq_AllocValidateTransition(XCP_CALL_ALLOC_ODT)) {
    #if XCP_DAQ_ENABLE_RESET_DYN_DAQ_CONFIG_ON_SEQUENCE_ERROR == XCP_ON
        XcpDaq_Init();
    #endif /* XCP_DAQ_ENABLE_RESET_DYN_DAQ_CONFIG_ON_SEQUENCE_ERROR */
        result = ERR_SEQUENCE;
    } else {
        if ((XcpDaq_EntityCount + odtCount) <= (XCP_DAQ_ENTITY_TYPE)XCP_DAQ_MAX_DYNAMIC_ENTITIES) {
            XcpDaq_AllocState = XCP_AFTER_ALLOC_ODT;
            for (idx = XcpDaq_EntityCount; idx < (XcpDaq_EntityCount + odtCount); ++idx) {
                XcpDaq_Entities[idx].kind = UINT8(XCP_ENTITY_ODT);
            }
            if (XcpDaq_Entities[daqListNumber].entity.daqList.numOdts == 0) {
                XcpDaq_Entities[daqListNumber].entity.daqList.firstOdt = XcpDaq_EntityCount;
            }
            XcpDaq_Entities[daqListNumber].entity.daqList.numOdts += odtCount;
            XcpDaq_OdtCount += odtCount;
            XcpDaq_EntityCount += odtCount;
        } else {
            result = ERR_MEMORY_OVERFLOW;
        }
    }
    return result;
}

Xcp_ReturnType XcpDaq_AllocOdtEntry(
    XcpDaq_ListIntegerType daqListNumber, XcpDaq_ODTIntegerType odtNumber, XcpDaq_ODTEntryIntegerType odtEntriesCount
) {
    XCP_DAQ_ENTITY_TYPE   idx;
    XcpDaq_ODTIntegerType odt;
    Xcp_ReturnType        result = ERR_SUCCESS;

    if (!XcpDaq_AllocValidateTransition(XCP_CALL_ALLOC_ODT_ENTRY)) {
    #if XCP_DAQ_ENABLE_RESET_DYN_DAQ_CONFIG_ON_SEQUENCE_ERROR == XCP_ON
        XcpDaq_Init();
    #endif /* XCP_DAQ_ENABLE_RESET_DYN_DAQ_CONFIG_ON_SEQUENCE_ERROR */
        result = ERR_SEQUENCE;
    } else {
        if ((XcpDaq_EntityCount + odtEntriesCount) <= (XCP_DAQ_ENTITY_TYPE)XCP_DAQ_MAX_DYNAMIC_ENTITIES) {
            XcpDaq_AllocState = XCP_AFTER_ALLOC_ODT_ENTRY;
            for (idx = XcpDaq_EntityCount; idx < (XcpDaq_EntityCount + odtEntriesCount); ++idx) {
                XcpDaq_Entities[idx].kind = UINT8(XCP_ENTITY_ODT_ENTRY);
            }
            odt = (XcpDaq_ODTIntegerType)(XcpDaq_Entities[daqListNumber].entity.daqList.firstOdt + UINT16(odtNumber));
            XcpDaq_Entities[odt].entity.odt.firstOdtEntry = XcpDaq_EntityCount;
            XcpDaq_Entities[odt].entity.odt.numOdtEntries = odtEntriesCount;
            XcpDaq_EntityCount += (XCP_DAQ_ENTITY_TYPE)odtEntriesCount;
        } else {
            result = ERR_MEMORY_OVERFLOW;
        }
    }
    return result;
}

XCP_STATIC bool XcpDaq_AllocValidateTransition(XcpDaq_AllocTransitionType transition) {
    /* printf("STATE: %u TRANSITION: %u\n", XcpDaq_AllocState, transition); */
    if (XcpDaq_AllocTransitionTable[XcpDaq_AllocState][transition] == UINT8(DAQ_ALLOC_OK)) {
        return (bool)XCP_TRUE;
    } else {
        return (bool)XCP_FALSE;
    }
}

XCP_STATIC XcpDaq_ListIntegerType XcpDaq_GetDynamicListCount(void) {
    return (XcpDaq_ListIntegerType)XcpDaq_ListCount;
}

XCP_DAQ_ENTITY_TYPE XcpDaq_GetDynamicDaqEntityCount(void) {
    return XcpDaq_EntityCount;
}
#endif /* XCP_DAQ_ENABLE_DYNAMIC_LISTS */

void XcpDaq_Init(void) {
#if XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_ON
    XcpDaq_ListIntegerType idx = 0;
#endif /* XCP_DAQ_ENABLE_PREDEFINED_LISTS */

    XcpDaq_StopAllLists();
    XcpDaq_SetProcessorState(XCP_DAQ_STATE_STOPPED);
    XcpDaq_SetPointer(0, 0, 0);
#if XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_ON
    for (idx = (XcpDaq_ListIntegerType)0; idx < XcpDaq_PredefinedListCount; ++idx) {
        XcpDaq_PredefinedListsState[idx].mode = UINT8(0);
    #if XCP_DAQ_ENABLE_PRESCALER == XCP_ON
        XcpDaq_PredefinedListsState[idx].prescaler = UINT8(1);
        XcpDaq_PredefinedListsState[idx].counter   = UINT8(0);
    #endif /* XCP_DAQ_ENABLE_PRESCALER */
    }
#endif /* XCP_DAQ_ENABLE_PREDEFINED_LISTS */

    /* Always reset the event-to-list mapping on init if single mapping is used */
#if XCP_DAQ_ENABLE_MULTIPLE_DAQ_LISTS_PER_EVENT == XCP_OFF
    XcpUtl_MemSet(XcpDaq_ListForEvent, UINT8(0), UINT32(sizeof(XcpDaq_ListForEvent[0]) * UINT8(XCP_DAQ_MAX_EVENT_CHANNEL)));
#endif /* XCP_DAQ_ENABLE_MULTIPLE_DAQ_LISTS_PER_EVENT */

#if XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON
    XcpDaq_AllocState = XCP_ALLOC_IDLE;
    (void)XcpDaq_Free();
#endif /* XCP_DAQ_ENABLE_DYNAMIC_LISTS */

#if XCP_DAQ_ENABLE_QUEUING == XCP_ON
    XcpDaq_QueueInit();
#endif
}

XcpDaq_ODTEntryType XcpDaq_GetOdtEntryValues(
    XcpDaq_ListIntegerType daqListNumber, XcpDaq_ODTIntegerType odtNumber, XcpDaq_ODTEntryIntegerType odtEntryNumber
) {
    XcpDaq_ODTEntryType *entry  = XcpDaq_GetOdtEntry(daqListNumber, odtNumber, odtEntryNumber);
    XcpDaq_ODTEntryType  result = {};
#if XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_ON
    uint32_t index = 0UL;
#endif /* XCP_DAQ_ENABLE_PREDEFINED_LISTS */

    if (entry == XCP_NULL) {
        /* Out-of-range or unavailable entry; return empty result */
        return result;
    }

#if (XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON) && (XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_OFF)
    /* Dynamic DAQs only */
    result = *entry;
#elif (XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_OFF) && (XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_ON)
    /* Predefined DAQs only */
    result = *entry;
#elif (XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON) && (XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_ON)
    /* Dynamic and predefined DAQs */
    if (daqListNumber >= XcpDaq_PredefinedListCount) {
        result = *entry;
    } else {
        index  = entry->mta.address;
        result = XcpDaq_PredefinedMeasurementVariables[index];
    }
#endif  // XCP_DAQ_ENABLE_DYNAMIC_LISTS
    return result;
}

XcpDaq_ODTEntryType *XcpDaq_GetOdtEntry(
    XcpDaq_ListIntegerType daqListNumber, XcpDaq_ODTIntegerType odtNumber, XcpDaq_ODTEntryIntegerType odtEntryNumber
) {
    XcpDaq_ODTType const *odt = XCP_NULL;
    XcpDaq_ODTIntegerType idx = 0;

    /* Range checking. */
    if (daqListNumber >= XcpDaq_GetListCount()) {
        return XCP_NULL;
    }
    {
        XcpDaq_ListConfigurationType const *listConf = XcpDaq_GetListConfiguration(daqListNumber);
        if ((listConf == XCP_NULL) || (odtNumber >= listConf->numOdts)) {
            return XCP_NULL;
        }
    }
    odt = XcpDaq_GetOdt(daqListNumber, odtNumber);
    if (odt == XCP_NULL) {
        return XCP_NULL;
    }
    if (odtEntryNumber >= odt->numOdtEntries) {
        return XCP_NULL;
    }
    idx = (XcpDaq_ODTIntegerType)(odt->firstOdtEntry + UINT16(odtEntryNumber));
#if (XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON) && (XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_OFF)
    /* Dynamic DAQs only */
    return &XcpDaq_Entities[idx].entity.odtEntry;
#elif (XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_OFF) && (XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_ON)
    /* Predefined DAQs only */
    return (XcpDaq_ODTEntryType *)&XcpDaq_PredefinedMeasurementVariables[XcpDaq_PredefinedOdtEntries[idx].mta.address];
#elif (XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON) && (XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_ON)
    /* Dynamic and predefined DAQs */
    if (daqListNumber >= XcpDaq_PredefinedListCount) {
        return &XcpDaq_Entities[idx].entity.odtEntry;
    } else {
        return (XcpDaq_ODTEntryType *)&XcpDaq_PredefinedMeasurementVariables[XcpDaq_PredefinedOdtEntries[idx].mta.address];
    }
#endif  // XCP_DAQ_ENABLE_DYNAMIC_LISTS
}

XcpDaq_ListConfigurationType const *XcpDaq_GetListConfiguration(XcpDaq_ListIntegerType daqListNumber) {
#if XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON

#endif  // XCP_DAQ_ENABLE_DYNAMIC_LISTS

    // printf("XcpDaq_GetListConfiguration(%u)\n", daqListNumber);
#if (XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON) && (XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_OFF)
    /* Dynamic DAQs only */
    XcpDaq_DynamicListType const *dl = &XcpDaq_Entities[daqListNumber].entity.daqList;

    XcpDaq_ListConfiguration.firstOdt = dl->firstOdt;
    XcpDaq_ListConfiguration.numOdts  = dl->numOdts;

    return &XcpDaq_ListConfiguration;
#elif (XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_OFF) && (XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_ON)
    /* Predefined DAQs only */
    return &XcpDaq_PredefinedLists[daqListNumber];
#elif (XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON) && (XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_ON)
    /* Dynamic and predefined DAQs */
    if (daqListNumber >= XcpDaq_PredefinedListCount) {
        XcpDaq_DynamicListType const *dl = &XcpDaq_Entities[daqListNumber].entity.daqList;

        XcpDaq_ListConfiguration.firstOdt = dl->firstOdt;
        XcpDaq_ListConfiguration.numOdts  = dl->numOdts;

        return &XcpDaq_ListConfiguration;
    } else {
        return &XcpDaq_PredefinedLists[daqListNumber];
    }
#endif
}

XcpDaq_ListStateType *XcpDaq_GetListState(XcpDaq_ListIntegerType daqListNumber) {
#if XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON

#endif /* XCP_DAQ_ENABLE_DYNAMIC_LISTS */

    /* printf("XcpDaq_GetListState() number: %u\n", daqListNumber); */
#if (XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON) && (XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_OFF)
    /* Dynamic DAQs only */
    return &XcpDaq_ListState;

#elif (XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_OFF) && (XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_ON)
    /* Predefined DAQs only */
    return &XcpDaq_PredefinedListsState[daqListNumber];
#elif (XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON) && (XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_ON)
    /* Dynamic and predefined DAQs */
    if (daqListNumber >= XcpDaq_PredefinedListCount) {
        XcpDaq_DynamicListType *dl = &XcpDaq_Entities[daqListNumber].entity.daqList;

        XcpDaq_ListState.mode = dl->mode;

        return &XcpDaq_ListState;
    } else {
        return &XcpDaq_PredefinedListsState[daqListNumber];
    }
#endif
}

void XcpDaq_SetListMode(
    XcpDaq_ListIntegerType daqListNumber, uint8_t mode, uint16_t eventChannelNumber, uint8_t prescaler, uint8_t priority
) {
    XcpDaq_ListStateType *entry = XcpDaq_GetListState(daqListNumber);

    XcpDaq_AddEventChannel(daqListNumber, eventChannelNumber);
    entry->mode = XcpUtl_SetResetBit8(entry->mode, mode, XCP_DAQ_LIST_MODE_TIMESTAMP);
    entry->mode = XcpUtl_SetResetBit8(entry->mode, mode, XCP_DAQ_LIST_MODE_ALTERNATING);
    entry->mode = XcpUtl_SetResetBit8(entry->mode, mode, XCP_DAQ_LIST_MODE_DIRECTION);
    entry->mode = XcpUtl_SetResetBit8(entry->mode, mode, XCP_DAQ_LIST_MODE_PID_OFF);
    entry->mode = XcpUtl_SetResetBit8(entry->mode, mode, XCP_DAQ_LIST_MODE_SELECTED);
    entry->mode = XcpUtl_SetResetBit8(entry->mode, mode, XCP_DAQ_LIST_MODE_STARTED);

#if XCP_DAQ_ENABLE_PRESCALER == XCP_ON
    entry->prescaler = prescaler;
#endif /* XCP_DAQ_ENABLE_PRESCALER */
}

void XcpDaq_SetPointer(
    XcpDaq_ListIntegerType daqListNumber, XcpDaq_ODTIntegerType odtNumber, XcpDaq_ODTEntryIntegerType odtEntryNumber
) {
    Xcp_StateType *Xcp_State = XCP_NULL;

    Xcp_State                      = Xcp_GetState();
    Xcp_State->daqPointer.daqList  = daqListNumber;
    Xcp_State->daqPointer.odt      = odtNumber;
    Xcp_State->daqPointer.odtEntry = odtEntryNumber;
}

void XcpDaq_WriteEntry(uint8_t bitOffset, uint8_t elemSize, uint8_t adddrExt, Xcp_PointerSizeType address) {
    XcpDaq_ODTEntryType *entry     = XCP_NULL;
    Xcp_StateType       *Xcp_State = XCP_NULL;

    Xcp_State = Xcp_GetState();

    DBG_TRACE(
        "\nXcpDaq_WriteEntry: [address: 0x%" PRIxPTR " ext: 0x%02x size: %u bitOffset: %u]\n\r",
        (uintptr_t)address, adddrExt, elemSize, bitOffset
    );

    entry = XcpDaq_GetOdtEntry(Xcp_State->daqPointer.daqList, Xcp_State->daqPointer.odt, Xcp_State->daqPointer.odtEntry);
    if (entry == XCP_NULL) {
        /* Invalid pointer (out of range or unavailable due to configuration). */
        return;
    }

#if XCP_DAQ_BIT_OFFSET_SUPPORTED == XCP_ON
    entry->bitOffset = bitOffset;
#endif /* XCP_DAQ_ENABLE_BIT_OFFSET */
    entry->length      = elemSize;
    entry->mta.address = address;
#if XCP_DAQ_ADDR_EXT_SUPPORTED == XCP_ON
    entry->mta.ext = adddrExt;
#endif /* XCP_DAQ_ENABLE_ADDR_EXT */

    /* Advance ODT entry pointer within one and the same ODT.
     * After writing to the last ODT entry of an ODT, the value
     * of the DAQ pointer is undefined!
     */
    Xcp_State->daqPointer.odtEntry += (XcpDaq_ODTEntryIntegerType)1;
}

void XcpDaq_ReadEntry(uint8_t *bitOffset, uint8_t *elemSize, uint8_t *adddrExt, Xcp_PointerSizeType *address) {
    XcpDaq_ODTEntryType *entry     = XCP_NULL;
    Xcp_StateType       *Xcp_State = XCP_NULL;

    Xcp_State = Xcp_GetState();

    entry = XcpDaq_GetOdtEntry(Xcp_State->daqPointer.daqList, Xcp_State->daqPointer.odt, Xcp_State->daqPointer.odtEntry);
    if (entry == XCP_NULL) {
        /* Invalid pointer; return zeroed outputs and do not advance pointer. */
        if (bitOffset) {
            *bitOffset = 0;
        }
        if (elemSize) {
            *elemSize = 0;
        }
        if (adddrExt) {
            *adddrExt = 0;
        }
        if (address) {
            *address = 0;
        }
        return;
    }

#if XCP_DAQ_BIT_OFFSET_SUPPORTED == XCP_ON
    *bitOffset = entry->bitOffset;
#else
    *bitOffset = 0;
#endif /* XCP_DAQ_ENABLE_BIT_OFFSET */
    *elemSize = entry->length;
    *address  = entry->mta.address;
#if XCP_DAQ_ADDR_EXT_SUPPORTED == XCP_ON
    *adddrExt = entry->mta.ext;
#else
    *adddrExt = 0;
#endif /* XCP_DAQ_ENABLE_ADDR_EXT */

    DBG_TRACE(
        "\nXcpDaq_ReadEntry: [address: 0x%" PRIxPTR " ext: 0x%02x size: %u bitOffset: %u]\n\r",
        (uintptr_t)*address, *adddrExt, *elemSize, *bitOffset
    );

    Xcp_State->daqPointer.odtEntry += (XcpDaq_ODTEntryIntegerType)1;  // s. `XcpDaq_WriteEntry` comment.
}

bool XcpDaq_ValidateConfiguration(void) {
#if (XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON) && (XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_OFF)
    /* Dynamic DAQs only */
    return (bool)((XcpDaq_EntityCount > (XCP_DAQ_ENTITY_TYPE)0) && (XcpDaq_ListCount > (XCP_DAQ_ENTITY_TYPE)0) &&
                  (XcpDaq_OdtCount > (XCP_DAQ_ENTITY_TYPE)0));
#elif (XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_OFF) && (XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_ON)
    /* Predefined DAQs only */
    return (bool)XCP_TRUE;
#elif (XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON) && (XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_ON)
    /* Dynamic and predefined DAQs */
    /* Valid if we have any predefined lists or a non-empty dynamic configuration */
    if (XcpDaq_PredefinedListCount > 0U) {
        return (bool)XCP_TRUE;
    }
    return (bool)((XcpDaq_EntityCount > (XCP_DAQ_ENTITY_TYPE)0) && (XcpDaq_ListCount > (XCP_DAQ_ENTITY_TYPE)0) &&
                  (XcpDaq_OdtCount > (XCP_DAQ_ENTITY_TYPE)0));
#endif
}

XcpDaq_ListIntegerType XcpDaq_GetListCount(void) {
#if (XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON) && (XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_OFF)
    /* Dynamic DAQs only */
    return XcpDaq_GetDynamicListCount();
#elif (XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_OFF) && (XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_ON)
    /* Predefined DAQs only */
    return XcpDaq_PredefinedListCount;
#elif (XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON) && (XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_ON)
    /* Dynamic and predefined DAQs */
    return XcpDaq_PredefinedListCount + XcpDaq_GetDynamicListCount();
#endif
}

bool XcpDaq_ValidateList(XcpDaq_ListIntegerType daqListNumber) {
    XcpDaq_ListConfigurationType const *daqList = XCP_NULL;
    XcpDaq_ODTType const               *odt     = XCP_NULL;
    bool                                result  = (bool)XCP_TRUE;
    XcpDaq_ODTIntegerType               numOdts = 0;
    uint8_t                             idx     = 0;

    if (daqListNumber > (XcpDaq_GetListCount() - UINT16(1))) {
        result = (bool)XCP_FALSE;
    } else {
        daqList = XcpDaq_GetListConfiguration(daqListNumber);
        numOdts = daqList->numOdts;
        if (numOdts == UINT8(0)) {
            result = (bool)XCP_FALSE;
        } else {
            result = (bool)XCP_FALSE;
            for (idx = UINT8(0); idx < numOdts; ++idx) {
                odt = XcpDaq_GetOdt(daqListNumber, idx);
                if (odt->numOdtEntries != UINT8(0)) {
                    result = (bool)XCP_TRUE;
                    break;
                }
            }
        }
    }
    return result;
}

bool XcpDaq_ValidateOdtEntry(
    XcpDaq_ListIntegerType daqListNumber, XcpDaq_ODTIntegerType odtNumber, XcpDaq_ODTEntryIntegerType odtEntry
) {
    XcpDaq_ListConfigurationType const *daqList = XCP_NULL;
    XcpDaq_ODTType const               *odt     = XCP_NULL;
    bool                                result  = (bool)XCP_TRUE;

    if (daqListNumber > (XcpDaq_GetListCount() - UINT16(1))) {
        result = (bool)XCP_FALSE;
    } else {
        daqList = XcpDaq_GetListConfiguration(daqListNumber);
        if (odtNumber > (daqList->numOdts - (XcpDaq_ODTIntegerType)1)) {
            result = (bool)XCP_FALSE;
        } else {
            odt = XcpDaq_GetOdt(daqListNumber, odtNumber);
            if (odtEntry > (odt->numOdtEntries - (XcpDaq_ODTEntryIntegerType)1)) {
                result = (bool)XCP_FALSE;
            }
        }
    }
    return result;
}

XcpDaq_EventType const *XcpDaq_GetEventConfiguration(uint16_t eventChannelNumber) {
    if (eventChannelNumber >= UINT8(XCP_DAQ_MAX_EVENT_CHANNEL)) {
        return (XcpDaq_EventType const *)XCP_NULL;
    }
    return &XcpDaq_Events[eventChannelNumber];
}

void XcpDaq_AddEventChannel(XcpDaq_ListIntegerType daqListNumber, uint16_t eventChannelNumber) {
#if XCP_DAQ_ENABLE_MULTIPLE_DAQ_LISTS_PER_EVENT == XCP_OFF
    if (eventChannelNumber < UINT8(XCP_DAQ_MAX_EVENT_CHANNEL)) {
        XcpDaq_ListForEvent[eventChannelNumber] = daqListNumber;
    }
#endif /* XCP_DAQ_ENABLE_MULTIPLE_DAQ_LISTS_PER_EVENT */
}

/** @brief Triggers acquisition and transmission of DAQ lists.
 *
 *  @param eventChannelNumber   Number of event to trigger.
 */
void XcpDaq_TriggerEvent(uint8_t eventChannelNumber) {
    Xcp_StateType const       *state         = XCP_NULL;
    XcpDaq_ListIntegerType     daqListNumber = 0;
    XcpDaq_ODTIntegerType      odtIdx        = 0;
    XcpDaq_ODTIntegerType      pid           = 0;
    XcpDaq_ODTEntryIntegerType odtEntryIdx   = 0;
    XcpDaq_ODTType const      *odt           = XCP_NULL;
    // XcpDaq_ODTEntryType                *entry             = XCP_NULL;
    XcpDaq_ODTEntryType                 entry;
    XcpDaq_ListConfigurationType const *listConf          = XCP_NULL;
    XcpDaq_ListStateType               *listState         = XCP_NULL;
    uint16_t                            offset            = UINT16(0);
    uint8_t                             data[XCP_MAX_DTO] = { 0 };
#if XCP_DAQ_ENABLE_TIMESTAMPING == XCP_ON
    uint32_t timestamp        = UINT32(0);
    bool     insert_timestamp = XCP_FALSE;

    timestamp = XcpHw_GetTimerCounter();
#endif /* XCP_DAQ_ENABLE_TIMESTAMPING */
    state = Xcp_GetState();
    if (state->daqProcessor.state != XCP_DAQ_STATE_RUNNING) {
        return;
    }
    if (eventChannelNumber >= UINT8(XCP_DAQ_MAX_EVENT_CHANNEL)) {
        return;
    }

#if XCP_DAQ_ENABLE_MULTIPLE_DAQ_LISTS_PER_EVENT == XCP_OFF
    daqListNumber = XcpDaq_ListForEvent[eventChannelNumber];
#endif /* XCP_DAQ_ENABLE_MULTIPLE_DAQ_LISTS_PER_EVENT */

    /* Ensure DAQ list is started and handle prescaler if configured */
    listState = XcpDaq_GetListState(daqListNumber);
    if ((listState == XCP_NULL) || ((listState->mode & XCP_DAQ_LIST_MODE_STARTED) != XCP_DAQ_LIST_MODE_STARTED)) {
        return;
    }
#if XCP_DAQ_ENABLE_PRESCALER == XCP_ON
    if (listState->prescaler > UINT8(1)) {
        listState->counter++;
        if (listState->counter < listState->prescaler) {
            return; /* Not yet time to send */
        }
        listState->counter = UINT8(0);
    }
#endif /* XCP_DAQ_ENABLE_PRESCALER */

    if (!XcpDaq_GetFirstPid(daqListNumber, &pid)) {
        return;
    }
    listConf = XcpDaq_GetListConfiguration(daqListNumber);
#if XCP_DAQ_ENABLE_TIMESTAMPING == XCP_ON
    if ((listState->mode & XCP_DAQ_LIST_MODE_TIMESTAMP) == XCP_DAQ_LIST_MODE_TIMESTAMP) {
        insert_timestamp = XCP_TRUE;
    }
#endif /* XCP_DAQ_ENABLE_TIMESTAMPING */
    for (odtIdx = (XcpDaq_ODTIntegerType)0; odtIdx < listConf->numOdts; ++odtIdx) {
        offset = UINT16(0);
        odt    = XcpDaq_GetOdt(daqListNumber, odtIdx);

        data[0] = pid; /* Absolute ODT number. */
        offset += UINT16(1);
#if XCP_DAQ_ENABLE_TIMESTAMPING == XCP_ON
        if ((odtIdx == (XcpDaq_ODTIntegerType)0) && (insert_timestamp == XCP_TRUE)) {  // TODO: check for sufficient space.
            XcpDaq_CopyMemory(&data[offset], (void *)&timestamp, XCP_DAQ_TIMESTAMP_SIZE);
            offset += XCP_DAQ_TIMESTAMP_SIZE;
        }
#endif /* XCP_DAQ_ENABLE_TIMESTAMPING */
        for (odtEntryIdx = (XcpDaq_ODTEntryIntegerType)0; odtEntryIdx < odt->numOdtEntries; ++odtEntryIdx) {
            entry = XcpDaq_GetOdtEntryValues(daqListNumber, odtIdx, odtEntryIdx);
            // XCP_ASSERT_LE(entry.length, (unsigned int)(XCP_MAX_DTO - offset));
            if (entry.length > (unsigned int)(XCP_MAX_DTO - offset)) {
                return;  // Error: Out of memory.
            }

            XcpDaq_CopyMemory(&data[offset], (void *)entry.mta.address, entry.length);
            offset += entry.length;
        }
        pid++;
        XcpDaq_QueueEnqueue(offset, data);
        //        XcpUtl_Hexdump(data, offset);
    }
    XcpDaq_TransmitDtos();
}

/** @brief Copies bytes from a source memory area to a destination memory area,
 *   where both areas may not overlap.
 *  @param[out] dst  The memory area to copy to.
 *  @param[in]  src  The memory area to copy from.
 *  @param[in]  len  The number of bytes to copy
 */
void XcpDaq_CopyMemory(void *dst, void const *src, uint32_t len) {
    XcpUtl_MemCopy(dst, src, len);
}

void XcpDaq_GetProperties(uint8_t *properties) {
    *properties = UINT8(0);
#if XCP_DAQ_ENABLE_PRESCALER == XCP_ON
    *properties |= XCP_DAQ_PROP_PRESCALER_SUPPORTED;
#endif /*XCP_DAQ_ENABLE_PRESCALER */

#if XCP_DAQ_ENABLE_TIMESTAMPING == XCP_ON
    *properties |= XCP_DAQ_PROP_TIMESTAMP_SUPPORTED;
#endif /*XCP_DAQ_ENABLE_TIMESTAMPING */

#if (XCP_DAQ_CONFIG_TYPE == XCP_DAQ_CONFIG_TYPE_NONE) || (XCP_DAQ_CONFIG_TYPE == XCP_DAQ_CONFIG_TYPE_STATIC)
    *properties |= UINT8(XCP_DAQ_CONFIG_TYPE_STATIC);
#elif XCP_DAQ_CONFIG_TYPE == XCP_DAQ_CONFIG_TYPE_DYNAMIC
    *properties |= UINT8(XCP_DAQ_CONFIG_TYPE_NONE);
#endif
}

void XcpDaq_SetProcessorState(XcpDaq_ProcessorStateType state) {
    Xcp_StateType *stateVar = XCP_NULL;

    stateVar = Xcp_GetState();
    XCP_DAQ_ENTER_CRITICAL();
    stateVar->daqProcessor.state = state;
    XCP_DAQ_LEAVE_CRITICAL();
}

void XcpDaq_StartSelectedLists(void) {
    XcpDaq_StartStopLists(DAQ_LIST_TRANSITION_START);
}

void XcpDaq_StopSelectedLists(void) {
    XcpDaq_StartStopLists(DAQ_LIST_TRANSITION_STOP);
}

void XcpDaq_StopAllLists(void) {
    XcpDaq_StartStopLists(DAQ_LIST_TRANSITION_STOP);
}

void XcpDaq_StartStopSingleList(XcpDaq_ListIntegerType daqListNumber, uint8_t mode) {
    XcpDaq_ListStateType *list_state = XCP_NULL;

    list_state = XcpDaq_GetListState(daqListNumber);

    if (mode == UINT8(0)) {
        /* Stop DAQ list immediately */
        list_state->mode &= UINT8(~XCP_DAQ_LIST_MODE_STARTED);
    } else if (mode == UINT8(1)) {
        /* Start DAQ list immediately */
        list_state->mode |= XCP_DAQ_LIST_MODE_STARTED;
    } else if (mode == UINT8(2)) {
        /* Select DAQ list for synchronized start/stop */
        list_state->mode |= XCP_DAQ_LIST_MODE_SELECTED;
    }
}

void XcpDaq_StartStopSynch(uint8_t mode) {
    if (mode == XCP_DAQ_LISTS_START_SELECTED) {
        XcpDaq_StartSelectedLists();
        XcpDaq_SetProcessorState(XCP_DAQ_STATE_RUNNING);
    } else if (mode == XCP_DAQ_LISTS_STOP_ALL) {
        XcpDaq_StopAllLists();
        XcpDaq_SetProcessorState(XCP_DAQ_STATE_STOPPED);
    } else if (mode == XCP_DAQ_LISTS_STOP_SELECTED) {
        XcpDaq_StopSelectedLists();
    }
}

void XcpDaq_TransmitDtos(void) {
    uint16_t len     = 0UL;
    uint8_t *dataOut = Xcp_GetDtoOutPtr();

    while (!XcpDaq_QueueEmpty()) {
        if (XcpDaq_QueueDequeue(&len, dataOut)) {
            Xcp_SetDtoOutLen(len);
            Xcp_SendDto();
        }
    }
}

/*
** Local Functions.
*/
XcpDaq_ODTType const *XcpDaq_GetOdt(XcpDaq_ListIntegerType daqListNumber, XcpDaq_ODTIntegerType odtNumber) {
    XcpDaq_ListConfigurationType const *dl  = XCP_NULL;
    XcpDaq_ODTIntegerType               idx = 0;

    /* Guard against invalid DAQ list or ODT index */
    if (daqListNumber >= XcpDaq_GetListCount()) {
        return XCP_NULL;
    }
    dl = XcpDaq_GetListConfiguration(daqListNumber);
    if ((dl == XCP_NULL) || (odtNumber >= dl->numOdts)) {
        return XCP_NULL;
    }
    idx = (XcpDaq_ODTIntegerType)dl->firstOdt + odtNumber;
#if (XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON) && (XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_OFF)
    /* Dynamic DAQs only */
    return &XcpDaq_Entities[idx].entity.odt;
#elif (XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_OFF) && (XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_ON)
    /* Predefined DAQs only */
    return &XcpDaq_PredefinedOdts[idx];
#elif (XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON) && (XCP_DAQ_ENABLE_PREDEFINED_LISTS == XCP_ON)
    /* Dynamic and predefined DAQs */
    if (daqListNumber >= XcpDaq_PredefinedListCount) {
        return &XcpDaq_Entities[idx].entity.odt;
    } else {
        return &XcpDaq_PredefinedOdts[idx];
    }
#endif
}

XCP_STATIC void XcpDaq_StartStopLists(XcpDaq_ListTransitionType transition) {
    XcpDaq_ListIntegerType idx   = 0;
    XcpDaq_ListStateType  *entry = XCP_NULL;

    for (idx = (XcpDaq_ListIntegerType)0; idx < XcpDaq_GetListCount(); ++idx) {
        entry = XcpDaq_GetListState(idx);
        if ((entry->mode & XCP_DAQ_LIST_MODE_SELECTED) == XCP_DAQ_LIST_MODE_SELECTED) {
            if (transition == DAQ_LIST_TRANSITION_START) {
                entry->mode |= XCP_DAQ_LIST_MODE_STARTED;
                /* printf("Started DAQ list #%u\n", idx); */
            } else if (transition == DAQ_LIST_TRANSITION_STOP) {
                entry->mode &= UINT8(~XCP_DAQ_LIST_MODE_STARTED);
                /* printf("Stopped DAQ list #%u\n", idx); */
            } else {
                /* Do nothing (to keep MISRA happy). */
            }
            /*
             *  The slave has to reset the SELECTED flag in the mode at
             * GET_DAQ_LIST_MODE as soon as the related START_STOP_SYNCH or
             * SET_REQUEST have been acknowledged.
             */
            entry->mode &= UINT8(~XCP_DAQ_LIST_MODE_SELECTED);
        }
    }
}

bool XcpDaq_GetFirstPid(XcpDaq_ListIntegerType daqListNumber, XcpDaq_ODTIntegerType *firstPID) {
    XcpDaq_ListIntegerType              listIdx = 0;
    XcpDaq_ListConfigurationType const *daqList = XCP_NULL;
    bool                                result  = (bool)XCP_TRUE;
    XcpDaq_ODTIntegerType               tmp     = (XcpDaq_ODTIntegerType)0;

    if (daqListNumber > (XcpDaq_GetListCount() - UINT16(1))) {
        result = (bool)XCP_FALSE;
    } else {
        for (listIdx = UINT16(0); listIdx < daqListNumber; ++listIdx) {
            daqList = XcpDaq_GetListConfiguration(listIdx);
            tmp += daqList->numOdts;
        }
    }
    *firstPID = tmp;
    return result;
}

/*
**  Debugging / Testing interface.
*/
#if XCP_BUILD_TYPE == XCP_DEBUG_BUILD

void XcpDaq_GetCounts(XCP_DAQ_ENTITY_TYPE *entityCount, XCP_DAQ_ENTITY_TYPE *listCount, XCP_DAQ_ENTITY_TYPE *odtCount) {
    #if (XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON)
    *entityCount = XcpDaq_EntityCount;
    *listCount   = XcpDaq_ListCount;
    *odtCount    = XcpDaq_OdtCount;
    #endif
}

uint16_t XcpDaq_TotalDynamicEntityCount(void) {
    return UINT16(XCP_DAQ_MAX_DYNAMIC_ENTITIES);
}

XcpDaq_EntityType *XcpDaq_GetDynamicEntities(void) {
    return &XcpDaq_Entities[0];
}

XcpDaq_EntityType *XcpDaq_GetDynamicEntity(uint16_t num) {
    return &XcpDaq_Entities[num];
}

void XcpDaq_QueueGetVar(XcpDaq_QueueType *var) {
    XcpUtl_MemCopy(var, &XcpDaq_Queue, sizeof(XcpDaq_QueueType));
}

#endif /* XCP_BUILD_TYPE */

// #if XCP_DAQ_ENABLE_DYNAMIC_LISTS == XCP_ON

#if XCP_DAQ_ENABLE_QUEUING == XCP_ON
void XcpDaq_QueueInit(void) {
    uint8_t idx;

    XcpDaq_Queue.head = XcpDaq_Queue.tail = UINT8(0);
    XcpDaq_Queue.overload                 = (bool)XCP_FALSE;
    for (idx = UINT8(0); idx < UINT8(XCP_DAQ_QUEUE_SIZE); ++idx) {
        // XcpUtl_ZeroMem(&XcpDaq_QueueDTOs[idx], sizeof(Xcp_PduType));
        XcpUtl_ZeroMem(&XcpDaq_QueueDTOs[idx], sizeof(XcpDaq_QueueDTOs[idx]));
    }
}

XCP_STATIC bool XcpDaq_QueueFull(void) {
    return ((XcpDaq_Queue.head + UINT8(1)) % UINT8(XCP_DAQ_QUEUE_SIZE)) == XcpDaq_Queue.tail;
}

bool XcpDaq_QueueEmpty(void) {
    return XcpDaq_Queue.head == XcpDaq_Queue.tail;
}

bool XcpDaq_QueueEnqueue(uint16_t len, uint8_t const *data) {
    if (XcpDaq_QueueFull()) {
        XcpDaq_Queue.overload = (bool)XCP_TRUE;
        return (bool)XCP_FALSE;
    }

    XcpDaq_QueueDTOs[XcpDaq_Queue.head].len = len;

    // XCP_ASSERT_LE(len, XCP_MAX_DTO);
    if (len > XCP_MAX_DTO) {
        return (bool)XCP_FALSE;
    }

    XcpUtl_MemCopy(XcpDaq_QueueDTOs[XcpDaq_Queue.head].data, data, len);
    XcpDaq_Queue.head = (XcpDaq_Queue.head + UINT8(1)) % UINT8(XCP_DAQ_QUEUE_SIZE);
    return (bool)XCP_TRUE;
}

bool XcpDaq_QueueDequeue(uint16_t *len, uint8_t *data) {
    uint16_t dto_len;

    if (XcpDaq_QueueEmpty()) {
        return (bool)XCP_FALSE;
    }
    dto_len = XcpDaq_QueueDTOs[XcpDaq_Queue.tail].len;
    // XCP_ASSERT_LE(dto_len, XCP_MAX_DTO);
    if (dto_len > XCP_MAX_DTO) {
        return (bool)XCP_FALSE;
    }
    *len = dto_len;
    XcpUtl_MemCopy(data, XcpDaq_QueueDTOs[XcpDaq_Queue.tail].data, dto_len);
    XcpDaq_Queue.tail = (XcpDaq_Queue.tail + UINT8(1)) % UINT8(XCP_DAQ_QUEUE_SIZE);
    return (bool)XCP_TRUE;
}

// #endif /* XCP_DAQ_ENABLE_DYNAMIC_LISTS */
#endif /* XCP_DAQ_ENABLE_QUEUING */

////////////////////////////////////////////////////////////////////////////////
//                               xcp_timecorr.c                               //
////////////////////////////////////////////////////////////////////////////////

/*
 * BlueParrot XCP
 *
 * (C) 2026 by Christoph Schueler <github.com/Christoph2,
 *                                  cpu12.gems@googlemail.com>
 *
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * s. FLOSS-EXCEPTION.txt
 */


#if defined(XCP_ENABLE_TIME_CORRELATION) && (XCP_ENABLE_TIME_CORRELATION == XCP_ON)

/*
** ============================================================
**  Local types
** ============================================================
*/

typedef struct tagXcpTimecorr_StateType {
    XcpTimecorr_ResponseFmtType       response_fmt;
    uint16_t                          cluster_affiliation;
    XcpTimecorr_TimeSourceType const *grandmaster_source; /* NULL if no grandmaster registered */
} XcpTimecorr_StateType;

    /*
    ** ============================================================
    **  Clock info data block (uploaded by master via UPLOAD cmd)
    ** ============================================================
    **
    ** Layout (all fields Intel byte order):
    **   [0 .. 23]  XCP slave clock info   (always present when SLV_CLK_INFO bit set)
    **   [24 .. 47] Grandmaster clock info (present when GRANDM_CLK_INFO bit set)
    **   [48 .. 63] Clock relation         (present when CLK_RELATION bit set, grandmaster required)
    **   [64 .. 87] ECU clock info         (present when ECU_CLK_INFO bit set)
    **   [88 .. 95] ECU grandmaster UUID   (present when ECU_GRANDM_CLK_INFO bit set)
    */

    /* Individual block sizes (per XCP spec) */
    #define CLOCK_INFO_BLOCK_SIZE      (24u) /* UUID(8) + ticks(2) + unit(1) + stratum(1) + native(1) + epoch(1) + rsv(2) + maxts(8) */
    #define CLOCK_RELATION_BLOCK_SIZE  (16u) /* origin DLONG(8) + slv_ts DLONG(8) */
    #define ECU_GRANDM_UUID_BLOCK_SIZE (8u)  /* UUID(8) only */

    #define MAX_CLOCK_INFO_BLOCK                                                                                                   \
        (CLOCK_INFO_BLOCK_SIZE + CLOCK_INFO_BLOCK_SIZE + CLOCK_RELATION_BLOCK_SIZE + CLOCK_INFO_BLOCK_SIZE +                       \
         ECU_GRANDM_UUID_BLOCK_SIZE) /* 96 bytes */

static uint8_t XcpTimecorr_ClockInfoBlock[MAX_CLOCK_INFO_BLOCK];
static uint8_t XcpTimecorr_ClockInfoBlockLen = UINT8(0);

/*
** ============================================================
**  Module state
** ============================================================
*/

static XcpTimecorr_StateType XcpTimecorr_State = {
    .response_fmt        = XCP_TIMECORR_FMT_LEGACY,
    .cluster_affiliation = XCP_TIMECORR_DEFAULT_CLUSTER_ID,
    .grandmaster_source  = XCP_NULL,
};

/* Built-in XCP slave clock info (uses XcpHw_GetTimerCounter) */
static const XcpTimecorr_ClockInfoType XcpTimecorr_SlaveClockInfo = {
    .uuid          = XCP_TIMECORR_SLV_UUID,
    .ticks         = UINT16(1),
    .unit          = XCP_DAQ_TIMESTAMP_UNIT,
    .stratum       = XCP_TIMECORR_SLV_STRATUM,
    .native_size   = XCP_TIMECORR_SLV_NATIVE_TS_SIZE,
    .epoch         = XCP_TIMECORR_EPOCH_ARBITRARY,
    .max_timestamp = (XCP_TIMECORR_SLV_NATIVE_TS_SIZE == 4u) ? UINT64(0xFFFFFFFFu) : UINT64(0xFFFFFFFFFFFFFFFFu),
};

/*
** ============================================================
**  Forward declarations
** ============================================================
*/

static void XcpTimecorr_BuildClockInfoBlock(bool include_grandm, bool include_relation);
static void XcpTimecorr_SendTimeSyncInternal(
    uint8_t trigger_initiator, bool include_cluster_id, uint16_t cluster_id, uint8_t counter
);
static void    XcpTimecorr_WriteClockInfoStruct(uint8_t *dst, XcpTimecorr_ClockInfoType const *info);
static uint8_t XcpTimecorr_BuildObservableClocks(void);
static uint8_t XcpTimecorr_BuildSyncState(void);

/*
** ============================================================
**  Helpers: little-endian write utilities
** ============================================================
*/

static void WriteU16LE(uint8_t *dst, uint16_t v) {
    dst[0] = (uint8_t)(v & 0xFFu);
    dst[1] = (uint8_t)((v >> 8u) & 0xFFu);
}

static void WriteU32LE(uint8_t *dst, uint32_t v) {
    dst[0] = (uint8_t)(v & 0xFFu);
    dst[1] = (uint8_t)((v >> 8u) & 0xFFu);
    dst[2] = (uint8_t)((v >> 16u) & 0xFFu);
    dst[3] = (uint8_t)((v >> 24u) & 0xFFu);
}

static void WriteU64LE(uint8_t *dst, uint64_t v) {
    dst[0] = (uint8_t)(v & 0xFFu);
    dst[1] = (uint8_t)((v >> 8u) & 0xFFu);
    dst[2] = (uint8_t)((v >> 16u) & 0xFFu);
    dst[3] = (uint8_t)((v >> 24u) & 0xFFu);
    dst[4] = (uint8_t)((v >> 32u) & 0xFFu);
    dst[5] = (uint8_t)((v >> 40u) & 0xFFu);
    dst[6] = (uint8_t)((v >> 48u) & 0xFFu);
    dst[7] = (uint8_t)((v >> 56u) & 0xFFu);
}

/*
** ============================================================
**  Public API implementation
** ============================================================
*/

void XcpTimecorr_Init(void) {
    XcpTimecorr_State.response_fmt        = XCP_TIMECORR_FMT_LEGACY;
    XcpTimecorr_State.cluster_affiliation = XCP_TIMECORR_DEFAULT_CLUSTER_ID;
    XcpTimecorr_State.grandmaster_source  = XCP_NULL;
    XcpTimecorr_ClockInfoBlockLen         = UINT8(0);
}

void XcpTimecorr_SetGrandmasterClockSource(XcpTimecorr_TimeSourceType const *source) {
    XCP_ENTER_CRITICAL();
    XcpTimecorr_State.grandmaster_source = source;
    XCP_LEAVE_CRITICAL();
}

XcpTimecorr_ResponseFmtType XcpTimecorr_GetResponseFmt(void) {
    return XcpTimecorr_State.response_fmt;
}

uint16_t XcpTimecorr_GetClusterAffiliation(void) {
    return XcpTimecorr_State.cluster_affiliation;
}

/*
** ============================================================
**  TIME_CORRELATION_PROPERTIES command handler (0xC6)
** ============================================================
*/

void XcpTimecorr_HandleTimeCorrelationProperties(Xcp_PduType const * const pdu) {
    /*
     * Command layout (Intel byte order):
     *   [0] 0xC6  command code
     *   [1]       SET_PROPERTIES
     *   [2]       GET_PROPERTIES_REQUEST
     *   [3]       RESERVED
     *   [4..5]    CLUSTER_ID (WORD)
     */
    uint8_t  set_props  = Xcp_GetByte(pdu, UINT8(1));
    uint8_t  get_req    = Xcp_GetByte(pdu, UINT8(2));
    uint16_t cluster_id = Xcp_GetWord(pdu, UINT8(4));

    uint8_t response_fmt_req = (uint8_t)(set_props & UINT8(0x03));         /* bits [1:0] */
    uint8_t time_sync_bridge = (uint8_t)((set_props >> 2u) & UINT8(0x03)); /* bits [3:2] */
    uint8_t set_cluster_id   = (uint8_t)((set_props >> 4u) & UINT8(0x01)); /* bit  [4]  */
    bool    get_clk_info     = ((get_req & UINT8(0x01)) != UINT8(0));      /* bit  [0]  */

    DBG_TRACE(
        "TIME_CORRELATION_PROPERTIES [set_props: 0x%02X get_req: 0x%02X cluster_id: 0x%04X]\n\r", set_props, get_req, cluster_id
    );

    /* --- Process SET_PROPERTIES --- */

    /* RESPONSE_FMT: value 0 = do not change */
    if (response_fmt_req != UINT8(0)) {
        if (response_fmt_req <= UINT8(2)) {
            XcpTimecorr_State.response_fmt = (XcpTimecorr_ResponseFmtType)response_fmt_req;
        }
        /* value 3 is reserved; silently ignore */
    }

    /* TIME_SYNC_BRIDGE: silently ignored – we do not offer a bridge feature.
     * The spec requires silent ignore when the slave does not support this. */
    (void)time_sync_bridge;

    /* SET_CLUSTER_ID */
    if (set_cluster_id != UINT8(0)) {
        if (cluster_id != XcpTimecorr_State.cluster_affiliation) {
            XcpTimecorr_State.cluster_affiliation = cluster_id;
            /* Notify transport layer to rejoin the correct multicast group */
            XcpTl_UpdateMulticastGroup(cluster_id);
        }
    }

    /* --- Process GET_PROPERTIES_REQUEST --- */

    uint8_t clock_info_byte  = UINT8(0);
    bool    include_grandm   = false;
    bool    include_relation = false;

    if (get_clk_info) {
        /* Decide which clock info blocks to include */
        clock_info_byte |= UINT8(0x01); /* SLV_CLK_INFO always present */

    #if XCP_TIMECORR_GRANDM_CLK > 0
        clock_info_byte |= UINT8(0x02); /* GRANDM_CLK_INFO */
        include_grandm = true;
        if (XcpTimecorr_State.grandmaster_source != XCP_NULL) {
            clock_info_byte |= UINT8(0x04); /* CLK_RELATION */
            include_relation = true;
        }
    #endif

    #if XCP_TIMECORR_ECU_CLK > 0
        clock_info_byte |= UINT8(0x08); /* ECU_CLK_INFO */
    #endif

        XcpTimecorr_BuildClockInfoBlock(include_grandm, include_relation);

        /* Set MTA to start of clock info data block */
        Xcp_MtaType mta;
        mta.ext     = UINT8(0);
        mta.address = (Xcp_PointerSizeType)(uintptr_t)XcpTimecorr_ClockInfoBlock;
        Xcp_SetMta(mta);
    }

    /* --- Build positive response --- */

    /*
     * Response layout:
     *   [0] 0xFF  positive response
     *   [1]       SLAVE_CONFIG
     *   [2]       OBSERVABLE_CLOCKS
     *   [3]       SYNC_STATE
     *   [4]       CLOCK_INFO
     *   [5]       RESERVED
     *   [6..7]    CLUSTER_ID (WORD, Intel byte order)
     */

    /* SLAVE_CONFIG byte:
     *   bits [1:0] RESPONSE_FMT
     *   bit  [2]   DAQ_TS_RELATION (0 = XCP slave clock, 1 = ECU clock)
     *   bits [4:3] TIME_SYNC_BRIDGE (0 = not offered)
     */
    uint8_t slave_config = (uint8_t)XcpTimecorr_State.response_fmt; /* RESPONSE_FMT */
    #if XCP_TIMECORR_ECU_CLK > 0
    slave_config |= UINT8(0x04); /* DAQ_TS_RELATION = 1 (ECU clock) */
    #endif
    /* TIME_SYNC_BRIDGE = 0 (not offered) */

    uint8_t observable_clocks = XcpTimecorr_BuildObservableClocks();
    uint8_t sync_state        = XcpTimecorr_BuildSyncState();

    Xcp_Send8(
        UINT8(8), UINT8(XCP_PACKET_IDENTIFIER_RES), slave_config, observable_clocks, sync_state, clock_info_byte,
        UINT8(0), /* RESERVED */
        XCP_LOBYTE(XcpTimecorr_State.cluster_affiliation), XCP_HIBYTE(XcpTimecorr_State.cluster_affiliation)
    );
}

/*
** ============================================================
**  GET_DAQ_CLOCK_MULTICAST handler
** ============================================================
*/

void XcpTimecorr_HandleMulticast(uint16_t cluster_id, uint8_t counter) {
    /* Only respond if cluster_id matches affiliation (or default affiliation == 0 accepts all) */
    if ((XcpTimecorr_State.cluster_affiliation != UINT16(0)) && (cluster_id != XcpTimecorr_State.cluster_affiliation)) {
        return;
    }

    /* Only send EV_TIME_SYNC if XCP is connected (master must receive the unicast response) */
    if (Xcp_GetConnectionState() != XCP_CONNECTED) {
        return;
    }

    /* In legacy mode, do not respond to GET_DAQ_CLOCK_MULTICAST */
    if (XcpTimecorr_State.response_fmt == XCP_TIMECORR_FMT_LEGACY) {
        return;
    }

    XcpTimecorr_SendTimeSyncInternal(
        XCP_TIMECORR_TRIG_MULTICAST, true, /* include cluster id + counter */
        cluster_id, counter
    );
}

void XcpTimecorr_SendTimeSyncEvent(uint8_t trigger_initiator) {
    if (Xcp_GetConnectionState() != XCP_CONNECTED) {
        return;
    }

    /* Internal triggers (TRIGGER_INITIATOR != 2 or 3) are only sent when RESPONSE_FMT == 2 */
    if ((trigger_initiator != XCP_TIMECORR_TRIG_MULTICAST) && (trigger_initiator != XCP_TIMECORR_TRIG_BRIDGE)) {
        if (XcpTimecorr_State.response_fmt != XCP_TIMECORR_FMT_EXT_2) {
            return;
        }
    }

    if (XcpTimecorr_State.response_fmt == XCP_TIMECORR_FMT_LEGACY) {
        return;
    }

    XcpTimecorr_SendTimeSyncInternal(trigger_initiator, false, UINT16(0), UINT8(0));
}

/*
** ============================================================
**  Internal helpers
** ============================================================
*/

static uint8_t XcpTimecorr_BuildObservableClocks(void) {
    uint8_t obs = UINT8(0);

    /* XCP_SLV_CLK: bits [1:0] */
    obs |= (uint8_t)(((uint8_t)XCP_TIMECORR_XCP_SLV_CLK) & UINT8(0x03));

    /* GRANDM_CLK: bits [3:2] – reflect actual capability */
    #if XCP_TIMECORR_GRANDM_CLK > 0
    {
        uint8_t grandm_val = UINT8(XCP_TIMECORR_GRANDM_CLK);
        obs |= (uint8_t)((grandm_val & UINT8(0x03)) << 2u);
    }
    #endif

    /* ECU_CLK: bits [5:4] */
    #if XCP_TIMECORR_ECU_CLK > 0
    {
        uint8_t ecu_val = UINT8(XCP_TIMECORR_ECU_CLK);
        obs |= (uint8_t)((ecu_val & UINT8(0x03)) << 4u);
    }
    #endif

    return obs;
}

static uint8_t XcpTimecorr_BuildSyncState(void) {
    uint8_t sync = UINT8(0);

    /*
     * SLV_CLK_SYNC_STATE (bits [2:0]):
     * Only relevant if XCP_SLV_CLK == 1.
     * For XCP_SLV_CLK == 0 (free running), report "no sync support" (7).
     */
    #if XCP_TIMECORR_XCP_SLV_CLK == 1
    /* XCP slave clock can be syntonized/synchronized; report actual state */
    sync |= XCP_TIMECORR_SLV_NO_SYNC_SUPPORT; /* TODO: override when sync is implemented */
    #else
    sync |= XCP_TIMECORR_SLV_NO_SYNC_SUPPORT;
    #endif

    /* GRANDM_CLK_SYNC_STATE (bit [3]): only relevant if GRANDM_CLK > 0 */
    #if XCP_TIMECORR_GRANDM_CLK > 0
    {
        uint8_t grandm_state = XCP_TIMECORR_GRANDM_NOT_SYNCED;
        if (XcpTimecorr_State.grandmaster_source != XCP_NULL && XcpTimecorr_State.grandmaster_source->get_sync_state != XCP_NULL) {
            grandm_state = XcpTimecorr_State.grandmaster_source->get_sync_state();
        }
        sync |= (uint8_t)((grandm_state & UINT8(0x01)) << 3u);
    }
    #endif

    /* ECU_CLK_SYNC_STATE (bits [5:4]): only relevant if ECU_CLK > 0 */
    #if XCP_TIMECORR_ECU_CLK > 0
    sync |= (uint8_t)(XCP_TIMECORR_ECU_SYNC_UNKNOWN << 4u);
    #endif

    return sync;
}

/*
 * Serialize a single 24-byte clock info block into dst.
 */
static void XcpTimecorr_WriteClockInfoStruct(uint8_t *dst, XcpTimecorr_ClockInfoType const *info) {
    XcpUtl_MemCopy(dst, info->uuid, UINT32(8)); /* UUID (8 bytes) */
    WriteU16LE(dst + 8u, info->ticks);          /* Timestamp Ticks */
    dst[10u] = info->unit;                      /* Timestamp Unit */
    dst[11u] = info->stratum;                   /* Stratum Level */
    dst[12u] = info->native_size;               /* Native timestamp size */
    dst[13u] = info->epoch;                     /* Epoch */
    dst[14u] = UINT8(0);                        /* Reserved (WORD) */
    dst[15u] = UINT8(0);
    WriteU64LE(dst + 16u, info->max_timestamp); /* MAX_TIMESTAMP_VALUE_BEFORE_WRAP_AROUND */
}

/*
 * Build the clock info data block that the master uploads via UPLOAD command.
 * The MTA is set to XcpTimecorr_ClockInfoBlock after this call.
 */
static void XcpTimecorr_BuildClockInfoBlock(bool include_grandm, bool include_relation) {
    uint8_t *p   = XcpTimecorr_ClockInfoBlock;
    uint8_t  len = UINT8(0);

    /* XCP slave clock info (always included) */
    XcpTimecorr_WriteClockInfoStruct(p, &XcpTimecorr_SlaveClockInfo);
    p += CLOCK_INFO_BLOCK_SIZE;
    len += CLOCK_INFO_BLOCK_SIZE;

    /* Grandmaster clock info */
    if (include_grandm) {
        if (XcpTimecorr_State.grandmaster_source != XCP_NULL) {
            XcpTimecorr_WriteClockInfoStruct(p, &XcpTimecorr_State.grandmaster_source->info);
        } else {
            /* No grandmaster connected: UUID = 0 */
            XcpUtl_ZeroMem(p, CLOCK_INFO_BLOCK_SIZE);
        }
        p += CLOCK_INFO_BLOCK_SIZE;
        len += CLOCK_INFO_BLOCK_SIZE;

        /* Clock relation (origin + slave timestamp pair, sampled simultaneously) */
        if (include_relation && XcpTimecorr_State.grandmaster_source != XCP_NULL) {
            uint64_t grandm_ts = UINT64(0);
            uint64_t slv_ts    = UINT64(0);

            /* Sample both clocks as simultaneously as possible */
            XCP_ENTER_CRITICAL();
            if (XcpTimecorr_State.grandmaster_source->info.native_size == UINT8(4)) {
                grandm_ts = (uint64_t)XcpTimecorr_State.grandmaster_source->get_timestamp_u32();
            } else if (XcpTimecorr_State.grandmaster_source->get_timestamp_u64 != XCP_NULL) {
                grandm_ts = XcpTimecorr_State.grandmaster_source->get_timestamp_u64();
            }
            slv_ts = (uint64_t)XcpHw_GetTimerCounter();
            XCP_LEAVE_CRITICAL();

            WriteU64LE(p, grandm_ts);   /* Origin (grandmaster timestamp) */
            WriteU64LE(p + 8u, slv_ts); /* XCP slave timestamp */
        } else {
            XcpUtl_ZeroMem(p, CLOCK_RELATION_BLOCK_SIZE);
        }
        p += CLOCK_RELATION_BLOCK_SIZE;
        len += CLOCK_RELATION_BLOCK_SIZE;
    }

    XcpTimecorr_ClockInfoBlockLen = len;
}

/*
 * Build and send the extended EV_TIME_SYNC event packet.
 *
 * Payload layout (after the 2-byte event header FD 08):
 *   [0]         TRIGGER_INFO
 *   [1]         PAYLOAD_FMT
 *   [2..5/9]    XCP slave clock timestamp (DWORD or DLONG per FMT_XCP_SLV)
 *   [+4/8]      Grandmaster timestamp     (DWORD or DLONG per FMT_GRANDM, if present)
 *   [+2]        Cluster Identifier (WORD)  (if CLUSTER_IDENTIFIER bit set)
 *   [+1]        Counter                    (if CLUSTER_IDENTIFIER bit set)
 *   [+1]        SYNC_STATE                 (if any clock can be sync'd)
 */
static void XcpTimecorr_SendTimeSyncInternal(
    uint8_t trigger_initiator, bool include_cluster_id, uint16_t cluster_id, uint8_t counter
) {
    uint8_t payload[32]; /* sufficient for all scenarios */
    uint8_t pos             = UINT8(0);
    uint8_t fmt_xcp_slv     = UINT8(0);
    uint8_t fmt_grandm      = UINT8(0);
    bool    send_sync_state = false;

    /* Determine timestamp format for XCP slave clock */
    #if XCP_TIMECORR_XCP_SLV_CLK != 2
    fmt_xcp_slv = (XcpTimecorr_SlaveClockInfo.native_size == UINT8(4)) ? UINT8(1) : UINT8(2);
    #endif

    /* Determine timestamp format for grandmaster clock */
    #if XCP_TIMECORR_GRANDM_CLK > 0
    if (XcpTimecorr_State.grandmaster_source != XCP_NULL) {
        fmt_grandm = (XcpTimecorr_State.grandmaster_source->info.native_size == UINT8(4)) ? UINT8(1) : UINT8(2);
    }
    #endif

    /* SYNC_STATE is sent if any observable clock supports synchronization */
    #if (XCP_TIMECORR_XCP_SLV_CLK == 1) || (XCP_TIMECORR_GRANDM_CLK > 0)
    send_sync_state = true;
    #endif

    /* TRIGGER_INFO:  bits[2:0] = trigger_initiator, bits[4:3] = TIME_OF_TS_SAMPLING
     * For multicast, use RX sampling (3) per spec recommendation. */
    uint8_t ts_sampling  = (trigger_initiator == XCP_TIMECORR_TRIG_MULTICAST || trigger_initiator == XCP_TIMECORR_TRIG_BRIDGE) ?
                               XCP_TIMECORR_TS_SAMP_RX :
                               XCP_TIMECORR_TS_SAMP_CMD;
    uint8_t trigger_info = (uint8_t)((trigger_initiator & UINT8(0x07)) | (uint8_t)((ts_sampling & UINT8(0x03)) << 3u));

    /* PAYLOAD_FMT:
     *   bits[1:0] FMT_XCP_SLV
     *   bits[3:2] FMT_GRANDM
     *   bits[5:4] FMT_ECU (always 0 here, ECU support via separate extension)
     *   bit[6]    CLUSTER_IDENTIFIER
     */
    uint8_t payload_fmt = (uint8_t)(fmt_xcp_slv | (uint8_t)(fmt_grandm << 2u) | (include_cluster_id ? UINT8(0x40) : UINT8(0)));

    payload[pos++] = trigger_info;
    payload[pos++] = payload_fmt;

    /* Sample all timestamps as close together as possible */
    uint32_t slv_ts_32    = UINT32(0);
    uint64_t slv_ts_64    = UINT64(0);
    uint32_t grandm_ts_32 = UINT32(0);
    uint64_t grandm_ts_64 = UINT64(0);

    XCP_ENTER_CRITICAL();

    if (fmt_xcp_slv == UINT8(1)) {
        slv_ts_32 = XcpHw_GetTimerCounter();
    } else if (fmt_xcp_slv == UINT8(2)) {
        slv_ts_64 = (uint64_t)XcpHw_GetTimerCounter();
    }

    #if XCP_TIMECORR_GRANDM_CLK > 0
    if (fmt_grandm != UINT8(0) && XcpTimecorr_State.grandmaster_source != XCP_NULL) {
        if (fmt_grandm == UINT8(1)) {
            grandm_ts_32 = XcpTimecorr_State.grandmaster_source->get_timestamp_u32();
        } else {
            grandm_ts_64 = (XcpTimecorr_State.grandmaster_source->get_timestamp_u64 != XCP_NULL) ?
                               XcpTimecorr_State.grandmaster_source->get_timestamp_u64() :
                               (uint64_t)XcpTimecorr_State.grandmaster_source->get_timestamp_u32();
        }
    }
    #endif

    XCP_LEAVE_CRITICAL();

    /* Append XCP slave timestamp */
    if (fmt_xcp_slv == UINT8(1)) {
        WriteU32LE(payload + pos, slv_ts_32);
        pos += UINT8(4);
    } else if (fmt_xcp_slv == UINT8(2)) {
        WriteU64LE(payload + pos, slv_ts_64);
        pos += UINT8(8);
    }

    /* Append grandmaster timestamp */
    if (fmt_grandm == UINT8(1)) {
        WriteU32LE(payload + pos, grandm_ts_32);
        pos += UINT8(4);
    } else if (fmt_grandm == UINT8(2)) {
        WriteU64LE(payload + pos, grandm_ts_64);
        pos += UINT8(8);
    }

    /* Cluster Identifier + Counter (only for TRIGGER_INITIATOR 2 or 3) */
    if (include_cluster_id) {
        payload[pos++] = XCP_LOBYTE(cluster_id);
        payload[pos++] = XCP_HIBYTE(cluster_id);
        payload[pos++] = counter;
    }

    /* SYNC_STATE (only if at least one clock supports synchronization) */
    if (send_sync_state) {
        payload[pos++] = XcpTimecorr_BuildSyncState();
    }

    Xcp_SendEventPacket(EV_TIME_SYNC, payload, pos);
}

#endif /* XCP_ENABLE_TIME_CORRELATION */

////////////////////////////////////////////////////////////////////////////////
//                                 xcp_util.c                                 //
////////////////////////////////////////////////////////////////////////////////

/*
 * BlueParrot XCP
 *
 * (C) 2007-2025 by Christoph Schueler <github.com/Christoph2,
 *                                      cpu12.gems@googlemail.com>
 *
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * s. FLOSS-EXCEPTION.txt
 */


void XcpUtl_MemCopy(/*@out@*/ void *dst, /*@in@*/ void const *src, uint32_t len) {
    uint8_t       *pd = (uint8_t *)dst;
    uint8_t const *ps = (uint8_t const *)src;

    XCP_ASSERT(dst != XCP_NULL);
    XCP_ASSERT(src != XCP_NULL);

    if (dst == XCP_NULL || src == XCP_NULL || len == 0UL) {
        return;
    }

    for (uint32_t i = 0U; i < len; ++i) {
        pd[i] = ps[i];
    }
}

void XcpUtl_MemSet(/*@out@*/ void *dest, uint8_t fill_char, uint32_t len) {
    uint8_t *p = (uint8_t *)dest;

    XCP_ASSERT(XCP_NULL != dest);
    if (dest == XCP_NULL || len == 0UL) {
        return;
    }

    for (uint32_t i = 0U; i < len; ++i) {
        p[i] = fill_char;
    }
}

bool XcpUtl_MemCmp(/*@in@*/ void const *lhs, /*@in@*/ void const *rhs, uint32_t len) {
    uint8_t const *pl = (uint8_t const *)lhs;
    uint8_t const *pr = (uint8_t const *)rhs;

    XCP_ASSERT(XCP_NULL != lhs);
    XCP_ASSERT(XCP_NULL != rhs);

    if (lhs == XCP_NULL || rhs == XCP_NULL || len == UINT32(0)) {
        return XCP_FALSE;
    }

    while (len && (*pl == *pr)) {
        ++pl;
        ++pr;
        --len;
    }
    return (bool)(len == UINT32(0));
}

uint32_t XcpUtl_StrLen(/*@in@*/ char const *str) {
    uint32_t len = 0U;

    XCP_ASSERT(XCP_NULL != str);
    if (str == XCP_NULL) {
        return 0UL;
    }

    while (str[len] != '\0') {
        ++len;
    }

    return len;
}

uint8_t XcpUtl_SetResetBit8(uint8_t result, uint8_t value, uint8_t flag) {
    if ((value & flag) == flag) {
        result |= flag;
    } else {
        result &= ~flag;
    }
    return result;
}

#if XCP_BUILD_TYPE == XCP_DEBUG_BUILD
void XcpUtl_Hexdump(/*@in@*/ uint8_t const *buf, uint16_t sz) {
    uint16_t idx;

    for (idx = UINT16(0); idx < sz; ++idx) {
        DBG_PRINT("%02X ", buf[idx]);
    }
    DBG_PRINT("\n\r");
}

void XcpUtl_Itoa(uint32_t value, uint8_t base, uint8_t *buf) {
    uint8_t mod;
    uint8_t pos = (uint8_t)0x00, swap_pos = (uint8_t)0x00;
    uint8_t ch;

    /* ASSERT(buf != (void *)NULL); */
    if (((int32_t)value) < 0L && base == (uint8_t)10) {
        value    = (uint32_t)((int32_t)value * -1L);
        buf[0]   = '-';
        swap_pos = 1;
        pos      = 1;
    }

    if (value == 0L) {
        buf[pos++] = '0';
    }

    while (value) {
        mod = value % base;
        if (mod < 10) {
            buf[pos++] = '0' + mod;
        } else {
            buf[pos++] = 'A' + mod - (uint8_t)10;
        }
        value /= base;
    }
    buf[pos--] = '\0';
    while (pos > swap_pos) {
        ch            = buf[swap_pos];
        buf[swap_pos] = buf[pos];
        buf[pos]      = ch;
        swap_pos++;
        pos--;
    }
}
#endif

////////////////////////////////////////////////////////////////////////////////
//                              xcp_tl_timeout.c                              //
////////////////////////////////////////////////////////////////////////////////

/*
 * BlueParrot XCP
 *
 * (C) 2007-2022 by Christoph Schueler <github.com/Christoph2,
 *                                      cpu12.gems@googlemail.com>
 *
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * s. FLOSS-EXCEPTION.txt
 */


/*
 *
 * Time-out handling functions for Transport-Layer.
 *
 */

typedef void (*void_function)(void);

static uint32_t      XcpTl_TimeoutValue    = 0UL;
static void_function XcpTl_TimeoutFunction = XCP_NULL;
static bool          XcpTl_TimeoutRunning  = XCP_FALSE;

void XcpTl_TimeoutInit(uint16_t timeout_value, void (*timeout_function)(void)) {
    XcpTl_TimeoutValue    = timeout_value;
    XcpTl_TimeoutRunning  = XCP_FALSE;
    XcpTl_TimeoutFunction = timeout_function;
}

void XcpTl_TimeoutStart(void) {
    XcpTl_TimeoutValue   = XcpHw_GetTimerCounterMS();
    XcpTl_TimeoutRunning = XCP_TRUE;
}

void XcpTl_TimeoutStop(void) {
    XcpTl_TimeoutRunning = XCP_FALSE;
}

void XcpTl_TimeoutCheck(void) {
    if (!XcpTl_TimeoutRunning) {
        return;
    }
    if ((XcpHw_GetTimerCounterMS() - XcpTl_TimeoutValue) > XcpTl_TimeoutValue) {
        if (XcpTl_TimeoutFunction != XCP_NULL) {
            XcpTl_TimeoutFunction();
        }
    }
}

void XcpTl_TimeoutReset(void) {
    XcpTl_TimeoutValue = XcpHw_GetTimerCounterMS();
}

////////////////////////////////////////////////////////////////////////////////
//                                  xcp_tl.c                                  //
////////////////////////////////////////////////////////////////////////////////

/*
 * BlueParrot XCP
 *
 * (C) 2007-2025 by Christoph Schueler <github.com/Christoph2,
 *                                      cpu12.gems@googlemail.com>
 *
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * s. FLOSS-EXCEPTION.txt
 */

// #include "xcp_config.h"

#include <stdio.h>


#if XCP_TRANSPORT_LAYER == XCP_ON_SXI

    #define XCP_SXI_MAKEWORD(buf, offs) ((uint16_t)(*(((buf)) + (offs))) | ((uint16_t)(*(((buf)) + (offs) + 1)) << 8))

    #define TIMEOUT_VALUE (100)

    #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_BYTE)
typedef uint8_t XcpSxiChecksumType;
    #elif (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_WORD)
typedef uint16_t XcpSxiChecksumType;
    #endif

typedef enum tagXcpTl_ReceiverStateType {
    XCP_RCV_IDLE,
    XCP_RCV_UNTIL_LENGTH,
    XCP_RCV_REMAINING
} XcpTl_ReceiverStateType;

typedef struct tagXcpTl_ReceiverType {
    uint8_t                 Buffer[XCP_COMM_BUFLEN];
    XcpTl_ReceiverStateType State;
    uint16_t                Index;
    uint16_t                Remaining;
    uint16_t                Dlc;
    uint16_t                Ctr;
    #if (XCP_ON_SXI_TAIL_CHECKSUM != XCP_ON_SXI_NO_CHECKSUM)
    XcpSxiChecksumType ReceivedChecksum;
    #endif
} XcpTl_ReceiverType;

    #if (XCP_ON_SXI_ENABLE_FRAMING == XCP_ON)
typedef enum {
    FRM_WAIT_FOR_SYNC,
    FRM_RECEIVING,
    FRM_WAIT_FOR_ESC_BYTE
} XcpTl_FramingStateType;

static XcpTl_FramingStateType s_FramingState = FRM_WAIT_FOR_SYNC;

    #endif

static XcpTl_ReceiverType XcpTl_Receiver;

static void XcpTl_ResetSM(void);

static void XcpTl_SignalTimeout(void);

static void XcpTl_ProcessOctet(uint8_t octet);

void XcpTl_Init(void) {
    Serial_Init();

    XcpTl_ResetSM();
    XcpTl_TimeoutInit(TIMEOUT_VALUE, XcpTl_ResetSM);
}

void XcpTl_DeInit(void) {
    Serial_DeInit();
}

void XcpTl_MainFunction(void) {
    uint32_t byte_count = 0UL;
    uint8_t  octet      = 0;

    Serial_MainFunction();
    byte_count = Serial_Available();
    if (byte_count > 0U) {
        while (byte_count--) {
            if (Serial_Read(&octet)) {
                XcpTl_FeedReceiver(octet);
            }
        }
    }
    XcpTl_TimeoutCheck();
}

/**
 * \brief Initialize, i.e. reset
 *receiver state Machine
 *
 * \param void
 * \return void
 *
 **/
static void XcpTl_ResetSM(void) {
    XcpTl_Receiver.State     = XCP_RCV_IDLE;
    XcpTl_Receiver.Index     = 0u;
    XcpTl_Receiver.Dlc       = 0u;
    XcpTl_Receiver.Ctr       = 0u;
    XcpTl_Receiver.Remaining = 0u;
    #if (XCP_ON_SXI_TAIL_CHECKSUM != XCP_ON_SXI_NO_CHECKSUM)
    XcpTl_Receiver.ReceivedChecksum = 0;
    #endif
    #if (XCP_ON_SXI_ENABLE_FRAMING == XCP_ON)
    s_FramingState = FRM_WAIT_FOR_SYNC;
    #endif
}

    #if defined(XCP_TL_TEST_HOOKS)
/* Test hook to invoke the internal state machine reset from unit-tests. */
void XcpTl_Test_ResetSM(void) {
    XcpTl_ResetSM();
}
    #endif

void XcpTl_RxHandler(void) {
}

void XcpTl_FeedReceiver(uint8_t octet) {
    #if (XCP_ON_SXI_ENABLE_FRAMING == XCP_ON)
    switch (s_FramingState) {
        case FRM_WAIT_FOR_SYNC:
            if (octet == XCP_ON_SXI_SYNC_CHAR) {
                s_FramingState = FRM_RECEIVING;
                XcpTl_ResetSM();
            }
            break;

        case FRM_RECEIVING:
            if (octet == XCP_ON_SXI_ESC_CHAR) {
                s_FramingState = FRM_WAIT_FOR_ESC_BYTE;
            } else if (octet == XCP_ON_SXI_SYNC_CHAR) {
                /* New frame started without proper termination of the old one. */
                XcpTl_ResetSM();
                s_FramingState = FRM_RECEIVING;
            } else {
                XcpTl_ProcessOctet(octet);
            }
            break;

        case FRM_WAIT_FOR_ESC_BYTE:
            if (octet == XCP_ON_SXI_ESC_SYNC_CHAR) {
                XcpTl_ProcessOctet(XCP_ON_SXI_SYNC_CHAR);
                s_FramingState = FRM_RECEIVING;
            } else if (octet == XCP_ON_SXI_ESC_ESC_CHAR) {
                XcpTl_ProcessOctet(XCP_ON_SXI_ESC_CHAR);
                s_FramingState = FRM_RECEIVING;
            } else {
                /* Protocol error, invalid escape sequence. Reset. */
                XcpTl_ResetSM();
                s_FramingState = FRM_WAIT_FOR_SYNC;
            }
            break;
    }
    #else
    XcpTl_ProcessOctet(octet);
    #endif
}

static void XcpTl_ProcessOctet(uint8_t octet) {
    #if (XCP_ON_SXI_TAIL_CHECKSUM != XCP_ON_SXI_NO_CHECKSUM)
    XcpSxiChecksumType calc_checksum = 0u;
    uint16_t           idx           = 0u;
    uint16_t           payload_off   = 0u;
    uint16_t           dlc           = 0u;
    uint16_t           fill          = 0u;
        #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_WORD)
    uint16_t w   = 0u;
    uint16_t w2  = 0u;
    uint16_t off = 0u;
        #endif
    #endif

    XCP_TL_ENTER_CRITICAL();

    /* Bounds check to protect Buffer[] */
    if (XcpTl_Receiver.Index >= (uint16_t)sizeof(XcpTl_Receiver.Buffer)) {
        /* Overflow/malformed frame: reset receiver */
        XcpTl_ResetSM();
        XcpTl_TimeoutStop();
        XCP_TL_LEAVE_CRITICAL();
        return;
    }
    XcpTl_Receiver.Buffer[XcpTl_Receiver.Index] = octet;

    if (XcpTl_Receiver.State == XCP_RCV_IDLE) {
        XcpTl_Receiver.State = XCP_RCV_UNTIL_LENGTH;
        XcpTl_TimeoutStart();
    #if (XCP_ON_SXI_HEADER_FORMAT == XCP_ON_SXI_HEADER_LEN_BYTE)
    } else if (XcpTl_Receiver.State == XCP_RCV_UNTIL_LENGTH) {
        if (XcpTl_Receiver.Index == 0x01) {
            XcpTl_Receiver.Dlc       = XcpTl_Receiver.Buffer[0];
            XcpTl_Receiver.State     = XCP_RCV_REMAINING;
            XcpTl_Receiver.Remaining = XcpTl_Receiver.Dlc - 1;
        #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_WORD)
            /* Even byte-count required. */
            if (((XcpTl_Receiver.Dlc + 1u) & 1u) != 0u) {
                XcpTl_Receiver.Remaining += 1u;
            }
        #endif
        #if (XCP_ON_SXI_TAIL_CHECKSUM != XCP_ON_SXI_NO_CHECKSUM)
            XcpTl_Receiver.Remaining += sizeof(XcpSxiChecksumType);
        #endif
            XcpTl_TimeoutReset();
        /* Handle minimal-length frames immediately (e.g., LEN=1, no checksum) */
        #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_NO_CHECKSUM)
            if (XcpTl_Receiver.Remaining == 0u) {
                Xcp_CtoIn.len  = XcpTl_Receiver.Dlc;
                Xcp_CtoIn.data = XcpTl_Receiver.Buffer + 1; /* payload follows 1-byte LEN */
                XcpTl_ResetSM();
                XcpTl_TimeoutStop();
                Xcp_DispatchCommand(&Xcp_CtoIn);
                XCP_TL_LEAVE_CRITICAL();
                return;
            }
        #endif
        }
    } else if (XcpTl_Receiver.State == XCP_RCV_REMAINING) {
        XcpTl_TimeoutReset();
        XcpTl_Receiver.Remaining--;
        if (XcpTl_Receiver.Remaining == 0u) {
        #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_NO_CHECKSUM)
            Xcp_CtoIn.len  = XcpTl_Receiver.Dlc;
            Xcp_CtoIn.data = XcpTl_Receiver.Buffer + 1;
            XcpTl_ResetSM();
            XcpTl_TimeoutStop();
            Xcp_DispatchCommand(&Xcp_CtoIn);
        #else
            payload_off = 1u;
            dlc         = XcpTl_Receiver.Dlc;
            fill        = 0u;
            #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_WORD)
            if (((dlc + 1u) & 1u) != 0u) {
                /* Even byte-count required. */
                fill = 1u;
            }
            #endif
            #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_BYTE)
            {
                calc_checksum = 0u;
                for (idx = 0u; idx < (1u + dlc); ++idx) {
                    calc_checksum += XcpTl_Receiver.Buffer[idx];
                }
                XcpTl_Receiver.ReceivedChecksum = XcpTl_Receiver.Buffer[payload_off + dlc];
                if (((uint8_t)calc_checksum) == XcpTl_Receiver.ReceivedChecksum) {
                    Xcp_CtoIn.len  = XcpTl_Receiver.Dlc;
                    Xcp_CtoIn.data = XcpTl_Receiver.Buffer + 1;
                    XcpTl_ResetSM();
                    XcpTl_TimeoutStop();
                    Xcp_DispatchCommand(&Xcp_CtoIn);
                } else {
                    /* Checksum error */
                    XcpTl_ResetSM();
                }
            }
            #else /* XCP_ON_SXI_CHECKSUM_WORD */
            {
                calc_checksum = 0u;
                /* Calculate WORD checksum over header and payload */
                for (idx = 0u; idx < ((payload_off + dlc + fill) / 2u); ++idx) {
                    calc_checksum += (uint16_t)((uint16_t)XcpTl_Receiver.Buffer[2u * idx] |
                                                ((uint16_t)XcpTl_Receiver.Buffer[2u * idx + 1u] << 8));
                }

                XcpTl_Receiver.ReceivedChecksum = (uint16_t)((uint16_t)XcpTl_Receiver.Buffer[payload_off + dlc + fill] |
                                                             ((uint16_t)XcpTl_Receiver.Buffer[payload_off + dlc + fill + 1u] << 8));

                if (calc_checksum == XcpTl_Receiver.ReceivedChecksum) {
                    Xcp_CtoIn.len  = XcpTl_Receiver.Dlc;
                    Xcp_CtoIn.data = XcpTl_Receiver.Buffer + 1;
                    XcpTl_ResetSM();
                    XcpTl_TimeoutStop();
                    Xcp_DispatchCommand(&Xcp_CtoIn);
                } else {
                    /* Checksum error */
                    XcpTl_ResetSM();
                }
            }
            #endif
        #endif
        }
    #elif (XCP_ON_SXI_HEADER_FORMAT == XCP_ON_SXI_HEADER_LEN_CTR_BYTE) ||                                                          \
        (XCP_ON_SXI_HEADER_FORMAT == XCP_ON_SXI_HEADER_LEN_FILL_BYTE)
    } else if (XcpTl_Receiver.State == XCP_RCV_UNTIL_LENGTH) {
        if (XcpTl_Receiver.Index == 0x01) {
            /* DLC and CTR/FILL received */
            XcpTl_Receiver.Dlc       = XcpTl_Receiver.Buffer[0];
            XcpTl_Receiver.State     = XCP_RCV_REMAINING;
            XcpTl_Receiver.Remaining = XcpTl_Receiver.Dlc;
        #if (XCP_ON_SXI_HEADER_FORMAT == XCP_ON_SXI_HEADER_LEN_CTR_BYTE)
            XcpTl_Receiver.Ctr = XcpTl_Receiver.Buffer[1];
        #endif
        #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_WORD)
            if ((XcpTl_Receiver.Dlc & 1u) != 0u) {
                XcpTl_Receiver.Remaining += 1u; /* fill byte before checksum */
            }
        #endif
        #if (XCP_ON_SXI_TAIL_CHECKSUM != XCP_ON_SXI_NO_CHECKSUM)
            XcpTl_Receiver.Remaining += sizeof(XcpSxiChecksumType);
        #endif
            XcpTl_TimeoutReset();
        /* Handle minimal-length frames immediately (e.g., LEN=0, no checksum) */
        #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_NO_CHECKSUM)
            if (XcpTl_Receiver.Remaining == 0u) {
                Xcp_CtoIn.len  = XcpTl_Receiver.Dlc;
                Xcp_CtoIn.data = XcpTl_Receiver.Buffer + 2; /* payload after LEN + CTR/FILL */
                XcpTl_ResetSM();
                XcpTl_TimeoutStop();
                Xcp_DispatchCommand(&Xcp_CtoIn);
                XCP_TL_LEAVE_CRITICAL();
                return;
            }
        #endif
        }
    } else if (XcpTl_Receiver.State == XCP_RCV_REMAINING) {
        XcpTl_TimeoutReset();
        XcpTl_Receiver.Remaining--;
        if (XcpTl_Receiver.Remaining == 0u) {

        #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_NO_CHECKSUM)
            Xcp_CtoIn.len  = XcpTl_Receiver.Dlc;
            Xcp_CtoIn.data = XcpTl_Receiver.Buffer + 2; /* Data starts after LEN and CTR/FILL */
            XcpTl_ResetSM();
            XcpTl_TimeoutStop();
            Xcp_DispatchCommand(&Xcp_CtoIn);
        #else
            payload_off = 2u; /* Data starts after LEN and CTR/FILL */
            dlc         = XcpTl_Receiver.Dlc;
            fill        = 0u;
            #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_WORD)
            if ((dlc & 1u) != 0u) {
                fill = 1u;
            }
            #endif
            #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_BYTE)
            {
                calc_checksum = 0u;
                for (idx = 0u; idx < (2u + dlc); ++idx) {
                    calc_checksum += XcpTl_Receiver.Buffer[idx];
                }
                XcpTl_Receiver.ReceivedChecksum = XcpTl_Receiver.Buffer[payload_off + dlc];
                if (((uint8_t)calc_checksum) == XcpTl_Receiver.ReceivedChecksum) {
                    Xcp_CtoIn.len  = XcpTl_Receiver.Dlc;
                    Xcp_CtoIn.data = XcpTl_Receiver.Buffer + 2;
                    XcpTl_ResetSM();
                    XcpTl_TimeoutStop();
                    Xcp_DispatchCommand(&Xcp_CtoIn);
                } else {
                    /* Checksum error */
                    XcpTl_ResetSM();
                }
            }
            #else /* XCP_ON_SXI_CHECKSUM_WORD */
            {
                calc_checksum = 0u;
                /* Calculate WORD checksum over header and payload */
                for (idx = 0u; idx < ((payload_off + dlc + fill) / 2u); ++idx) {
                    calc_checksum += (uint16_t)((uint16_t)XcpTl_Receiver.Buffer[2u * idx] |
                                                ((uint16_t)XcpTl_Receiver.Buffer[2u * idx + 1u] << 8));
                }

                XcpTl_Receiver.ReceivedChecksum = (uint16_t)((uint16_t)XcpTl_Receiver.Buffer[payload_off + dlc + fill] |
                                                             ((uint16_t)XcpTl_Receiver.Buffer[payload_off + dlc + fill + 1u] << 8));

                if (calc_checksum == XcpTl_Receiver.ReceivedChecksum) {
                    Xcp_CtoIn.len  = XcpTl_Receiver.Dlc;
                    Xcp_CtoIn.data = XcpTl_Receiver.Buffer + 2;
                    XcpTl_ResetSM();
                    XcpTl_TimeoutStop();
                    Xcp_DispatchCommand(&Xcp_CtoIn);
                } else {
                    /* Checksum error */
                    XcpTl_ResetSM();
                }
            }
            #endif
        #endif
        }
    #elif (XCP_ON_SXI_HEADER_FORMAT == XCP_ON_SXI_HEADER_LEN_WORD)
    } else if (XcpTl_Receiver.State == XCP_RCV_UNTIL_LENGTH) {
        if (XcpTl_Receiver.Index == 0x01) {
            XcpTl_Receiver.Dlc = XCP_SXI_MAKEWORD(XcpTl_Receiver.Buffer, 0x00);
            /* Validate DLC against buffer capacity (account for 2-byte LEN + 2-byte CTR) */
            if (XcpTl_Receiver.Dlc > ((uint16_t)sizeof(XcpTl_Receiver.Buffer) - 4u)) {
                /* Length too large: reset */
                XcpTl_ResetSM();
                XcpTl_TimeoutStop();
                XCP_TL_LEAVE_CRITICAL();
                return;
            }
            XcpTl_Receiver.State     = XCP_RCV_REMAINING;
            XcpTl_Receiver.Remaining = XcpTl_Receiver.Dlc;
        #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_WORD)
            if ((XcpTl_Receiver.Dlc & 1u) != 0u) {
                XcpTl_Receiver.Remaining += 1u; /* fill byte before checksum */
            }
        #endif
        #if (XCP_ON_SXI_TAIL_CHECKSUM != XCP_ON_SXI_NO_CHECKSUM)
            XcpTl_Receiver.Remaining += sizeof(XcpSxiChecksumType);
        #endif
            XcpTl_TimeoutReset();
        /* Handle minimal-length frames immediately (e.g., LEN=0, no checksum) */
        #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_NO_CHECKSUM)
            if (XcpTl_Receiver.Remaining == 0u) {
                Xcp_CtoIn.len  = XcpTl_Receiver.Dlc;
                Xcp_CtoIn.data = XcpTl_Receiver.Buffer + 2; /* payload after 2-byte LEN */
                XcpTl_ResetSM();
                XcpTl_TimeoutStop();
                Xcp_DispatchCommand(&Xcp_CtoIn);
                XCP_TL_LEAVE_CRITICAL();
                return;
            }
        #endif
        }
    } else if (XcpTl_Receiver.State == XCP_RCV_REMAINING) {
        XcpTl_TimeoutReset();
        XcpTl_Receiver.Remaining--;
        if (XcpTl_Receiver.Remaining == 0u) {

        #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_NO_CHECKSUM)
            Xcp_CtoIn.len  = XcpTl_Receiver.Dlc;
            Xcp_CtoIn.data = XcpTl_Receiver.Buffer + 2;
            XcpTl_ResetSM();
            XcpTl_TimeoutStop();
            Xcp_DispatchCommand(&Xcp_CtoIn);
        #else
            payload_off = 2u;
            dlc         = XcpTl_Receiver.Dlc;
            fill        = 0u;
            #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_WORD)
            if ((dlc & 1u) != 0u) {
                fill = 1u;
            }
            #endif
            #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_BYTE)
            {
                calc_checksum = 0u;
                for (idx = 0u; idx < (2u + dlc); ++idx) {
                    calc_checksum += XcpTl_Receiver.Buffer[idx];
                }
                XcpTl_Receiver.ReceivedChecksum = XcpTl_Receiver.Buffer[payload_off + dlc];
                if (((uint8_t)calc_checksum) == XcpTl_Receiver.ReceivedChecksum) {
                    Xcp_CtoIn.len  = XcpTl_Receiver.Dlc;
                    Xcp_CtoIn.data = XcpTl_Receiver.Buffer + 2;
                    XcpTl_ResetSM();
                    XcpTl_TimeoutStop();
                    Xcp_DispatchCommand(&Xcp_CtoIn);
                } else {
                    /* Checksum error */
                    XcpTl_ResetSM();
                }
            }
            #else /* XCP_ON_SXI_CHECKSUM_WORD */
            {
                calc_checksum = 0u;
                /* Calculate WORD checksum over header and payload */
                for (idx = 0u; idx < ((payload_off + dlc + fill) / 2u); ++idx) {
                    calc_checksum += (uint16_t)((uint16_t)XcpTl_Receiver.Buffer[2u * idx] |
                                                ((uint16_t)XcpTl_Receiver.Buffer[2u * idx + 1u] << 8));
                }

                XcpTl_Receiver.ReceivedChecksum = (uint16_t)((uint16_t)XcpTl_Receiver.Buffer[payload_off + dlc + fill] |
                                                             ((uint16_t)XcpTl_Receiver.Buffer[payload_off + dlc + fill + 1u] << 8));

                if (calc_checksum == XcpTl_Receiver.ReceivedChecksum) {
                    Xcp_CtoIn.len  = XcpTl_Receiver.Dlc;
                    Xcp_CtoIn.data = XcpTl_Receiver.Buffer + 2;
                    XcpTl_ResetSM();
                    XcpTl_TimeoutStop();
                    Xcp_DispatchCommand(&Xcp_CtoIn);
                } else {
                    /* Checksum error */
                    XcpTl_ResetSM();
                }
            }
            #endif
        #endif
        }
    #elif (XCP_ON_SXI_HEADER_FORMAT == XCP_ON_SXI_HEADER_LEN_CTR_WORD) ||                                                          \
        (XCP_ON_SXI_HEADER_FORMAT == XCP_ON_SXI_HEADER_LEN_FILL_WORD)
    } else if (XcpTl_Receiver.State == XCP_RCV_UNTIL_LENGTH) {
        if (XcpTl_Receiver.Index == 0x03) {
            /* Wait for 4 header bytes (LEN+CTR/FILL) */
            XcpTl_Receiver.Dlc = XCP_SXI_MAKEWORD(XcpTl_Receiver.Buffer, 0x00);
            /* Validate DLC against buffer capacity (account for 4-byte header) */
            if (XcpTl_Receiver.Dlc > ((uint16_t)sizeof(XcpTl_Receiver.Buffer) - 4u)) {
                /* Length too large: reset */
                XcpTl_ResetSM();
                XcpTl_TimeoutStop();
                XCP_TL_LEAVE_CRITICAL();
                return;
            }
            XcpTl_Receiver.State     = XCP_RCV_REMAINING;
            XcpTl_Receiver.Remaining = XcpTl_Receiver.Dlc;
        #if (XCP_ON_SXI_HEADER_FORMAT == XCP_ON_SXI_HEADER_LEN_CTR_WORD)
            XcpTl_Receiver.Ctr = XCP_SXI_MAKEWORD(XcpTl_Receiver.Buffer, 0x02);
        #endif
        #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_WORD)
            if ((XcpTl_Receiver.Dlc & 1u) != 0u) {
                XcpTl_Receiver.Remaining += 1u; /* fill byte before checksum */
            }
        #endif
        #if (XCP_ON_SXI_TAIL_CHECKSUM != XCP_ON_SXI_NO_CHECKSUM)
            XcpTl_Receiver.Remaining += sizeof(XcpSxiChecksumType);
        #endif
            XcpTl_TimeoutReset();
        }
    } else if (XcpTl_Receiver.State == XCP_RCV_REMAINING) {
        XcpTl_TimeoutReset();
        XcpTl_Receiver.Remaining--;
        if (XcpTl_Receiver.Remaining == 0u) {

        #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_NO_CHECKSUM)
            Xcp_CtoIn.len  = XcpTl_Receiver.Dlc;
            Xcp_CtoIn.data = XcpTl_Receiver.Buffer + 4; /* Data starts after LEN and CTR/FILL */
            XcpTl_ResetSM();
            XcpTl_TimeoutStop();
            Xcp_DispatchCommand(&Xcp_CtoIn);
        #else
            payload_off = 4u; /* Data starts after LEN and CTR/FILL */
            dlc         = XcpTl_Receiver.Dlc;
            fill        = 0u;
            #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_WORD)
            if ((dlc & 1u) != 0u) {
                fill = 1u;
            }
            #endif
            #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_BYTE)
            {
                calc_checksum = 0u;
                for (idx = 0u; idx < (4u + dlc); ++idx) {
                    calc_checksum += XcpTl_Receiver.Buffer[idx];
                }
                XcpTl_Receiver.ReceivedChecksum = XcpTl_Receiver.Buffer[payload_off + dlc];
                if (((uint8_t)calc_checksum) == XcpTl_Receiver.ReceivedChecksum) {
                    Xcp_CtoIn.len  = XcpTl_Receiver.Dlc;
                    Xcp_CtoIn.data = XcpTl_Receiver.Buffer + 4;
                    XcpTl_ResetSM();
                    XcpTl_TimeoutStop();
                    Xcp_DispatchCommand(&Xcp_CtoIn);
                } else {
                    /* Checksum error */
                    XcpTl_ResetSM();
                }
            }
            #else /* XCP_ON_SXI_CHECKSUM_WORD */
            {
                calc_checksum = 0u;
                for (idx = 0u; idx < ((payload_off + dlc + fill) / 2u); ++idx) {
                    calc_checksum += (uint16_t)((uint16_t)XcpTl_Receiver.Buffer[2u * idx] |
                                                ((uint16_t)XcpTl_Receiver.Buffer[2u * idx + 1u] << 8));
                }

                XcpTl_Receiver.ReceivedChecksum = (uint16_t)((uint16_t)XcpTl_Receiver.Buffer[payload_off + dlc + fill] |
                                                             ((uint16_t)XcpTl_Receiver.Buffer[payload_off + dlc + fill + 1u] << 8));

                if (calc_checksum == XcpTl_Receiver.ReceivedChecksum) {
                    Xcp_CtoIn.len  = XcpTl_Receiver.Dlc;
                    Xcp_CtoIn.data = XcpTl_Receiver.Buffer + 4;
                    XcpTl_ResetSM();
                    XcpTl_TimeoutStop();
                    Xcp_DispatchCommand(&Xcp_CtoIn);
                } else {
                    /* Checksum error */
                    XcpTl_ResetSM();
                }
            }
            #endif
        #endif
        }
    #else
        #error "Please define XCP_ON_SXI_HEADER_FORMAT to a valid value"
    #endif
    }
    if (XcpTl_Receiver.State != XCP_RCV_IDLE) {
        XcpTl_Receiver.Index++;
    }
    XCP_TL_LEAVE_CRITICAL();
}

    #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_BYTE)
typedef uint8_t XcpSxiChecksumType;
    #elif (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_WORD)
typedef uint16_t XcpSxiChecksumType;
    #endif

static void XcpTl_TransformAndSend(uint8_t const *buf, uint16_t len) {
    uint16_t idx;

    Serial_WriteByte((uint8_t)XCP_ON_SXI_SYNC_CHAR);

    for (idx = 0; idx < len; idx++) {
        if (buf[idx] == XCP_ON_SXI_SYNC_CHAR) {
            Serial_WriteByte((uint8_t)XCP_ON_SXI_ESC_CHAR);
            Serial_WriteByte((uint8_t)XCP_ON_SXI_ESC_SYNC_CHAR);
        } else if (buf[idx] == XCP_ON_SXI_ESC_CHAR) {
            Serial_WriteByte((uint8_t)XCP_ON_SXI_ESC_CHAR);
            Serial_WriteByte((uint8_t)XCP_ON_SXI_ESC_ESC_CHAR);
        } else {
            Serial_WriteByte(buf[idx]);
        }
    }
}

void XcpTl_Send(uint8_t const *buf, uint16_t len) {
    XCP_TL_ENTER_CRITICAL();

    #if (XCP_ON_SXI_TAIL_CHECKSUM != XCP_ON_SXI_NO_CHECKSUM)
    {
        XcpSxiChecksumType checksum = 0;
        uint16_t           idx;

        #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_WORD)
        /* Calculate WORD checksum over header and payload */
        for (idx = 0; idx < (len / 2u); idx++) {
            checksum += (uint16_t)((uint16_t)buf[2u * idx] | ((uint16_t)buf[2u * idx + 1u] << 8));
        }
        /* If the total length is odd, add the last byte and a fill byte of 0 for checksum calculation */
        if ((len & 1u) != 0u) {
            checksum += (uint16_t)buf[len - 1u]; /* Last byte + 0x00 fill byte */
        }
        #else /* XCP_ON_SXI_CHECKSUM_BYTE */
        /* Calculate BYTE checksum over header and payload */
        for (idx = 0; idx < len; idx++) {
            checksum += buf[idx];
        }
        #endif

        /* Send header + payload */
        #if (XCP_ON_SXI_ENABLE_FRAMING == XCP_ON)
        XcpTl_TransformAndSend(buf, len);
        #else
        Serial_WriteBuffer(buf, len);
        #endif

        #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_WORD)
        {
            const uint16_t header_off  = (uint16_t)XCP_TRANSPORT_LAYER_BUFFER_OFFSET;
            const uint16_t payload_len = (len >= header_off) ? (len - header_off) : 0u;

            /* Add fill byte to stream if payload length is odd for word checksum */
            if ((payload_len & 1u) != 0u) {
                uint8_t fill_byte = 0x00;
            #if (XCP_ON_SXI_ENABLE_FRAMING == XCP_ON)
                XcpTl_TransformAndSend(&fill_byte, 1);
            #else
                Serial_WriteBuffer(&fill_byte, 1);
            #endif
            }
        }
        #endif

        /* Append checksum to stream */
        #if (XCP_ON_SXI_TAIL_CHECKSUM == XCP_ON_SXI_CHECKSUM_WORD)
        {
            uint8_t checksum_bytes[2];
            checksum_bytes[0] = (uint8_t)(checksum);      /* Little Endian: LSB */
            checksum_bytes[1] = (uint8_t)(checksum >> 8); /* Little Endian: MSB */
            #if (XCP_ON_SXI_ENABLE_FRAMING == XCP_ON)
            XcpTl_TransformAndSend(checksum_bytes, 2);
            #else
            Serial_WriteBuffer(checksum_bytes, 2);
            #endif
        }
        #else /* XCP_ON_SXI_CHECKSUM_BYTE */
        {
            uint8_t checksum_byte = (uint8_t)checksum;
            #if (XCP_ON_SXI_ENABLE_FRAMING == XCP_ON)
            XcpTl_TransformAndSend(&checksum_byte, 1);
            #else
            Serial_WriteBuffer(&checksum_byte, 1);
            #endif
        }
        #endif
    }
    #else
        #if (XCP_ON_SXI_ENABLE_FRAMING == XCP_ON)
    XcpTl_TransformAndSend(buf, len);
        #else
    Serial_WriteBuffer(buf, len);
        #endif
    #endif /* XCP_ON_SXI_TAIL_CHECKSUM != XCP_ON_SXI_NO_CHECKSUM */

    XCP_TL_LEAVE_CRITICAL();
}

void XcpTl_SaveConnection(void) {
}

void XcpTl_ReleaseConnection(void) {
}

void XcpTl_PrintConnectionInformation(void) {
    #if 0
        #if defined(ARDUINO)
Serial.println("XCPonSxi");
        #else
printf("\nXCPonSxi\n");
        #endif
    #endif
}

static void XcpTl_SignalTimeout(void) {
    XCP_TL_ENTER_CRITICAL();
    XcpTl_ResetSM();
    XCP_TL_LEAVE_CRITICAL();
}

#endif /* XCP_TRANSPORT_LAYER == XCP_ON_SXI */

////////////////////////////////////////////////////////////////////////////////
//                                 ardbt.cpp                                  //
////////////////////////////////////////////////////////////////////////////////

/*
 * BlueParrot XCP - Arduino BLE Transport (Peripheral)
 *
 * Provides a BLE Peripheral with two characteristics:
 * - RX (Write Without Response): Host writes XCP request frames
 * - TX (Notify): Device notifies XCP response frames
 *
 * Frame format on RX:
 *   [len_lo, len_hi, ctr_lo, ctr_hi, payload...]
 * Where len is payload size (without header), ctr is a simple counter (ignored here).
 *
 * Outgoing TX uses raw XCP response data payload (no header) like other TLs.
 *
 * Boards: e.g. Arduino Nicla Sense ME (nRF52), Nano 33 BLE, etc.
 */

#include "xcp_config.h"

#if (XCP_TRANSPORT_LAYER == XCP_ON_BT) && defined(ARDUINO)

    #include <Arduino.h>
    #include <ArduinoBLE.h>


/* Nordic UART Service (NUS)-style UUIDs */
static const char* BLE_SVC_UUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
static const char* BLE_RX_UUID  = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"; /* Write (w/o response) */
static const char* BLE_TX_UUID  = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"; /* Notify */

/* BLE objects */
static BLEService        XcpBleService(BLE_SVC_UUID);
static BLECharacteristic XcpBleRx(BLE_RX_UUID, BLEWriteWithoutResponse, XCP_COMM_BUFLEN);
static BLECharacteristic XcpBleTx(BLE_TX_UUID, BLENotify, XCP_COMM_BUFLEN);

/* Simple RX buffer for one incoming frame */
static uint8_t  XcpBle_RxBuf[XCP_COMM_BUFLEN];
static uint16_t XcpBle_RxLen = 0U;

/* Optional: connection state */
static bool XcpBle_CentralConnected = false;
/* Staged frame for processing in RxHandler (mirrors CAN TL pattern) */
static volatile bool XcpBle_FrameReceived = false;
static uint8_t       XcpBle_FrameBuf[XCP_COMM_BUFLEN];
static uint16_t      XcpBle_FrameLen = 0U;

static inline uint16_t u16_le(const uint8_t* p) {
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

static bool XcpBle_HandleWrite(void) {
    int avail = XcpBleRx.valueLength();
    if (avail <= 0) {
        return false;
    }
    if ((size_t)avail > sizeof(XcpBle_RxBuf)) {
        /* Drop over-sized write */
        return false;
    }

    int rd = XcpBleRx.readValue(XcpBle_RxBuf, (size_t)avail);
    if (rd <= 0) {
        return false;
    }

    /* Expect: len(2) + ctr(2) + payload(len) */
    if (rd < 4) {
        return false;
    }
    uint16_t len = u16_le(&XcpBle_RxBuf[0]);
    /* uint16_t ctr = u16_le(&XcpBle_RxBuf[2]); -- currently unused */

    if ((uint16_t)(rd - 4) != len) {
        /* Malformed frame; length mismatch */
        return false;
    }
    if (len > XCP_TRANSPORT_LAYER_CTO_BUFFER_SIZE) {
        /* Payload exceeds TL buffer limit */
        return false;
    }

    /* Stage frame for processing in RxHandler (consistent with CAN driver) */
    XcpBle_FrameLen = len;
    XcpUtl_MemCopy(XcpBle_FrameBuf, &XcpBle_RxBuf[4], len);
    XcpBle_FrameReceived = true;
    return true;
}

void XcpTl_Init(void) {
    /* Begin BLE stack */
    if (!BLE.begin()) {
        /* If BLE can't start, nothing else we can do here on Arduino */
        return;
    }

    /* Set advertised name and service */
    BLE.setLocalName("XCP-BLE");
    BLE.setDeviceName("XCP-BLE");
    BLE.setAdvertisedService(XcpBleService);

    /* Configure characteristics */
    XcpBleTx.setValue((const uint8_t*)"", 0);
    XcpBleService.addCharacteristic(XcpBleRx);
    XcpBleService.addCharacteristic(XcpBleTx);

    /* Add service and start advertising */
    BLE.addService(XcpBleService);
    BLE.advertise();

    XcpBle_CentralConnected = false;
    XcpBle_RxLen            = 0U;
}

void XcpTl_DeInit(void) {
    BLE.stopAdvertise();
    BLE.end();
    XcpBle_CentralConnected = false;
}

void XcpTl_MainFunction(void) {
    /* Poll BLE events */
    BLEDevice central = BLE.central();

    if (central && central.connected()) {
        if (!XcpBle_CentralConnected) {
            XcpBle_CentralConnected = true;
        }
        /* Handle inbound writes to RX characteristic */
        if (XcpBleRx.written()) {
            (void)XcpBle_HandleWrite();
        }
    } else {
        if (XcpBle_CentralConnected) {
            XcpBle_CentralConnected = false;
        }
    }
    /* Process staged frame if available (like CAN TL) */
    if (XcpTl_FrameAvailable(0, 0) > 0) {
        XcpTl_RxHandler();
    }
}

void XcpTl_Send(uint8_t const * buf, uint16_t len) {
    if (!buf || len == 0U) {
        return;
    }
    /* BLE Characteristic notifications are bounded by MTU/char size.
       We chunk if needed. */
    const uint16_t mtu  = (uint16_t)XCP_COMM_BUFLEN; /* Using characteristic buffer size as cap */
    uint16_t       sent = 0U;

    XCP_TL_ENTER_CRITICAL();
    while (sent < len) {
        uint16_t chunk = (uint16_t)((len - sent) > mtu ? mtu : (len - sent));
        /* Best effort: setValue triggers a notify for BLENotify characteristics */
        if (!XcpBleTx.setValue((const uint8_t*)(buf + sent), chunk)) {
            /* If notify fails, abort this send to avoid busy loop */
            break;
        }
        sent = (uint16_t)(sent + chunk);
        /* Give BLE stack a chance; optional small delay */
        /* delay(1); */
    }
    XCP_TL_LEAVE_CRITICAL();
}

void XcpTl_SaveConnection(void) {
    /* Not needed on BLE peripheral */
}

void XcpTl_ReleaseConnection(void) {
    /* Not needed on BLE peripheral */
}

void XcpTl_PrintConnectionInformation(void) {
    /* On Arduino, print via Serial if available */
    #if defined(SERIAL_PORT_MONITOR) || defined(Serial)
    if (Serial) {
        Serial.println("XCP over BLE Peripheral (NUS)");
    }
    #endif
}

void XcpTl_RxHandler(void) {
    /* Consume the staged frame and dispatch to XCP core */
    if (!XcpBle_FrameReceived) {
        return;
    }
    XcpBle_FrameReceived = false;
    XcpUtl_MemCopy(Xcp_CtoIn.data, XcpBle_FrameBuf, XcpBle_FrameLen);
    Xcp_CtoIn.len = XcpBle_FrameLen;
    Xcp_DispatchCommand(&Xcp_CtoIn);
}

int16_t XcpTl_FrameAvailable(uint32_t sec, uint32_t usec) {
    (void)sec;
    (void)usec;
    return (int16_t)(XcpBle_FrameReceived ? 1 : 0);
}

#endif /* (XCP_TRANSPORT_LAYER == XCP_ON_BT) && defined(ARDUINO) */

////////////////////////////////////////////////////////////////////////////////
//                              arduino_can.cpp                               //
////////////////////////////////////////////////////////////////////////////////


/*
 * BlueParrot XCP
 *
 * (C) 2022-2025 by Christoph Schueler <github.com/Christoph2,
 *                                      cpu12.gems@googlemail.com>
 *
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * s. FLOSS-EXCEPTION.txt
 */

#include "xcp_config.h"

#if XCP_TRANSPORT_LAYER == XCP_ON_CAN

    #if (XCP_CAN_INTERFACE == XCP_CAN_IF_SEED_STUDIO_CAN_SHIELD) || (XCP_CAN_INTERFACE == XCP_CAN_IF_SEED_STUDIO_CAN_FD_SHIELD)
        #include <SPI.h>
        #include <can-serial.h>
        #include <mcp2515_can.h>
        #include <mcp2515_can_dfs.h>
        #include <mcp2518fd_can.h>
        #include <mcp2518fd_can_dfs.h>
        #include <mcp_can.h>
    #elif XCP_CAN_INTERFACE == XCP_CAN_IF_SPARKFUN_CAN_SHIELD
        #include <Canbus.h>
        #include <defaults.h>
        #include <global.h>
        #include <mcp2515.h>
        #include <mcp2515_defs.h>
    #elif XCP_CAN_INTERFACE == XCP_CAN_IF_MKR_ZERO_CAN_SHIELD
        #include <CAN.h>
    #else
        #error "Unsupported XCP_CAN_INTERFACE"
    #endif

    #include <stdint.h>

uint32_t              filter_mask(uint32_t identifier);
extern const uint32_t Xcp_DaqIDs[];
extern const uint16_t Xcp_DaqIDCount;

static const char XCP_MAGIC[] = "XCP";

static bool          connected = false;
static volatile bool XcpTl_FrameReceived{ false };

static unsigned char XcpTl_Buffer[64];
static unsigned char XcpTl_Dlc = 0;
static int           XcpTl_ID  = 0;

static void on_receive_seedstudio();
static void on_receive_mkr(int packetSize);

    #if (XCP_CAN_INTERFACE == XCP_CAN_IF_SEED_STUDIO_CAN_SHIELD) || (XCP_CAN_INTERFACE == XCP_CAN_IF_SEED_STUDIO_CAN_FD_SHIELD)
        #if XCP_CAN_INTERFACE == XCP_CAN_IF_SEED_STUDIO_CAN_SHIELD
mcp2515_can CAN(XCP_CAN_IF_MCP25XX_PIN_CS);
        #elif XCP_CAN_INTERFACE == XCP_CAN_IF_SEED_STUDIO_CAN_FD_SHIELD
mcp2518fd CAN(XCP_CAN_IF_MCP25XX_PIN_CS);
        #endif
    #elif XCP_CAN_INTERFACE == XCP_CAN_IF_MKR_ZERO_CAN_SHIELD
    /* MKR Zero CAN shield uses built-in CAN object; no explicit declaration needed here. */
    #else
        #error "Unsupported XCP_CAN_INTERFACE"
    #endif

void XcpTl_Init(void) {
    Serial.begin(9600);
    while (!Serial) {
    }

    Serial.println("Starting Blueparrot XCP...");

    #if (XCP_CAN_INTERFACE == XCP_CAN_IF_SEED_STUDIO_CAN_SHIELD) || (XCP_CAN_INTERFACE == XCP_CAN_IF_SEED_STUDIO_CAN_FD_SHIELD)
    attachInterrupt(digitalPinToInterrupt(XCP_CAN_IF_MCP25XX_PIN_INT), on_receive_seedstudio, FALLING);

    while (CAN_OK != CAN.begin(XCP_ON_CAN_FREQ)) {
        Serial.println("CAN init (SeedStudio) failed!, retry...");
        delay(100);
    }

    Serial.println("CAN init (SeedStudio) OK!");

    // #if 0
    CAN.init_Mask(0, XCP_ON_CAN_IS_EXTENDED_IDENTIFIER(XCP_ON_CAN_INBOUND_IDENTIFIER), filter_mask(XCP_ON_CAN_INBOUND_IDENTIFIER));
    CAN.init_Mask(
        1, XCP_ON_CAN_IS_EXTENDED_IDENTIFIER(XCP_ON_CAN_BROADCAST_IDENTIFIER), filter_mask(XCP_ON_CAN_BROADCAST_IDENTIFIER)
    );
    //   #endif
    // CAN.init_Mask(0, 0, 0);
    // CAN.init_Mask(1, 0, 0);

    CAN.init_Filt(0, XCP_ON_CAN_IS_EXTENDED_IDENTIFIER(XCP_ON_CAN_INBOUND_IDENTIFIER), XCP_ON_CAN_INBOUND_IDENTIFIER);
    CAN.init_Filt(1, XCP_ON_CAN_IS_EXTENDED_IDENTIFIER(XCP_ON_CAN_BROADCAST_IDENTIFIER), XCP_ON_CAN_BROADCAST_IDENTIFIER);
    #elif XCP_CAN_INTERFACE == XCP_CAN_IF_SPARKFUN_CAN_SHIELD
    if (Canbus.init(CANSPEED_500)) {  // TDO: Make speed configurable
        Serial.println("CAN init (SparkFun) OK!");
    } else {
        Serial.println("CAN init (SparkFun) failed!");
        while (1) {
        }
    }
    #elif XCP_CAN_INTERFACE == XCP_CAN_IF_MKR_ZERO_CAN_SHIELD

    if (!CAN.begin(XCP_ON_CAN_FREQ)) {
        Serial.println("Starting CAN (MKR Zero) failed!");
        while (1) {
        }
    } else {
        Serial.println("CAN init ((MKR Zero) OK!");
    }
    CAN.setTimeout(1000);
    CAN.onReceive(on_receive_mkr);
    #else
        #error "Unsupported XCP_CAN_INTERFACE"
    #endif
}

void XcpTl_DeInit(void) {
}

void *XcpTl_Thread(void *param) {
    XCP_FOREVER {
        XcpTl_MainFunction();
    }
    return NULL;
}

void XcpTl_MainFunction(void) {
    if (XcpTl_FrameAvailable(0, 1000) > 0) {
        XcpTl_FrameReceived = false;
        XcpTl_RxHandler();
    }
}

    #if (XCP_CAN_INTERFACE == XCP_CAN_IF_SEED_STUDIO_CAN_SHIELD) || (XCP_CAN_INTERFACE == XCP_CAN_IF_SEED_STUDIO_CAN_FD_SHIELD)
static void on_receive_seedstudio() {
    if (CAN.checkReceive() == CAN_MSGAVAIL) {
        XcpTl_FrameReceived = true;
        CAN.readMsgBuf(&XcpTl_Dlc, static_cast<byte *>(XcpTl_Buffer));
        // XcpTl_ID
        //  canBusPacket.id = CAN.getCanId();
    }
}
    #elif XCP_CAN_INTERFACE == XCP_CAN_IF_MKR_ZERO_CAN_SHIELD
static void on_receive_mkr(int packetSize) {
    uint_least8_t idx   = 0;
    XcpTl_Dlc           = packetSize;
    XcpTl_FrameReceived = true;
    if (CAN.packetExtended()) {
    }
    if (CAN.packetRtr()) {
    }
    if (CAN.packetRtr()) {
    } else {
        while (CAN.available()) {
            XcpTl_Buffer[idx++] = CAN.read();
        }
    }
}
    #endif

void XcpTl_RxHandler(void) {
    XcpUtl_MemCopy(Xcp_CtoIn.data, XcpTl_Buffer, XcpTl_Dlc);
    Xcp_DispatchCommand(&Xcp_CtoIn);
}

void XcpTl_TxHandler(void) {
}

int16_t XcpTl_FrameAvailable(uint32_t sec, uint32_t usec) {
    #if XCP_CAN_INTERFACE == XCP_CAN_IF_SPARKFUN_CAN_SHIELD
    if (mcp2515_check_message()) {
        if (mcp2515_get_message(&message)) {
            XcpTl_FrameReceived = true;
            XcpTl_Dlc           = message.header.length;
            XcpTl_ID            = message.id;
            for (int i = 0; i < message.header.length; i++) {
                XcpTl_Buffer[i] = message.data[i];
            }
            return 1;
        }
    }
    return 0;
    #else
    return static_cast<int16_t>(XcpTl_FrameReceived);
    #endif
}

void XcpTl_Send(uint8_t const *buf, uint16_t len) {
    uint32_t can_id;

    can_id = XCP_ON_CAN_STRIP_IDENTIFIER(XCP_ON_CAN_OUTBOUND_IDENTIFIER);

    #if (XCP_CAN_INTERFACE == XCP_CAN_IF_SEED_STUDIO_CAN_SHIELD) || (XCP_CAN_INTERFACE == XCP_CAN_IF_SEED_STUDIO_CAN_FD_SHIELD)

    CAN.sendMsgBuf(can_id, XCP_ON_CAN_IS_EXTENDED_IDENTIFIER(XCP_ON_CAN_OUTBOUND_IDENTIFIER), len, buf);

    #elif XCP_CAN_INTERFACE == XCP_CAN_IF_SPARKFUN_CAN_SHIELD
    message.id            = can_id;
    message.header.rtr    = 0;
    message.header.length = len;
    for (int i = 0; i < len; i++) {
        message.data[i] = buf[i];
    }
    mcp2515_bit_modify(CANCTRL, (1 << REQOP2) | (1 << REQOP1) | (1 << REQOP0), 0);
    mcp2515_send_message(&message);
    #elif XCP_CAN_INTERFACE == XCP_CAN_IF_MKR_ZERO_CAN_SHIELD

    if (XCP_ON_CAN_IS_EXTENDED_IDENTIFIER(XCP_ON_CAN_OUTBOUND_IDENTIFIER)) {
        CAN.beginExtendedPacket(can_id, len);
    } else {
        CAN.beginPacket(can_id);
    }

    CAN.write(buf, len);
    CAN.endPacket();
    #else
        #error "Unsupported XCP_CAN_INTERFACE"
    #endif
}

uint32_t filter_mask(uint32_t identifier) {
    if (XCP_ON_CAN_IS_EXTENDED_IDENTIFIER(identifier)) {
        return (2UL << (29 - 1)) - 1;
    } else {
        return (2UL << (11 - 1)) - 1;
    }
}

void XcpTl_SaveConnection(void) {
    connected = XCP_TRUE;
}

void XcpTl_ReleaseConnection(void) {
    connected = XCP_FALSE;
}

bool XcpTl_VerifyConnection(void) {
    return connected;
}

void XcpTl_PrintConnectionInformation(void) {
}

    #if XCP_ENABLE_TRANSPORT_LAYER_CMD == XCP_ON
        #if (XCP_ENABLE_CAN_GET_SLAVE_ID == XCP_ON)
void XcpTl_GetSlaveId_Res(Xcp_PduType const * const pdu) {
    uint8_t mask = 0x00;

    if (pdu->data[2]) {
    }

    if (pdu->data[5] == 1) {
        /*
         Mode
            0 = identify by echo
            1 = confirm by inverse echo
        */
        mask = 0xff;
    }

    Xcp_Send8(
        UINT8(8), UINT8(XCP_PACKET_IDENTIFIER_RES), UINT8(0x58 ^ mask), UINT8(0x43 ^ mask), UINT8(0x50 ^ mask),
        XCP_LOBYTE(XCP_LOWORD(XCP_ON_CAN_OUTBOUND_IDENTIFIER)), XCP_HIBYTE(XCP_LOWORD(XCP_ON_CAN_OUTBOUND_IDENTIFIER)),
        XCP_LOBYTE(XCP_HIWORD(XCP_ON_CAN_OUTBOUND_IDENTIFIER)), XCP_HIBYTE(XCP_HIWORD(XCP_ON_CAN_OUTBOUND_IDENTIFIER))
    );
}
        #endif /* XCP_ENABLE_CAN_GET_SLAVE_ID */

        #if (XCP_ENABLE_CAN_GET_DAQ_ID == XCP_ON)
void XcpTl_GetDaqId_Res(Xcp_PduType const * const pdu) {
    uint8_t daq_id = pdu->data[2];

    if (daq_id > (Xcp_DaqIDCount - 1)) {
        Xcp_ErrorResponse(UINT8(ERR_OUT_OF_RANGE));
        return;
    };
    Xcp_Send8(
        UINT8(8), UINT8(XCP_PACKET_IDENTIFIER_RES), UINT8(1), UINT8(0), UINT8(0), XCP_LOBYTE(XCP_LOWORD(Xcp_DaqIDs[daq_id])),
        XCP_HIBYTE(XCP_LOWORD(Xcp_DaqIDs[daq_id])), XCP_LOBYTE(XCP_HIWORD(Xcp_DaqIDs[daq_id])),
        XCP_HIBYTE(XCP_HIWORD(Xcp_DaqIDs[daq_id]))
    );
}
        #endif /* XCP_ENABLE_CAN_GET_DAQ_ID */

        #if (XCP_ENABLE_CAN_SET_DAQ_ID == XCP_ON)
void XcpTl_SetDaqId_Res(Xcp_PduType const * const pdu) {
}
        #endif /* XCP_ENABLE_CAN_SET_DAQ_ID */

void XcpTl_TransportLayerCmd_Res(Xcp_PduType const * const pdu) {
        #if (XCP_ENABLE_CAN_GET_SLAVE_ID == XCP_ON)
    if (pdu->data[1] == UINT8(XCP_GET_SLAVE_ID)) {
        XcpTl_GetSlaveId_Res(pdu);  // TODO: This is a response to a broadcast message.
        return;
    }
        #endif /* XCP_ENABLE_CAN_GET_SLAVE_ID */

        #if (XCP_ENABLE_CAN_GET_DAQ_ID == XCP_ON)
    if (pdu->data[1] == UINT8(XCP_GET_DAQ_ID)) {
        XcpTl_GetDaqId_Res(pdu);
        return;
    }
        #endif /* XCP_ENABLE_CAN_GET_DAQ_ID */

        #if (XCP_ENABLE_CAN_SET_DAQ_ID == XCP_ON)
    if (pdu->data[1] == UINT8(XCP_SET_DAQ_ID)) {
        XcpTl_SetDaqId_Res(pdu);
        return;
    }
        #endif /* XCP_ENABLE_CAN_SET_DAQ_ID */
    Xcp_ErrorResponse(UINT8(ERR_CMD_UNKNOWN));
}
    #endif /* XCP_ENABLE_TRANSPORT_LAYER_CMD */

#endif /* XCP_TRANSPORT_LAYER == XCP_ON_CAN */

/////
#define SIZE 1000

// A class to store a queue

////////////////////////////////////////////////////////////////////////////////
//                           arduino_networking.cpp                           //
////////////////////////////////////////////////////////////////////////////////


/*
 * BlueParrot XCP
 *
 * (C) 2007-2025 by Christoph Schueler <github.com/Christoph2,
 *                                      cpu12.gems@googlemail.com>
 *
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * s. FLOSS-EXCEPTION.txt
 */


#if (XCP_TRANSPORT_LAYER == XCP_ON_ETHERNET) && (defined(ARDUINO))

    #include <Arduino.h>

    #ifdef ESP32
        #include <WiFi.h>
        #include <WiFiUdp.h>
    #else
        #include <Ethernet.h>
        #include <EthernetUdp.h>
        #include <SPI.h>
    #endif

const unsigned long READ_TIMEOUT_MS = 2000;

class ClientWrapper {
   public:

    virtual int    available()                                 = 0;
    virtual int    read_bytes(uint8_t* buf, size_t len)        = 0;
    virtual size_t write_bytes(const uint8_t* buf, size_t len) = 0;
    virtual bool   connected()                                 = 0;
    virtual void   stop()                                      = 0;
    virtual bool   is_datagram() const                         = 0;

    virtual ~ClientWrapper() {
    }
};

class ArduinoNetworkIf {
   public:

    virtual bool           begin()         = 0;
    virtual ClientWrapper* accept_client() = 0;

    virtual void process() {
    }

    virtual String getInfo() {
        return String();
    }

    virtual ~ArduinoNetworkIf() {
    }
};

    #if XCP_ON_ETHERNET_ARDUINO_DRIVER == XCP_ON_ETHERNET_DRIVER_ETHERNET
class EthernetClientWrapper : public ClientWrapper {
   public:

    EthernetClientWrapper(EthernetClient c) : m_client(c) {
    }

    int available() override {
        return m_client.available();
    }

    int read_bytes(uint8_t* buf, size_t len) override {
        return m_client.read(buf, len);
    }

    size_t write_bytes(const uint8_t* buf, size_t len) override {
        size_t nbytes = m_client.write(buf, len);
        return nbytes;
    }

    bool connected() override {
        return m_client && m_client.connected();
    }

    void stop() override {
        if (m_client) {
            m_client.stop();
        }
    }

    bool is_datagram() const override {
        return false;
    }

   private:

    EthernetClient m_client;
};

class EthernetUdpClientWrapper : public ClientWrapper {
   public:

    EthernetUdpClientWrapper() : m_udp(nullptr), m_remoteIp(), m_remotePort(0), m_size(0), m_offset(0) {
    }

    void reset(EthernetUDP* udp, IPAddress remoteIp, uint16_t remotePort) {
        m_udp        = udp;
        m_remoteIp   = remoteIp;
        m_remotePort = remotePort;
        m_size       = 0;
        m_offset     = 0;
    }

    int loadFromUdp(int packetSize) {
        if (!m_udp)
            return -1;
        if (packetSize <= 0)
            return -1;
        if (packetSize > (int)XCP_COMM_BUFLEN)
            packetSize = (int)XCP_COMM_BUFLEN;
        int len = m_udp->read(m_buf, (size_t)packetSize);
        if (len <= 0) {
            m_size   = 0;
            m_offset = 0;
            return -1;
        }
        m_size   = (size_t)len;
        m_offset = 0;
        return len;
    }

    int available() override {
        return (int)(m_size - m_offset);
    }

    int read_bytes(uint8_t* buf, size_t len) override {
        size_t remaining = m_size - m_offset;
        size_t toCopy    = remaining < len ? remaining : len;
        if (toCopy == 0)
            return 0;
        memcpy(buf, m_buf + m_offset, toCopy);
        m_offset += toCopy;
        return (int)toCopy;
    }

    size_t write_bytes(const uint8_t* buf, size_t len) override {
        if (!m_udp)
            return 0;
        if (m_udp->beginPacket(m_remoteIp, m_remotePort) == 0)
            return 0;
        size_t written = m_udp->write(buf, len);
        m_udp->endPacket();
        return written;
    }

    bool connected() override {
        return m_offset < m_size;
    }

    void stop() override {
        // no persistent connection to stop for UDP
    }

    bool is_datagram() const override {
        return true;
    }

   private:

    EthernetUDP* m_udp;
    IPAddress    m_remoteIp;
    uint16_t     m_remotePort;
    uint8_t      m_buf[XCP_COMM_BUFLEN];
    size_t       m_size;
    size_t       m_offset;
};

class EthernetAdapter : public ArduinoNetworkIf {
   public:

    EthernetAdapter(uint16_t port) : m_port(port), m_use_dhcp(true), m_ip(0, 0, 0, 0) {
    }

    EthernetAdapter(IPAddress ip, uint16_t port) : m_port(port), m_use_dhcp(false), m_ip(ip) {
    }

    bool begin() override {
        Serial.begin(115200);
        while (!Serial) {
            ;  // wait for serial port to connect. Needed for native USB port only
        }
        Serial.println("\nBlueparrot XCP (UDP) on Ethernet");
        Ethernet.init(10);
        delay(1000);  // Give the hardware some time to initialize

        int result = 0;
        if (m_use_dhcp) {
            Serial.println("Attempting to get an IP address using DHCP...");
            result = Ethernet.begin(m_mac_address);  // try DHCP
            if (result == 0) {
                Serial.println("Failed to configure Ethernet using DHCP");
                return false;
            }
        } else {
            Serial.print("Configuring Ethernet with static IP: ");
            Serial.println(m_ip);
            Ethernet.begin(m_mac_address, m_ip);  // use static IP
            result = 1;                           // Assume success for static IP, as begin() doesn't indicate failure
        }

        // Check for a valid IP address
        if (Ethernet.localIP() == IPAddress(0, 0, 0, 0)) {
            Serial.println("Failed to get a valid IP address.");
            return false;
        }
        m_udp.begin(m_port);
        return true;
    }

    ClientWrapper* accept_client() override {
        int packetSize = m_udp.parsePacket();
        if (packetSize > 0) {
            if (packetSize > (int)XCP_COMM_BUFLEN)
                packetSize = (int)XCP_COMM_BUFLEN;
            IPAddress rip   = m_udp.remoteIP();
            uint16_t  rport = m_udp.remotePort();
            m_udpClientWrapper.reset(&m_udp, rip, rport);
            int len = m_udpClientWrapper.loadFromUdp(packetSize);
            if (len <= 0)
                return nullptr;
            return &m_udpClientWrapper;
        }
        return nullptr;
    }

    String getInfo() override {
        IPAddress ip = Ethernet.localIP();
        return String("Ethernet-UDP ") + String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]);
    }

   private:

    uint8_t                  m_mac_address_storage[6] = XCP_ON_ETHERNET_MAC_ADDRESS;
    uint8_t*                 m_mac_address            = m_mac_address_storage;
    bool                     m_use_dhcp;
    IPAddress                m_ip;
    uint16_t                 m_port;
    EthernetUDP              m_udp;
    EthernetUdpClientWrapper m_udpClientWrapper;  // preallocated UDP client wrapper
};
    #endif  // XCP_ON_ETHERNET_ARDUINO_DRIVER == XCP_ON_ETHERNET_DRIVER_ETHERNET

    #if XCP_ON_ETHERNET_ARDUINO_DRIVER == XCP_ON_ETHERNET_DRIVER_WIFI

class WiFiUdpClientWrapper : public ClientWrapper {
   public:

    WiFiUdpClientWrapper() : m_udp(nullptr), m_remoteIp(), m_remotePort(0), m_size(0), m_offset(0) {
    }

    void reset(WiFiUDP* udp, IPAddress remoteIp, uint16_t remotePort) {
        m_udp        = udp;
        m_remoteIp   = remoteIp;
        m_remotePort = remotePort;
        m_size       = 0;
        m_offset     = 0;
    }

    int loadFromUdp(int packetSize) {
        if (!m_udp)
            return -1;
        if (packetSize <= 0)
            return -1;
        if (packetSize > (int)XCP_COMM_BUFLEN)
            packetSize = (int)XCP_COMM_BUFLEN;
        int len = m_udp->read(m_buf, (size_t)packetSize);
        if (len <= 0) {
            m_size   = 0;
            m_offset = 0;
            return -1;
        }
        m_size   = (size_t)len;
        m_offset = 0;
        return len;
    }

    int available() override {
        return (int)(m_size - m_offset);
    }

    int read_bytes(uint8_t* buf, size_t len) override {
        size_t remaining = m_size - m_offset;
        size_t toCopy    = remaining < len ? remaining : len;
        if (toCopy == 0)
            return 0;
        memcpy(buf, m_buf + m_offset, toCopy);
        m_offset += toCopy;
        return (int)toCopy;
    }

    size_t write_bytes(const uint8_t* buf, size_t len) override {
        if (!m_udp)
            return 0;
        if (m_udp->beginPacket(m_remoteIp, m_remotePort) == 0)
            return 0;
        size_t written = m_udp->write(buf, len);
        m_udp->endPacket();
        return written;
    }

    bool connected() override {
        return m_offset < m_size;
    }

    void stop() override {
        // no-op for UDP
    }

    bool is_datagram() const override {
        return true;
    }

   private:

    WiFiUDP*  m_udp;
    IPAddress m_remoteIp;
    uint16_t  m_remotePort;
    uint8_t   m_buf[XCP_COMM_BUFLEN];
    size_t    m_size;
    size_t    m_offset;
};

class WiFiAdapter : public ArduinoNetworkIf {
   public:

    WiFiAdapter(const char* s, const char* p, uint16_t port) : m_port(port), m_ssid(s), m_pass(p) {
    }

    bool begin() override {
        // Put ESP32 in station mode and try to keep things deterministic during connect
        WiFi.persistent(false);  // avoid writing credentials repeatedly to NVS
        WiFi.mode(WIFI_STA);
        WiFi.setSleep(false);  // keep radio awake during provisioning/UDP XCP
                               // Optional set TX power (requires Arduino-ESP32 supporting WiFi.setTxPower)
        #ifdef XCP_ON_WIFI_TX_POWER
        WiFi.setTxPower(XCP_ON_WIFI_TX_POWER);
        #endif

        // Optional static IP configuration (do this BEFORE begin())
        {
        #if defined(XCP_ON_ETHERNET_IP_OCTETS)
            IPAddress ip(XCP_ON_ETHERNET_IP_OCTETS);
            if (!WiFi.config(ip)) {
                Serial.println("[WiFi] Failed to apply static IP config");
            }
        #elif defined(XCP_ON_ETHERNET_IP)
            IPAddress ip;
            ip.fromString(XCP_ON_ETHERNET_IP);
            if (!WiFi.config(ip)) {
                Serial.println("[WiFi] Failed to apply static IP config");
            }
        #endif
        }

        Serial.print("[WiFi] Connecting to SSID: ");
        Serial.println(m_ssid ? m_ssid : "<null>");
        wl_status_t startStatus = WiFi.begin(m_ssid, m_pass);
        (void)startStatus;  // not all cores return a meaningful value here

        const unsigned long timeoutMs = 15000;  // allow a bit more time on congested APs
        unsigned long       start     = millis();
        wl_status_t         st;
        while ((st = WiFi.status()) != WL_CONNECTED && (millis() - start < timeoutMs)) {
            switch (st) {
                case WL_CONNECT_FAILED:
                    Serial.println("[WiFi] Connect failed (bad SSID/Password?)");
                    break;
                case WL_NO_SSID_AVAIL:
                    Serial.println("[WiFi] SSID not available");
                    break;
                case WL_DISCONNECTED:
                case WL_IDLE_STATUS:
                default:
                    break;
            }
            delay(200);
        }

        bool connected = (WiFi.status() == WL_CONNECTED);
        if (connected) {
            IPAddress ip = WiFi.localIP();
            Serial.print("\nBlueparrot XCP (UDP) on WiFi -- IP: ");
            Serial.println(ip);
            if (!m_udp_active) {
                if (m_udp.begin(m_port)) {
                    m_udp_active = true;
                } else {
                    Serial.println("[WiFi] Failed to start UDP listener");
                }
            }
        } else {
            Serial.println("[WiFi] Not connected after timeout");
        }
        return connected;
    }

    ClientWrapper* accept_client() override {
        // Avoid UDP operations while STA is connecting; ESP-IDF logs: "wifi:sta is connecting, return error"
        if (WiFi.status() != WL_CONNECTED || !m_udp_active) {
            return nullptr;
        }
        int packetSize = m_udp.parsePacket();
        if (packetSize > 0) {
            if (packetSize > (int)XCP_COMM_BUFLEN)
                packetSize = (int)XCP_COMM_BUFLEN;
            IPAddress rip   = m_udp.remoteIP();
            uint16_t  rport = m_udp.remotePort();
            m_udpClientWrapper.reset(&m_udp, rip, rport);
            int len = m_udpClientWrapper.loadFromUdp(packetSize);
            if (len <= 0)
                return nullptr;
            return &m_udpClientWrapper;
        }
        return nullptr;
    }

    void process() override {
        // Keep UDP aligned with link state and perform gentle reconnects.
        static unsigned long lastAttempt = 0;
        wl_status_t          st          = WiFi.status();

        if (st == WL_CONNECTED) {
            if (!m_udp_active) {
                if (m_udp.begin(m_port)) {
                    m_udp_active = true;
                    Serial.println("[WiFi] UDP listener started after reconnect");
                }
            }
            m_reconnect_backoff_ms = 2000;  // reset backoff on success
        } else {
            // Ensure UDP is not used while disconnected/connecting
            if (m_udp_active) {
                m_udp.stop();
                m_udp_active = false;
            }
            // Throttled reconnect attempts
            unsigned long now = millis();
            if (now - lastAttempt >= m_reconnect_backoff_ms) {
                lastAttempt = now;
                Serial.println("[WiFi] Attempting reconnect...");
                WiFi.reconnect();
                // Exponential backoff up to 30s
                if (m_reconnect_backoff_ms < 30000) {
                    m_reconnect_backoff_ms = m_reconnect_backoff_ms * 2;
                }
            }
        }
    }

    String getInfo() override {
        if (WiFi.status() == WL_CONNECTED) {
            IPAddress ip = WiFi.localIP();
            return String("WiFi-UDP ") + String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]);
        }
        return String("WiFi-UDP <disconnected>");
    }

   private:

    uint16_t             m_port;
    WiFiUDP              m_udp;
    const char*          m_ssid;
    const char*          m_pass;
    WiFiUdpClientWrapper m_udpClientWrapper;
    bool                 m_udp_active           = false;
    unsigned long        m_reconnect_backoff_ms = 2000;  // start with 2s backoff
};
    #endif  // XCP_ON_ETHERNET_ARDUINO_DRIVER == XCP_ON_ETHERNET_DRIVER_WIFI

static ArduinoNetworkIf* s_net       = nullptr;
static ClientWrapper*    s_client    = nullptr;
static bool              s_connected = false;

static volatile bool s_timeout_triggered = false;

static void XcpTl_OnTimeout(void) {
    s_timeout_triggered = true;
}

class FrameParser {
   public:

    static int readFrame(ClientWrapper* client, uint8_t* buffer, size_t bufSize, unsigned long timeoutMs) {
        unsigned long start = millis();

        uint8_t lenBuf[2];

        if (!readN(client, lenBuf, 2, start, timeoutMs)) {
            return -1;
        }
        uint16_t len = (uint16_t(lenBuf[0]) << 8) | uint16_t(lenBuf[1]);
        if (len == 0 || len > bufSize) {
            return -1;
        }
        if (!readN(client, buffer, len, start, timeoutMs)) {
            return -1;
        }
        return int(len);
    }

    static bool readN(ClientWrapper* client, uint8_t* out, size_t n, unsigned long startTime, unsigned long timeoutMs) {
        size_t got = 0;
        while (got < n) {
            if (!client->connected() && client->available() == 0) {
                return false;
            }
            int av = client->available();
            if (av > 0) {
                int toRead = min((size_t)av, n - got);
                int r      = client->read_bytes(out + got, toRead);
                if (r <= 0) {
                    return false;
                }
                got += r;
            } else {
                if (millis() - startTime > timeoutMs) {
                    return false;
                }
            }
        }
        return true;
    }
};

#if XCP_ON_ETHERNET_ARDUINO_DRIVER == XCP_ON_ETHERNET_DRIVER_WIFI
/* TCP client wrapper and adapter for WiFi (ESP32) */
class WiFiClientWrapper : public ClientWrapper {
   public:

    explicit WiFiClientWrapper(WiFiClient c = WiFiClient()) : m_client(c) {
    }

    void set(WiFiClient c) {
        m_client = c;
    }

    int available() override {
        return m_client ? m_client.available() : 0;
    }

    int read_bytes(uint8_t* buf, size_t len) override {
        return m_client.read(buf, len);
    }

    size_t write_bytes(const uint8_t* buf, size_t len) override {
        return m_client.write(buf, len);
    }

    bool connected() override {
        return m_client && m_client.connected();
    }

    void stop() override {
        if (m_client) {
            m_client.stop();
        }
    }

    bool is_datagram() const override {
        return false;
    }

   private:

    WiFiClient m_client;
};

class WiFiTcpAdapter : public ArduinoNetworkIf {
   public:

    WiFiTcpAdapter(const char* s, const char* p, uint16_t port) : m_port(port), m_ssid(s), m_pass(p), m_server(port) {
    }

    bool begin() override {
        // Station mode tweaks
        WiFi.persistent(false);
        WiFi.mode(WIFI_STA);
        WiFi.setSleep(false);
    #ifdef XCP_ON_WIFI_TX_POWER
        WiFi.setTxPower(XCP_ON_WIFI_TX_POWER);
    #endif
        Serial.begin(115200);
        while (!Serial) {
            ;
        }
        Serial.println("\nBlueparrot XCP (TCP) on WiFi");

        // Optional static IP configuration
    #if defined(XCP_ON_ETHERNET_IP_OCTETS)
        IPAddress ip(XCP_ON_ETHERNET_IP_OCTETS);
        if (!WiFi.config(ip)) {
            Serial.println("[WiFi] Failed to apply static IP config");
        }
    #elif defined(XCP_ON_ETHERNET_IP)
        {
            IPAddress ip;
            ip.fromString(XCP_ON_ETHERNET_IP);
            if (!WiFi.config(ip)) {
                Serial.println("[WiFi] Failed to apply static IP config (string)");
            }
        }
    #endif
        WiFi.begin(m_ssid, m_pass);
        unsigned long start = millis();
        while (WiFi.status() != WL_CONNECTED && (millis() - start) < 15000UL) {
            delay(100);
            Serial.print('.');
        }
        Serial.println();
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("[WiFi] Connect failed");
            return false;
        }
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
        m_server.begin();
        m_server.setNoDelay(true);
        return true;
    }

    ClientWrapper* accept_client() override {
        // Accept new clients if none or the existing is disconnected
        if (!m_client.connected()) {
            WiFiClient incoming = m_server.available();
            if (incoming) {
                // If an old client existed, drop it
                if (m_client) {
                    m_client.stop();
                }
                m_client = incoming;
                m_client.setNoDelay(true);
                m_wrapper.set(m_client);
                return &m_wrapper;  // Return immediately; Rx handler will poll available()
            }
            return nullptr;
        }

        // Existing client remains current; ensure wrapper wraps it
        m_wrapper.set(m_client);
        return &m_wrapper;
    }

    String getInfo() override {
        IPAddress ip = WiFi.localIP();
        return String("WiFi-TCP ") + String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]);
    }

   private:

    uint16_t          m_port;
    const char*       m_ssid;
    const char*       m_pass;
    WiFiServer        m_server;
    WiFiClient        m_client;
    WiFiClientWrapper m_wrapper;
};
#endif // XCP_ON_ETHERNET_ARDUINO_DRIVER == XCP_ON_ETHERNET_DRIVER_WIFI

static bool ar_read_n(uint8_t* out, size_t n, unsigned long timeout_ms) {
    if (!s_client)
        return false;
    unsigned long start = millis();
    size_t        got   = 0;
    while (got < n) {
        if (s_timeout_triggered) {
            return false;
        }
        if (!s_client->connected() && s_client->available() == 0) {
            return false;
        }
        int av = s_client->available();
        if (av > 0) {
            int toRead = (int)min((size_t)av, n - got);
            int r      = s_client->read_bytes(out + got, (size_t)toRead);
            if (r <= 0)
                return false;
            got += (size_t)r;
            XcpTl_TimeoutReset();
        } else {
            XcpTl_TimeoutCheck();
            if (millis() - start > timeout_ms) {
                s_timeout_triggered = true;
                return false;
            }
            delay(1);
        }
    }
    return true;
}

static int ar_read_header(uint16_t* len, uint16_t* counter) {
    s_timeout_triggered = false;
    XcpTl_TimeoutStart();
    uint8_t hdr[4];
    if (!ar_read_n(hdr, 4, READ_TIMEOUT_MS)) {
        return 0;
    }

    *len     = (uint16_t)hdr[0] | ((uint16_t)hdr[1] << 8);
    *counter = (uint16_t)hdr[2] | ((uint16_t)hdr[3] << 8);
    return 1;
}

static int ar_read_data(uint8_t* data, uint16_t len) {
    return ar_read_n(data, len, READ_TIMEOUT_MS) ? 1 : 0;
}

extern "C" {

    void XcpTl_Init(void) {
        XcpTl_TimeoutInit((uint16_t)READ_TIMEOUT_MS, XcpTl_OnTimeout);
        s_timeout_triggered = false;
        /* Initialize backend network adapter and start listening. */
    #if XCP_ON_ETHERNET_ARDUINO_DRIVER == XCP_ON_ETHERNET_DRIVER_WIFI
        #if (XCP_ON_ETHERNET_PROTOCOL == XCP_ON_ETHERNET_USE_TCP)
        s_net = new WiFiTcpAdapter(XCP_ON_ETHERNET_WIFI_SSID, XCP_ON_ETHERNET_WIFI_PASSWORD, XCP_ON_ETHERNET_PORT);
        #else
        s_net = new WiFiAdapter(XCP_ON_ETHERNET_WIFI_SSID, XCP_ON_ETHERNET_WIFI_PASSWORD, XCP_ON_ETHERNET_PORT);
        #endif
    #else
        #if defined(XCP_ON_ETHERNET_IP_OCTETS)
        s_net = new EthernetAdapter(IPAddress(XCP_ON_ETHERNET_IP_OCTETS), XCP_ON_ETHERNET_PORT);
        #elif defined(XCP_ON_ETHERNET_IP)
        {
            IPAddress ip;
            ip.fromString(XCP_ON_ETHERNET_IP);
            s_net = new EthernetAdapter(ip, XCP_ON_ETHERNET_PORT);
        }
        #else
        s_net = new EthernetAdapter(XCP_ON_ETHERNET_PORT);
        #endif
    #endif
        if (s_net) {
            (void)s_net->begin();
        }
        s_connected = false;
    }

    void XcpTl_DeInit(void) {
        if (s_client) {
            s_client->stop();
            s_client = nullptr;
        }
        if (s_net) {
            delete s_net;
            s_net = nullptr;
        }
        s_connected = false;
    }

    static void XcpTl_Accept(void) {
        if (!s_net) {
            return;
        }
        s_net->process();
        ClientWrapper* c = s_net->accept_client();
        if (c) {
            s_client = c;
            if (!s_connected) {
                s_connected = true;
                XcpTl_SaveConnection();
            }
        }
    }

    void XcpTl_MainFunction(void) {
        XcpTl_Accept();
        XcpTl_TimeoutCheck();
        XcpTl_RxHandler();
    }

    extern Xcp_PduType Xcp_CtoIn;
    extern void        Xcp_DispatchCommand(Xcp_PduType const * pdu);

    void XcpTl_RxHandler(void) {
        if (!s_connected || !s_client) {
            return;
        }

        uint16_t dlc     = 0U;
        uint16_t counter = 0U;

        /* Non-blocking: only try if at least header is available. */
        if (s_client->available() < (int)XCP_ETH_HEADER_SIZE) {
            return;
        }

        int hres = ar_read_header(&dlc, &counter);
        if (hres <= 0) {
            if (s_timeout_triggered) {
                s_timeout_triggered = false;
                XcpTl_TimeoutStop();
                Serial.println("Reception timeout (header) — aborting current frame");
                return;
            }
            /* Connection lost or other error */
            if (s_client && s_client->is_datagram()) {
                /* For UDP, keep last endpoint; just stop TL timeout and abort this frame */
                XcpTl_TimeoutStop();
                return;
            }
            XcpTl_ReleaseConnection();
            s_client->stop();
            s_client    = nullptr;
            s_connected = false;
            XcpTl_TimeoutStop();
            return;
        }

        if ((dlc > XCP_TRANSPORT_LAYER_CTO_BUFFER_SIZE) || (dlc > (uint16_t)XCP_COMM_BUFLEN)) {
            // XcpHw_ErrorMsg((char*)"XcpTl_RxHandler: DLC too large", EINVAL);
            if (s_client && s_client->is_datagram()) {
                /* For UDP, keep endpoint but drop this malformed frame */
                XcpTl_TimeoutStop();
                return;
            }
            XcpTl_ReleaseConnection();
            s_client->stop();
            s_client    = nullptr;
            s_connected = false;
            return;
        }

        Xcp_CtoIn.len = dlc;
        int dres      = ar_read_data(Xcp_CtoIn.data, dlc);
        if (dres <= 0) {
            if (s_timeout_triggered) {
                s_timeout_triggered = false;
                XcpTl_TimeoutStop();
                Serial.println("Reception timeout (data) — aborting current frame");
                return;
            }
            if (s_client && s_client->is_datagram()) {
                /* For UDP, keep last endpoint on short read or error; abort this frame only */
                XcpTl_TimeoutStop();
                return;
            }
            XcpTl_ReleaseConnection();
            s_client->stop();
            s_client    = nullptr;
            s_connected = false;
            XcpTl_TimeoutStop();
            return;
        }
        /* Dispatch the freshly received CTO command. */
        Xcp_DispatchCommand(&Xcp_CtoIn);
        XcpTl_TimeoutStop();
    }

    void XcpTl_Send(uint8_t const * buf, uint16_t len) {
        XCP_TL_ENTER_CRITICAL();
        if (s_connected && s_client) {
    #ifdef XCP_UDP_DEBUG
            Serial.print("XCP_TL_SEND len=");
            Serial.print(len);
            Serial.print(" conn=");
            Serial.print(s_connected ? 1 : 0);
            Serial.print(" datagram=");
            Serial.println(s_client->is_datagram() ? 1 : 0);
    #endif
            /* Data already contains ETH header (LEN/CTR) + payload. */
            (void)s_client->write_bytes(buf, len);
        }
        XCP_TL_LEAVE_CRITICAL();
    }

    void XcpTl_SaveConnection(void) {
        /* Not using address structs on Arduino; mark connected state. */
        s_connected = true;
    }

    void XcpTl_ReleaseConnection(void) {
        s_connected = false;
    }

    bool XcpTl_VerifyConnection(void) {
        /* Single client only; verify simply returns connection state. */
        return s_connected;
    }

}  // extern "C"

#endif /* (XCP_TRANSPORT_LAYER == XCP_ON_ETHERNET) && (defined(ARDUINO)) */

////////////////////////////////////////////////////////////////////////////////
//                             arduino_serial.cpp                             //
////////////////////////////////////////////////////////////////////////////////

/*
 * BlueParrot XCP
 *
 * (C) 2007-2025 by Christoph Schueler <github.com/Christoph2,
 *                                      cpu12.gems@googlemail.com>
 *
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * s. FLOSS-EXCEPTION.txt
 */


#if (XCP_TRANSPORT_LAYER == XCP_ON_SXI) && (defined(ARDUINO))

    #include "Arduino.h"

extern "C" {

    #if XCP_ON_SXI_CUSTOM_INTERFACE == XCP_OFF

    void Serial_Init(void) {
        #if ((defined(CONFIG_IDF_TARGET_ESP32C3)) || (defined(CONFIG_IDF_TARGET_ESP32S3)))
        // CDC only.
        (XCP_ON_SXI_PORT_NAME).begin(XCP_ON_SXI_BITRATE);
        #else
        (XCP_ON_SXI_PORT_NAME).begin(XCP_ON_SXI_BITRATE, XCP_ON_SXI_CONFIG);
        #endif
    }

    void Serial_DeInit(void) {
    }

    uint32_t Serial_Available(void) {
        return (XCP_ON_SXI_PORT_NAME).available();
    }

    bool Serial_Read(uint8_t *in_byte) {
        uint8_t octet;

        if ((XCP_ON_SXI_PORT_NAME).available() > 0) {
            octet    = (XCP_ON_SXI_PORT_NAME).read();
            *in_byte = octet;
            return true;
        }
        return false;
    }

    void Serial_WriteByte(uint8_t out_byte) {
        (XCP_ON_SXI_PORT_NAME).write(out_byte);
    }

    void Serial_WriteBuffer(uint8_t const *out_bytes, uint32_t size) {
        (XCP_ON_SXI_PORT_NAME).write(out_bytes, size);
    }

    #endif /* XCP_ON_SXI_CUSTOM_INTERFACE == XCP_OFF */

    void Serial_MainFunction(void) {
    }
}

#endif /* (XCP_TRANSPORT_LAYER == XCP_ON_SXI) && (defined(ARDUINO)) */

////////////////////////////////////////////////////////////////////////////////
//                                   hw.cpp                                   //
////////////////////////////////////////////////////////////////////////////////

/*
 * pySART - Simplified AUTOSAR-Toolkit for Python.
 *
 * (C) 2007-2025 by Christoph Schueler <github.com/Christoph2,
 *                                      cpu12.gems@googlemail.com>
 *
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * s. FLOSS-EXCEPTION.txt
 */

#include <Arduino.h>
#include <stdint.h>


typedef struct tagHwStateType {
    uint32_t StartingTime;
} HwStateType;

static HwStateType HwState = { 0 };

void XcpHw_Init(void) {
    HwState.StartingTime = millis();
}

void XcpHw_Deinit(void) {
}

uint32_t XcpHw_GetTimerCounter(void) {
#if XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_1US
    return micros();
#elif XCP_DAQ_TIMESTAMP_UNIT == XCP_DAQ_TIMESTAMP_UNIT_1MS
    return millis();
#else
    #error Timestamp-unit not supported.
#endif  // XCP_DAQ_TIMESTAMP_UNIT
}

uint32_t XcpHw_GetTimerCounterMS(void) {
    return millis();
}

bool XcpHw_SxIAvailable(void) {
    return Serial.available();
}

uint8_t XcpHw_SxIRead(void) {
    return (uint8_t)Serial.read();
}

void XcpHw_AcquireLock(uint8_t lockIdx) {
    if (lockIdx >= XCP_HW_LOCK_COUNT) {
        return;
    }
    noInterrupts();
}

void XcpHw_ReleaseLock(uint8_t lockIdx) {
    if (lockIdx >= XCP_HW_LOCK_COUNT) {
        return;
    }
    interrupts();
}

void XcpHw_Sleep(uint64_t usec) {
    delayMicroseconds(usec);
}
