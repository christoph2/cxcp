//
// Created by HP on 23.10.2025.
//

#include <time.h>

#include "hardware/timer.h"
#include "xcp.h"

#define BUILD_DATE __DATE__
#define BUILD_TIME __TIME__

#define XCP_MAKE_EPK(name) const char Xcp_EEPROM_Kennung[] = name " " BUILD_DATE " " BUILD_TIME

XCP_MAKE_EPK("ArduinoXCP V1.1.0");

// Measurements.
volatile uint8_t dummy;
volatile float   voltage1;
volatile float   voltage2;
volatile float   voltage3;
volatile float   voltage4;

/////
///// BEGIN / Predefined DAQ-Lists
/////

XCP_DAQ_PREDEFINED_MEASUREMENT_VARIABLES_BEGIN
XCP_DAQ_DEFINE_MEASUREMENT_VARIABLE(dummy), XCP_DAQ_DEFINE_MEASUREMENT_VARIABLE(voltage1),
    XCP_DAQ_DEFINE_MEASUREMENT_VARIABLE(voltage2), XCP_DAQ_DEFINE_MEASUREMENT_VARIABLE(voltage3),
    XCP_DAQ_DEFINE_MEASUREMENT_VARIABLE(voltage4),
    XCP_DAQ_PREDEFINED_MEASUREMENT_VARIABLES_END

    XCP_DAQ_PREDEFINED_ODT_ENTRIES_BEGIN XCP_DAQ_DEFINE_ODT_VARIABLE_IDX(0),
    XCP_DAQ_DEFINE_ODT_VARIABLE_IDX(1), XCP_DAQ_DEFINE_ODT_VARIABLE_IDX(2), XCP_DAQ_DEFINE_ODT_VARIABLE_IDX(3),
    XCP_DAQ_DEFINE_ODT_VARIABLE_IDX(4),
    XCP_DAQ_PREDEFINED_ODT_ENTRIES_END

    XCP_DAQ_PREDEFINED_ODTS_BEGIN{ .numOdtEntries = 1, .firstOdtEntry = 0 },
    { .numOdtEntries = 1, .firstOdtEntry = 1 }, { .numOdtEntries = 1, .firstOdtEntry = 2 },
    { .numOdtEntries = 1, .firstOdtEntry = 3 }, { .numOdtEntries = 1, .firstOdtEntry = 4 },
    XCP_DAQ_PREDEFINED_ODTS_END

    XCP_DAQ_PREDEFINED_LISTS_BEGIN{ .numOdts = 5, .firstOdt = 0 } XCP_DAQ_PREDEFINED_LISTS_END

    XCP_DAQ_BEGIN_EVENTS XCP_DAQ_DEFINE_EVENT(
        "EVT 10ms", XCP_DAQ_EVENT_CHANNEL_TYPE_DAQ | XCP_DAQ_CONSISTENCY_DAQ_LIST, XCP_DAQ_EVENT_CHANNEL_TIME_UNIT_1MS, 10
    ),
    XCP_DAQ_DEFINE_EVENT(
        "EVT sporadic", XCP_DAQ_EVENT_CHANNEL_TYPE_DAQ | XCP_DAQ_CONSISTENCY_DAQ_LIST, XCP_DAQ_EVENT_CHANNEL_TIME_UNIT_1MS, 0
    ),
    XCP_DAQ_END_EVENTS

    XCP_DAQ_BEGIN_ID_LIST 0x201,
    0x202, 0x203, 0x204, 0x205, 0x206,
    XCP_DAQ_END_ID_LIST

    __attribute__((section(".measurements"))) volatile float values[8];

/////
///// END / Predefined DAQ-Lists
/////
/////

#if defined(RASPBERRYPI_PICO)

#endif

int main() {
    unsigned long start        = 0;
    unsigned long stop         = 0;
    unsigned long time_point   = 0;
    unsigned long elapsed_time = 0;

    voltage1 = 10.0f;
    voltage2 = 20.0f;
    voltage3 = 30.0f;
    voltage4 = 40.0f;

    Xcp_Init();

    // timer_busy_wait_until()
    time_point = time_us_32();
    while (1) {
        XcpTl_MainFunction();
        Xcp_MainFunction();

        if ((time_us_32() - time_point) >= 10 * 1000) {
            time_point = time_us_32();
            start      = time_us_32();
#if 0
            voltage1 = wg1.nextSample();
            voltage2 = wg2.nextSample();
            voltage3 = wg3.nextSample();
            voltage4 = wg4.nextSample();
#endif
            XcpDaq_TriggerEvent(0);

            stop         = time_us_32();
            elapsed_time = stop - start;
            time_point -= elapsed_time;
        }
    }

    return 0;
}
