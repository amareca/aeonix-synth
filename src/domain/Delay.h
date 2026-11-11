#pragma once

#include <algorithm>
#include <vector>

namespace aeonix
{

// ============================================================================
//  Delay — linea de retardo estereo con realimentacion (feedback) y mix.
//
//  process() solo hace aritmetica sobre buffers preasignados en prepare()
//  (RT-safe). El tiempo de retardo se fija con setTime().
// ============================================================================
class Delay
{
public:
    Delay() = default;

    void prepare (double sampleRate, double maxTimeSeconds = 2.0)
    {
        sampleRate_ = sampleRate;
        bufferSize_ = static_cast<int> (sampleRate * maxTimeSeconds) + 2;
        bufferL_.assign (static_cast<size_t> (bufferSize_), 0.0f);
        bufferR_.assign (static_cast<size_t> (bufferSize_), 0.0f);
        writeIndex_ = 0;
        updateDelaySamples();
    }

    void setTime (double seconds)
    {
        timeSeconds_ = seconds;
        updateDelaySamples();
    }

    void setFeedback (float fb) { feedback_ = std::clamp (fb, 0.0f, 0.95f); }
    void setMix (float mix)     { mix_ = std::clamp (mix, 0.0f, 1.0f); }

    void reset()
    {
        std::fill (bufferL_.begin(), bufferL_.end(), 0.0f);
        std::fill (bufferR_.begin(), bufferR_.end(), 0.0f);
    }

    void process (float* left, float* right, int numSamples)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            const int readIndex = (writeIndex_ - delaySamples_ + bufferSize_) % bufferSize_;

            const float delayedL = bufferL_[readIndex];
            const float delayedR = bufferR_[readIndex];

            bufferL_[writeIndex_] = left[i]  + delayedL * feedback_;
            bufferR_[writeIndex_] = right[i] + delayedR * feedback_;

            left[i]  = left[i]  * (1.0f - mix_) + delayedL * mix_;
            right[i] = right[i] * (1.0f - mix_) + delayedR * mix_;

            if (++writeIndex_ >= bufferSize_)
                writeIndex_ = 0;
        }
    }

private:
    void updateDelaySamples()
    {
        const int s = static_cast<int> (timeSeconds_ * sampleRate_);
        delaySamples_ = std::clamp (s, 0, bufferSize_ - 2);
    }

    std::vector<float> bufferL_;
    std::vector<float> bufferR_;
    int bufferSize_    = 0;
    int writeIndex_    = 0;
    int delaySamples_  = 0;
    double sampleRate_ = 44100.0;
    double timeSeconds_ = 0.3;
    float feedback_    = 0.4f;
    float mix_         = 0.3f;
};

} // namespace aeonix
