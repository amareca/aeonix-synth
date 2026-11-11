#pragma once

#include <algorithm>
#include <vector>

namespace aeonix
{

// ============================================================================
//  Reverb — reverberacion estilo Freeverb (8 combs + 4 allpass por canal).
//
//  Estereo, con control de "room size" y damping. process() es RT-safe
//  (solo aritmetica sobre buffers preasignados en prepare()).
// ============================================================================
class Reverb
{
public:
    Reverb() = default;

    void prepare (double sampleRate)
    {
        sampleRate_ = sampleRate;
        const float scale = static_cast<float> (sampleRate) / 44100.0f;

        static constexpr int combTuning[]     = { 1116, 1188, 1277, 1356, 1422, 1491, 1557, 1617 };
        static constexpr int allpassTuning[]  = { 556, 441, 341, 225 };
        static constexpr int stereoSpread     = 23;

        for (int c = 0; c < 2; ++c)
        {
            for (int i = 0; i < 8; ++i)
                combs_[c][i].setSize (static_cast<int> (combTuning[i] * scale) + stereoSpread + 1);

            for (int i = 0; i < 4; ++i)
                allpass_[c][i].setSize (static_cast<int> (allpassTuning[i] * scale) + stereoSpread + 1);
        }

        update();
        reset();
    }

    void setRoomSize (float size) { roomSize_ = std::clamp (size, 0.0f, 1.0f); update(); }
    void setDamping (float damp)  { damping_  = std::clamp (damp, 0.0f, 1.0f); update(); }
    void setMix (float mix)       { mix_      = std::clamp (mix, 0.0f, 1.0f); }

    void reset()
    {
        for (int c = 0; c < 2; ++c)
        {
            for (auto& comb : combs_[c]) comb.reset();
            for (auto& ap : allpass_[c])  ap.reset();
        }
    }

    void process (float* left, float* right, int numSamples)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            const float inputL = left[i];
            const float inputR = right[i];

            float outL = 0.0f;
            float outR = 0.0f;

            for (int c = 0; c < 8; ++c)
            {
                outL += combs_[0][c].process (inputL);
                outR += combs_[1][c].process (inputR);
            }

            for (int a = 0; a < 4; ++a)
            {
                outL = allpass_[0][a].process (outL);
                outR = allpass_[1][a].process (outR);
            }

            constexpr float wetGain = 0.02f;
            left[i]  = inputL * (1.0f - mix_) + outL * wetGain * mix_;
            right[i] = inputR * (1.0f - mix_) + outR * wetGain * mix_;
        }
    }

private:
    struct Comb
    {
        void setSize (int n) { buffer_.assign (static_cast<size_t> (n), 0.0f); index_ = 0; }

        void reset() { std::fill (buffer_.begin(), buffer_.end(), 0.0f); filterStore_ = 0.0f; }

        float process (float input)
        {
            const float out = buffer_[index_];

            // Damping: filtro paso-bajo en la realimentacion.
            filterStore_ = out * (1.0f - damping_) + filterStore_ * damping_;
            buffer_[index_] = input + filterStore_ * feedback_;

            if (++index_ >= static_cast<int> (buffer_.size()))
                index_ = 0;

            return out;
        }

        float feedback_    = 0.84f;
        float damping_     = 0.4f;
        float filterStore_ = 0.0f;
        std::vector<float> buffer_;
        int index_ = 0;
    };

    struct Allpass
    {
        void setSize (int n) { buffer_.assign (static_cast<size_t> (n), 0.0f); index_ = 0; }

        void reset() { std::fill (buffer_.begin(), buffer_.end(), 0.0f); }

        float process (float input)
        {
            const float buffered = buffer_[index_];
            const float out = -input + buffered;

            buffer_[index_] = input + buffered * 0.5f;

            if (++index_ >= static_cast<int> (buffer_.size()))
                index_ = 0;

            return out;
        }

        std::vector<float> buffer_;
        int index_ = 0;
    };

    void update()
    {
        const float fb   = roomSize_ * 0.28f + 0.7f;   // 0.7 .. 0.98
        const float damp = damping_ * 0.4f;

        for (int c = 0; c < 2; ++c)
            for (auto& comb : combs_[c])
            {
                comb.feedback_ = fb;
                comb.damping_  = damp;
            }
    }

    double sampleRate_ = 44100.0;
    float roomSize_    = 0.5f;
    float damping_     = 0.4f;
    float mix_         = 0.15f;

    Comb    combs_[2][8];
    Allpass allpass_[2][4];
};

} // namespace aeonix
