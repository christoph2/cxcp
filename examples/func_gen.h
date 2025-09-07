
#ifndef __FUNC_GEN_H
#define __FUNC_GEN_H

#include <cmath>
#include <cstdint>

#define PI (3.14159265358979323846)


enum WaveformType {
    SINE,
    COSINE,
    TRIANGLE,
    SQUARE,
    SAWTOOTH,
    PRBS
};

struct WaveformParams {
    float frequency;     // Hz
    float amplitude;     // Peak value
    float phase;         // Radians
    float pulse_width;    // 0.0 - 1.0 (only SQ)
};

inline float amplitiude_value(float base=1.0F, float amplification=0.0F) {
    return base * (::pow(10.0F, amplification / 20.0F));
}

class FunctionGenerator {
private:
    WaveformType m_type;
    WaveformParams m_params;
    float m_sample_rate;
    uint32_t m_pbrs_state;

public:
    FunctionGenerator(WaveformType t, WaveformParams p, float sr)
        : m_type(t), m_params(p), m_sample_rate(sr), m_pbrs_state(0xACE1u) {}

    float generateSample(uint32_t sample_index) {
        float t = sample_index / m_sample_rate;
        float omega = 2.0F * PI * m_params.frequency;
        float phase_shift = m_params.phase;

        switch (m_type) {
            case SINE:
                return m_params.amplitude * ::sin(omega * t + phase_shift);
            case COSINE:
                return m_params.amplitude * ::cos(omega * t + phase_shift);
            case TRIANGLE:
                return m_params.amplitude * (2.0F * ::abs(2.0F * (t * m_params.frequency - ::floor(t * m_params.frequency + 0.5F))) - 1.0F);
            case SQUARE:
                return (::fmod(t * m_params.frequency + phase_shift / (2.0F * PI), 1.0F) < m_params.pulse_width)
                    ? m_params.amplitude : -m_params.amplitude;
            case SAWTOOTH:
                return m_params.amplitude * (2.0F * (t * m_params.frequency - ::floor(t * m_params.frequency + 0.5F)));
            case PRBS:
                return generatePRBS();
            default:
                return 0.0F;
        }
    }

private:
    float generatePRBS() {
        // 16-bit LFSR PRBS
        uint32_t bit = ((m_pbrs_state >> 0) ^ (m_pbrs_state >> 2) ^ (m_pbrs_state >> 3) ^ (m_pbrs_state >> 5)) & 1;
        m_pbrs_state = (m_pbrs_state >> 1) | (bit << 15);
        return (m_pbrs_state & 1) ? m_params.amplitude : -m_params.amplitude;
    }
};

#endif  // __FUNC_GEN_H
