#pragma once

#include <map>
#include <string>
#include <vector>

namespace aeonix
{

// ============================================================================
//  Preset - a synthesizer preset: a name + the normalized value (0..1)
//  of each parameter. Pure C++ (no JUCE, no I/O).
// ============================================================================
struct Preset
{
    std::string name;
    std::map<std::string, float> parameters;   // id -> normalized value 0..1
};

// Parameters of the default patch (id -> normalized).
std::map<std::string, float> makeDefaultParameters();

// Factory presets: Default + v1 library (7 families, 35 presets).
std::vector<Preset> makeFactoryPresets();

} // namespace aeonix
