

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

class AbstractWave {
   public:

    explicit AbstractWave(uint16_t min = 0x00, uint16_t max = 0xff, uint8_t increment = 1) :
        m_counter(0), m_min(min), m_max(max), m_increment(increment) {
    }

    virtual ~AbstractWave() {
    }

    AbstractWave() = delete;

    virtual void step() noexcept {};

    uint16_t get_value() const noexcept {
        return m_counter;
    }

    virtual void update(uint16_t value) noexcept {
    }

   protected:

    uint16_t m_counter;
    uint16_t m_min;
    uint16_t m_max;
    uint8_t  m_increment;
};

class TriangleWave : public AbstractWave {
   public:

    explicit TriangleWave(uint16_t min = 0x00, uint16_t max = 0xff, uint8_t increment = 1) :
        AbstractWave(min, max, increment), m_positive(true) {
    }

    void step() noexcept override {
        if (m_positive) {
            m_counter += m_increment;
            if (m_counter >= m_max) {
                m_positive = false;
                m_counter  = m_max;
            }
        } else {
            m_counter -= m_increment;
            if (m_counter <= m_min) {
                m_positive = true;
                m_counter  = m_min;
            }
        }
        // notify_observers();
    }

    bool m_positive;
};

////////////////////////////////

// Subject class.
template<typename Ty, size_t C>
class Subject {
   public:

    void         Attach(const Ty& observer);
    void         Detach(const Ty& observer);
    void         Notify();
    virtual void Update(int value) = 0;

   protected:

    Ty observers[C];
};

#if 0
// Concrete Subject class.
class ConcreteSubject : public Subject {
public:
    void Update(int value) override {
        this->value = value;
        Notify();
    }

    int GetValue() const {
        return value;
    }

private:
    int value;
};


// Observer class.
class Observer {
public:
    virtual void OnUpdate(int value) = 0;

protected:
    // Subject<AbstractWave, 4> subject;
};

// Concrete Observer class.
class ConcreteObserver : public Observer {
public:
    ConcreteObserver(Subject* subject) : subject(subject) {
        subject->Attach(this);
    }

    void OnUpdate(int value) override {
        // std::cout << "ConcreteObserver received update: " << value << std::endl;
    }
};

// Subject implementation.
void Subject::Attach(Observer* observer) {
    observers.push_back(observer);
}

void Subject::Detach(Observer* observer) {
    observers.erase(std::remove(observers.begin(), observers.end(), observer), observers.end());
}

void Subject::Notify() {
    for (Observer* observer : observers) {
        observer->OnUpdate(GetValue());
    }
}

// Usage example.
int main() {
    ConcreteSubject subject;
    ConcreteObserver observer1(&subject);
    ConcreteObserver observer2(&subject);

    subject.Update(10);
    subject.Update(20);

    return 0;
}
#endif
/////////////////////////////////

#if 0
class SquareWave : public AbstractWave {
   public:

    SquareWave(AbstractWave *wave, uint16_t level, uint16_t skew = 0U, uint8_t increment, bool raising_edge = true) :
        AbstractWave(0x00, 0xffff, increment), m_level(level), m_skew(skew), m_raising_edge(raising_edge), m_queue(skew) {
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
            m_counter = m_raising_edge ? m_max : m_min;
        } else {
            m_counter = m_raising_edge ? m_min : m_max;
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

    SineWave(AbstractWave *wave, uint16_t steps, float amplitude, uint8_t increment, bool symetrical = true) :
        AbstractWave(0x00, 0xffff, increment), m_steps(steps), m_amplitude(amplitude), m_symetrical(symetrical), m_angle(0.0f), m_value(0.0f) {
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
#endif

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
