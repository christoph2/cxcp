

#include <math.h>
// #include <stdint.h>

#include "xcp.h"

#define EPK_SECTION_NAME ".calflash_signature"
#define EPK_CONST_NAME   "EcuName V1.2.0 01.03.2012"

// GNU syntax.
//__attribute__((section(EPK_SECTION_NAME))) __attribute__((used)) volatile static const char epk[/*sizeof(EPK_CONST_NAME)*/]
//  __attribute__((section(EPK_SECTION_NAME))) = EPK_CONST_NAME;

XCP_DAQ_BEGIN_EVENTS
XCP_DAQ_DEFINE_EVENT(
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

// Parameters
volatile uint16_t triangle_min = 0x00;
volatile uint16_t triangle_max = 0xff;

unsigned long time_point;
unsigned long elapsed_time;

// volatile uint8_t buffer[256];

volatile uint32_t voltage1;
volatile uint32_t voltage2;
volatile uint32_t voltage3;
volatile uint32_t voltage4;
// Measurements.
volatile uint16_t triangle_wave;
volatile uint8_t  sq0_wave;
volatile uint8_t  sq1_wave;

auto tri0 = TriangleWave(0, 0xff, 1);
auto tri1 = TriangleWave(0, 0xff, 2);
auto tri2 = TriangleWave(0, 0xff, 4);
auto tri3 = TriangleWave(0, 0xff, 8);

void setup() {
#if 0
    voltage1 = 0xAAAAAAAA;
    voltage2 = 0xBBBBBBBB;
    voltage3 = 0xCCCCCCCC;
    voltage4 = 0xDDDDDDDD;
#endif
    triangle_wave = 0xee;
    sq0_wave      = 0xaa;
    sq1_wave      = 0x99;

    time_point = micros();
    Xcp_Init();
}

void loop() {
    unsigned long start = 0;
    unsigned long stop  = 0;

    XcpTl_MainFunction();
    Xcp_MainFunction();
    // #if 0
    if ((micros() - time_point) >= 10 * 1000) {
        time_point = micros();
        start      = micros();
        tri0.step();
        tri1.step();
        tri2.step();
        tri3.step();

        voltage1 = static_cast<uint32_t>(tri0.get_value());
        voltage2 = static_cast<uint32_t>(tri1.get_value());
        voltage3 = static_cast<uint32_t>(tri2.get_value());
        voltage4 = static_cast<uint32_t>(tri3.get_value());
        // triangle.step();
        // triangle_wave = triangle.get_value();
        // sq0_wave      = static_cast<uint8_t>(square0.get_value());
        // sq1_wave      = static_cast<uint8_t>(square1.get_value());
        // sine_wave     = sin0.get_float_value();
        XcpDaq_TriggerEvent(0);
        stop         = micros();
        elapsed_time = stop - start;
        time_point -= elapsed_time;
    }
    // #endif
}
