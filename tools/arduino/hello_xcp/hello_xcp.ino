

#include <math.h>
// #include <queue>

#include "xcp.h"

#define EPK_SECTION_NAME "calflash_signature"
#define EPK_CONST_NAME   "EcuName V1.2.0 01.03.2012"

// GNU syntax.
/*__attribute__((section(EPK_SECTION_NAME)))*/ __attribute__((used)) volatile static const char epk[/*sizeof(EPK_CONST_NAME)*/]
    __attribute__((section(EPK_SECTION_NAME))) = EPK_CONST_NAME;

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

    // Measurements.
    __attribute__((section(".measurements"))) volatile uint16_t triangle_wave = 0U;
__attribute__((section(".measurements"))) volatile uint8_t sq0_wave           = 0U;
__attribute__((section(".measurements"))) volatile uint8_t sq1_wave           = 0U;
__attribute__((section(".measurements"))) volatile float   sine_wave          = 0U;

// Parameters
volatile uint16_t triangle_min = 0x00;
volatile uint16_t triangle_max = 0xff;

unsigned long startTime;
unsigned long elapsedTime;

class AbstractWave {
   public:

    explicit AbstractWave(uint16_t min = 0x00, uint16_t max = 0xff) : m_wave(0), m_min(min), m_max(max), m_observers(32) {
    }

    virtual ~AbstractWave() {
    }

    AbstractWave() = delete;

    virtual void step() noexcept {};

    uint16_t get_value() const noexcept {
        return m_wave;
    }

    void register_observer(AbstractWave *slave_wave) noexcept {
        m_observers.append(slave_wave);
    }

    void notify_observers() noexcept {
        m_observers.notify(m_wave);
#if 0
        for (auto element : m_observers) {
            element->update(m_wave);
        }
#endif
    }

    virtual void update(uint16_t value) noexcept {
    }

   protected:

    uint16_t                  m_wave;
    uint16_t                  m_min;
    uint16_t                  m_max;
    Observers<AbstractWave *> m_observers;
};

class TriangleWave : public AbstractWave {
   public:

    explicit TriangleWave(uint16_t min = 0x00, uint16_t max = 0xff) : AbstractWave(min, max), m_positive(true) {
    }

    void step() noexcept override {
        if (m_positive) {
            m_wave++;
            if (m_wave >= m_max) {
                m_positive = false;
                m_wave     = m_max;
            }
        } else {
            m_wave--;
            if (m_wave <= m_min) {
                m_positive = true;
                m_wave     = m_min;
            }
        }
        notify_observers();
    }

    bool m_positive;
};

class SquareWave : public AbstractWave {
   public:

    SquareWave(AbstractWave *wave, uint16_t level, uint16_t skew = 0U, bool raising_edge = true) :
        AbstractWave(0x00, 0xffff), m_level(level), m_skew(skew), m_raising_edge(raising_edge), m_queue(skew) {
        wave->register_observer(this);
        if (skew > 0U) {
            prefill_queue(skew, 0x0000u);
        }
    }

    void update(uint16_t value) noexcept override {
        if (m_skew > 0) {
            auto tmp_value = m_queue.pop();
            m_queue.push(value);
            value = tmp_value;
        }
        if (value >= m_level) {
            m_wave = m_raising_edge ? m_max : m_min;
        } else {
            m_wave = m_raising_edge ? m_min : m_max;
        }
    }

   private:

    void prefill_queue(uint16_t size, uint16_t value) {
        for (auto i = 0; i < size; ++i) {
            m_queue.push(value);
        }
    }

    uint16_t        m_level;
    uint16_t        m_skew;
    bool            m_raising_edge;
    Queue<uint16_t> m_queue;
};

class SineWave : public AbstractWave {
   public:

    SineWave(AbstractWave *wave, uint16_t steps, float amplitude, bool symetrical = true) :
        AbstractWave(0x00, 0xffff), m_steps(steps), m_amplitude(amplitude), m_symetrical(symetrical), m_angle(0.0f), m_value(0.0f) {
        wave->register_observer(this);
    }

    float get_float_value() const noexcept {
        return m_value;
    }

    void update(uint16_t value) noexcept override {
        m_value = m_amplitude * sin(m_angle) /* + (static_cast<float>(m_amplitude) / 2.0f)*/;
        m_angle += (2.0f * M_PI) / static_cast<float>(m_steps);
    }

   private:

    uint16_t m_steps;
    float    m_amplitude;
    bool     m_symetrical;
    float    m_angle;
    float    m_value;
};

void setup() {
    Xcp_Init();

    startTime = millis();
}

#if 0
auto triangle = TriangleWave();
auto square0  = SquareWave(&triangle, 64);
auto square1  = SquareWave(&triangle, 64, 96, false);
auto sin0     = SineWave(&triangle, 0xff, 100.0, true);
#endif

void loop() {
    XcpTl_MainFunction();
    Xcp_MainFunction();
    // #if 0
    if ((millis() - startTime) >= 10) {
        startTime = millis();
        triangle.step();
        triangle_wave = triangle.get_value();
        sq0_wave      = static_cast<uint8_t>(square0.get_value());
        sq1_wave      = static_cast<uint8_t>(square1.get_value());
        sine_wave     = sin0.get_float_value();
        XcpDaq_TriggerEvent(0);
    }
    // #endif
}
