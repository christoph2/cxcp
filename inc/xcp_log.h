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
