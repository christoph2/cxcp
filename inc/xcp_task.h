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
    using Func = void (*)();

    Func m_func;

    Delegate(Func func = nullptr) : m_func(func) {
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
    using Func = uint32_t (*)();

    Func m_func;

    DelegateU32(Func func = nullptr) : m_func(func) {
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
 * Simple periodic DAQ task management for foreground/background systems like Arduino.
 */
class PeriodicTask {
   public:

    PeriodicTask(uint32_t interval, uint16_t event_num, Delegate task, DelegateU32 timestamp_function) :
        m_interval(interval),
        m_event_num(event_num),
        m_task(task),
        m_timestamp_function(timestamp_function),
        m_time_point(0UL),
        m_elapsed_time(0UL),
        m_start(0UL),
        m_stop(0UL) {
        m_time_point = m_timestamp_function();
    }

    void invoke() {
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

    uint32_t    m_interval;
    uint16_t    m_event_num;
    Delegate    m_task;
    DelegateU32 m_timestamp_function;
    uint32_t    m_time_point;
    uint32_t    m_elapsed_time;
    uint32_t    m_start;
    uint32_t    m_stop;
};

    #endif  // __cplusplus

#endif  // __XCP_ARDUINO_HPP
