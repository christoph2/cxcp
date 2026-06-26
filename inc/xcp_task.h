/*
 * BlueParrot XCP
 *
 * (C) 2026 by Christoph Schueler <github.com/Christoph2,
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

/*!!! START-INCLUDE-SECTION !!!*/
#include "xcp.h"
/*!!! END-INCLUDE-SECTION !!!*/

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
    m_func (func){
    }

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
    m_func (func){
    }

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
 * Type-safe duration combining a numeric value with an XCP DAQ timestamp unit.
 *
 * Uses XCP_DAQ_TIMESTAMP_UNIT_* constants to make the time unit explicit.
 */
struct Duration {
    uint32_t value;
    uint8_t unit;

    Duration (uint32_t val
    ,
    uint8_t u
    )
    :
    value (val), unit(u) {
    }

    static Duration picoseconds(uint32_t val) {
        return Duration(val, XCP_DAQ_TIMESTAMP_UNIT_1PS);
    }

    static Duration nanoseconds(uint32_t val) {
        return Duration(val, XCP_DAQ_TIMESTAMP_UNIT_1NS);
    }

    static Duration microseconds(uint32_t val) {
        return Duration(val, XCP_DAQ_TIMESTAMP_UNIT_1US);
    }

    static Duration milliseconds(uint32_t val) {
        return Duration(val, XCP_DAQ_TIMESTAMP_UNIT_1MS);
    }

    static Duration seconds(uint32_t val) {
        return Duration(val, XCP_DAQ_TIMESTAMP_UNIT_1S);
    }
};

/**
 * Simple periodic DAQ task management for foreground/background systems like Arduino.
 */
class PeriodicTask {
public:

    PeriodicTask(Duration interval, uint16_t event_num, Delegate task, DelegateU32 timestamp_function)
    :
    m_interval(interval),
            m_event_num(event_num),
            m_task(task),
            m_timestamp_function(timestamp_function),
            m_time_point(0UL),
            m_elapsed_time(0UL),
            m_start(0UL),
            m_stop(0UL)
    {
        m_time_point = m_timestamp_function();
    }

    void invoke() {
        if ((m_timestamp_function() - m_time_point) >= m_interval.value) {
            m_time_point = m_timestamp_function();
            m_start = m_timestamp_function();

            m_task();

            XcpDaq_TriggerEvent(m_event_num);

            m_stop = m_timestamp_function();
            m_elapsed_time = m_stop - m_start;
            m_time_point -= m_elapsed_time;
        }
    }

    uint8_t getTimeUnit() const {
        return m_interval.unit;
    }

private:

    Duration m_interval;
    uint16_t m_event_num;
    Delegate m_task;
    DelegateU32 m_timestamp_function;
    uint32_t m_time_point;
    uint32_t m_elapsed_time;
    uint32_t m_start;
    uint32_t m_stop;
};

#endif  // __cplusplus

#endif  // __XCP_ARDUINO_HPP
