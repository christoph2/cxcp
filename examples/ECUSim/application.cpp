#include "application.h"

#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <thread>

extern "C" {
#include "xcp.h"
#include "xcp_threads.h"
}

extern "C" {
    volatile float values[8];

    volatile std::uint16_t triangle_min = 0x00U;
    volatile std::uint16_t triangle_max = 0xffU;

    volatile std::uint32_t time_point;
    volatile std::uint32_t elapsed_time;

    volatile float voltage1;
    volatile float voltage2;
    volatile float voltage3;
    volatile float voltage4;

    volatile std::uint8_t sq0_wave;
    volatile std::uint8_t sq1_wave;

    XCP_DAQ_BEGIN_EVENTS
    XCP_DAQ_DEFINE_EVENT(
        "EVT 10ms", XCP_DAQ_EVENT_CHANNEL_TYPE_DAQ | XCP_DAQ_CONSISTENCY_DAQ_LIST /* | XCP_DAQ_EVENT_CHANNEL_PACKED*/,
        XCP_DAQ_EVENT_CHANNEL_TIME_UNIT_1MS, 10
    ),
        XCP_DAQ_DEFINE_EVENT(
            "EVT sporadic", XCP_DAQ_EVENT_CHANNEL_TYPE_DAQ | XCP_DAQ_CONSISTENCY_DAQ_LIST /* | XCP_DAQ_EVENT_CHANNEL_PACKED*/,
            XCP_DAQ_EVENT_CHANNEL_TIME_UNIT_1MS, 0
        ),
        XCP_DAQ_END_EVENTS
}

enum class WaveformType {
    Sine,
    Triangle,
    Square,
    Sawtooth
};

static float amplitude_value(float base = 1.0F, float amplification = 0.0F) {
    return base * std::pow(10.0F, amplification / 20.0F);
}

class WaveformGenerator {
   public:

    WaveformGenerator(WaveformType type, float frequency, float sample_rate, float amplitude = 1.0F, float phase = 0.0F) :
        phase_(phase), phase_increment_(frequency / sample_rate), amplitude_(amplitude), type_(type) {
    }

    float next_sample() {
        constexpr float pi = 3.14159265358979323846F;

        phase_ += phase_increment_;
        if (phase_ >= 1.0F) {
            phase_ -= 1.0F;
        }

        float value = 0.0F;
        switch (type_) {
            case WaveformType::Sine:
                value = std::sin(2.0F * pi * phase_);
                break;
            case WaveformType::Triangle:
                value = 2.0F * std::fabs(2.0F * (phase_ - std::floor(phase_ + 0.5F))) - 1.0F;
                break;
            case WaveformType::Square:
                value = (phase_ < 0.5F) ? 1.0F : -1.0F;
                break;
            case WaveformType::Sawtooth:
                value = 2.0F * (phase_ - 0.5F);
                break;
        }
        return amplitude_ * value;
    }

   private:

    float        phase_;
    float        phase_increment_;
    float        amplitude_;
    WaveformType type_;
};

void EcuSim_RunApplication(void) {
    using Clock = std::chrono::steady_clock;
    using namespace std::chrono;

    constexpr float   amplitude_scale = 32.0F;
    WaveformGenerator wg1(WaveformType::Sine, 16.0F, 100.0F, amplitude_value(amplitude_scale));
    WaveformGenerator wg2(WaveformType::Sine, 16.0F, 100.0F, amplitude_value(amplitude_scale, -3.0F), 90.0F);
    WaveformGenerator wg3(WaveformType::Triangle, 10.0F, 100.0F, amplitude_value(amplitude_scale, -6.0F));
    WaveformGenerator wg4(WaveformType::Sawtooth, 10.0F, 100.0F, amplitude_value(amplitude_scale, -6.0F), -90.0F);
    // const auto period = milliseconds(10);
    const auto period    = microseconds(50);
    auto       next_tick = Clock::now();
#if 0
    auto stats_window_start = Clock::now();
#endif

    std::uint64_t trigger_count    = 0U;
    std::uint64_t overrun_count    = 0U;
    std::uint64_t busy_time_sum_us = 0U;
    std::uint32_t busy_time_max_us = 0U;
    std::int64_t  wake_late_max_us = 0;

    (void)wg2;
    (void)wg4;
    sq0_wave = 0xaaU;
    sq1_wave = 0x55U;
    voltage1 = 0.0F;
    voltage2 = 0.0F;
    voltage3 = 0.0F;
    voltage4 = 10.0F;

    printf("voltage1: %p, voltage2: %p, voltage3: %p, voltage4: %p\n", &voltage1, &voltage2, &voltage3, &voltage4);
    fflush(stdout);

    while (!XcpThrd_IsShuttingDown()) {
        next_tick += period;
        const auto start = Clock::now();

        voltage1 = wg1.next_sample();
        voltage2 = wg2.next_sample();
        voltage3 = wg3.next_sample();
        voltage4 += 0.05F;
        XcpDaq_TriggerEvent(0);

        elapsed_time = static_cast<std::uint32_t>(duration_cast<microseconds>(Clock::now() - start).count());
        ++trigger_count;
        busy_time_sum_us += elapsed_time;
        if (elapsed_time > busy_time_max_us) {
            busy_time_max_us = elapsed_time;
        }
        ::time_point = XcpHw_GetTimerCounter();

        // std::this_thread::sleep_until(next_tick);
        const auto wake_late_us = duration_cast<microseconds>(Clock::now() - next_tick).count();
        if (wake_late_us > wake_late_max_us) {
            wake_late_max_us = wake_late_us;
        }

        if (Clock::now() > next_tick + period) {
            ++overrun_count;
            next_tick = Clock::now();
        }

#if 0
        const auto now = Clock::now();
        if (now - stats_window_start >= seconds(1)) {
            const auto avg_busy_us = (trigger_count > 0U) ? (busy_time_sum_us / trigger_count) : 0U;
            printf(
                "[ECUSim perf] triggers/s=%llu avg_busy_us=%llu max_busy_us=%u overruns/s=%llu max_wake_late_us=%lld\n",
                static_cast<unsigned long long>(trigger_count),
                static_cast<unsigned long long>(avg_busy_us),
                static_cast<unsigned>(busy_time_max_us),
                static_cast<unsigned long long>(overrun_count),
                static_cast<long long>(wake_late_max_us)
            );
            fflush(stdout);

            stats_window_start = now;
            trigger_count = 0U;
            overrun_count = 0U;
            busy_time_sum_us = 0U;
            busy_time_max_us = 0U;
            wake_late_max_us = 0;
        }
#endif
    }
}
