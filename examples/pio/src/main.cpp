#include <Arduino.h>

#include "xcp.h"

#define BUILD_DATE __DATE__
#define BUILD_TIME __TIME__

extern "C" {
#define XCP_MAKE_EPK(name) const char Xcp_EEPROM_Kennung[] = name " " BUILD_DATE " " BUILD_TIME

    XCP_MAKE_EPK("ArduinoXCP V1.1.0");
}

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

    XCP_DAQ_PREDEFINED_ODTS_BEGIN{ .numOdtEntries = 1, .firstOdtEntry = 0 }

,
    { .numOdtEntries = 1, .firstOdtEntry = 1 }, { .numOdtEntries = 1, .firstOdtEntry = 2 },
    { .numOdtEntries = 1, .firstOdtEntry = 3 }, { .numOdtEntries = 1, .firstOdtEntry = 4 },
    XCP_DAQ_PREDEFINED_ODTS_END

    XCP_DAQ_PREDEFINED_LISTS_BEGIN{ .numOdts = 5, .firstOdt = 0 } XCP_DAQ_PREDEFINED_LISTS_END
    /////
    ///// END / Predefined DAQ-Lists
    /////

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

unsigned long time_point;
unsigned long elapsed_time;

enum WaveformType {
    SINE,
    TRIANGLE,
    SQUARE,
    SAWTOOTH
};

inline float amplitiude_value(float base = 1.0F, float amplification = 0.0F) {
    return base * (pow(10.0F, amplification / 20.0F));
}

class WaveformGenerator {
   private:

    float        phase           = 0.0F;
    float        phase_increment = 0.0F;
    float        amplitude       = 1.0F;
    WaveformType type            = SINE;

   public:

    WaveformGenerator(WaveformType t, float freq, float sample_rate, float amp = 1.0F, float p = 0.0F) :
        type(t), phase_increment(freq / sample_rate), amplitude(amp), phase(p) {
    }

    void setFrequency(float freq, float sample_rate) {
        phase_increment = freq / sample_rate;
    }

    void setAmplitude(float amp) {
        amplitude = amp;
    }

    void setWaveform(WaveformType t) {
        type = t;
    }

    float nextSample() {
        phase += phase_increment;
        if (phase >= 1.0F)
            phase -= 1.0F;

        float value = 0.0F;
        switch (type) {
            case SINE:
                value = ::sin(2.0F * PI * phase);
                break;
            case TRIANGLE:
                value = 2.0F * abs(2.0F * (phase - floor(phase + 0.5F))) - 1.0F;
                break;
            case SQUARE:
                value = (phase < 0.5F) ? 1.0F : -1.0F;
                break;
            case SAWTOOTH:
                value = 2.0F * (phase - 0.5F);
                break;
        }

        return amplitude * value;
    }
};

const uint32_t AMPLITUDE_SCALE = 32.0F;

auto wg1 = WaveformGenerator(SINE, 1.0f, 100.0f, amplitiude_value(AMPLITUDE_SCALE));
auto wg2 = WaveformGenerator(SINE, 1.0f, 100.0f, amplitiude_value(AMPLITUDE_SCALE, -3.0F), 90.0);
auto wg3 = WaveformGenerator(TRIANGLE, 0.5f, 100.0f, amplitiude_value(AMPLITUDE_SCALE, -6.0F));
auto wg4 = WaveformGenerator(SAWTOOTH, 0.5f, 100.0f, amplitiude_value(AMPLITUDE_SCALE, -6.0F), -90.0);

void setup() {
    dummy    = 0x55;
    voltage1 = 90.0f;
    voltage2 = 180.0f;
    voltage3 = 270.0f;
    voltage4 = 360.0f;

    time_point = micros();
    Xcp_Init();
}

void loop() {
    unsigned long start = 0;
    unsigned long stop  = 0;

    XcpTl_MainFunction();
    Xcp_MainFunction();

    if ((micros() - time_point) >= 10 * 1000) {
        time_point = micros();
        start      = micros();

        voltage1 = wg1.nextSample();
        voltage2 = wg2.nextSample();
        voltage3 = wg3.nextSample();
        voltage4 = wg4.nextSample();

        XcpDaq_TriggerEvent(0);
        dummy++;

        stop         = micros();
        elapsed_time = stop - start;
        time_point -= elapsed_time;
    }
}
