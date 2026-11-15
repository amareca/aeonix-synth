#pragma once

#include <juce_core/juce_core.h>

namespace aeonix
{

// Waveforms shared by the oscillator and the LFO.
inline juce::StringArray waveformNames()
{
    juce::StringArray names;
    names.add ("Saw");
    names.add ("Square");
    names.add ("Sine");
    names.add ("Triangle");
    return names;
}

} // namespace aeonix
