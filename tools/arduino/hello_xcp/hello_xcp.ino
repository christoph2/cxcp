

#include <math.h>
//#include <queue>

#include "xcp.h"

#define EPK_SECTION_NAME ".calflash_signature"
#define EPK_CONST_NAME "EcuName V1.2.0 01.03.2012"

// GNU syntax.
//__attribute__((section(EPK_SECTION_NAME))) __attribute__((used)) volatile static const char epk[/*sizeof(EPK_CONST_NAME)*/]
//  __attribute__((section(EPK_SECTION_NAME))) = EPK_CONST_NAME;

XCP_DAQ_BEGIN_EVENTS
XCP_DAQ_DEFINE_EVENT(
  "EVT 10ms", XCP_DAQ_EVENT_CHANNEL_TYPE_DAQ | XCP_DAQ_CONSISTENCY_DAQ_LIST, XCP_DAQ_EVENT_CHANNEL_TIME_UNIT_1MS, 10),
  XCP_DAQ_DEFINE_EVENT(
    "EVT sporadic", XCP_DAQ_EVENT_CHANNEL_TYPE_DAQ | XCP_DAQ_CONSISTENCY_DAQ_LIST, XCP_DAQ_EVENT_CHANNEL_TIME_UNIT_1MS, 0),
  XCP_DAQ_END_EVENTS

  XCP_DAQ_BEGIN_ID_LIST 0x201,
  0x202, 0x203, 0x204, 0x205, 0x206,
  XCP_DAQ_END_ID_LIST

// Measurements.
__attribute__((section(".measurements"))) volatile uint16_t triangle_wave = 0U;
__attribute__((section(".measurements"))) volatile uint8_t sq0_wave           = 0U;
__attribute__((section(".measurements"))) volatile uint8_t sq1_wave           = 0U;
__attribute__((section(".measurements"))) volatile float   sine_wave          = 0U;


  __attribute__((section(".measurements"))) volatile float values[8];

// Parameters
volatile uint16_t triangle_min = 0x00;
volatile uint16_t triangle_max = 0xff;

unsigned long startTime;
unsigned long elapsedTime;

const int ANALOG_INS[] = { A0, A1, A2, A3 };

void setup() {

  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  
  Xcp_Init();

  startTime = millis();
}

void loop() {
  XcpTl_MainFunction();
  Xcp_MainFunction();

uint8_t idx = 0;

  for (const auto port : ANALOG_INS) {
    int sensorValue = analogRead(port);

    // values[idx++] = (sensorValue * (4.3 / 1023.0))*0.969543; //0.969543

    values[idx++] = (500 * sensorValue) / 1024;
  
    // float batteryLevel = analogRead(ADC_BATTERY) * 3.3f / 1023.0f / 1.2f * (1.2f+0.33f);
  }
  //#if 0
  if ((millis() - startTime) >= 5) {
    startTime = millis();
#if 0
        triangle.step();
        triangle_wave = triangle.get_value();
        sq0_wave      = static_cast<uint8_t>(square0.get_value());
        sq1_wave      = static_cast<uint8_t>(square1.get_value());
        sine_wave     = sin0.get_float_value();
#endif
    XcpDaq_TriggerEvent(0);
  }
  //#endif
}
