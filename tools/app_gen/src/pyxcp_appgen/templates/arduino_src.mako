

#include <math.h>
#include <stdint.h>

#include "xcp.h"

#define EPK_SECTION_NAME ".calflash_signature"
#define EPK_CONST_NAME   "${project['name']} V1.2.0 01.03.2012"

// GNU syntax.
//__attribute__((section(EPK_SECTION_NAME))) __attribute__((used)) volatile static const char epk[/*sizeof(EPK_CONST_NAME)*/]
//  __attribute__((section(EPK_SECTION_NAME))) = EPK_CONST_NAME;

#define BUILD_DATE     __DATE__
#define BUILD_TIME     __TIME__

#define XCP_MAKE_EPK(name) \
    const char Xcp_EEPROM_Kennung[] = name " " BUILD_DATE " " BUILD_TIME

XCP_MAKE_EPK("${project['name']} V1.1.0");

XCP_DAQ_BEGIN_EVENTS
%for event in project["module"].get("events", []):
XCP_DAQ_DEFINE_EVENT(${'"' + event["name"] + '"'}, XCP_DAQ_EVENT_CHANNEL_TYPE_DAQ | XCP_DAQ_CONSISTENCY_DAQ_LIST, XCP_DAQ_EVENT_CHANNEL_TIME_UNIT_1MS, ${event["interval"]}),
%endfor
XCP_DAQ_END_EVENTS
##    XCP_DAQ_BEGIN_ID_LIST 0x201,
##    0x202, 0x203, 0x204, 0x205, 0x206,
##    XCP_DAQ_END_ID_LIST

##    __attribute__((section(".measurements"))) volatile float values[8];
%for meas in project["module"].get("measurements", []):
volatile ${meas["type"]} ${meas["name"]}; // ${meas["comment"]}
%endfor

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

auto wg1 = WaveformGenerator(SINE, 16, 100, amplitiude_value(AMPLITUDE_SCALE));
auto wg2 = WaveformGenerator(SINE, 16, 100, amplitiude_value(AMPLITUDE_SCALE, -3.0F), 90.0);
auto wg3 = WaveformGenerator(TRIANGLE, 10, 100, amplitiude_value(AMPLITUDE_SCALE, -6.0F));
auto wg4 = WaveformGenerator(SAWTOOTH, 10, 100, amplitiude_value(AMPLITUDE_SCALE, -6.0F), -90.0);

void setup() {
    // Initialize measurement variables.
%for meas in project["module"].get("measurements", []):
    ${meas["name"]} = ${get_initializer(meas)};
%endfor

    Xcp_Init();
}

void update_measurements() {
    voltage1 = wg1.nextSample();
    voltage2 = wg2.nextSample();
    voltage3 = wg3.nextSample();
    // voltage4 = wg4.nextSample();
    voltage4 += 0.05f;
}


auto daq_task = PeriodicTask(10, 0, Delegate::create(update_measurements), DelegateU32::create(millis));

void loop() {

    XcpTl_MainFunction();
    Xcp_MainFunction();

    daq_task.invoke();
}
