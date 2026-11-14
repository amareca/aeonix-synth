#pragma once

#include <map>
#include <string>
#include <vector>

namespace aeonix
{

// ============================================================================
//  Preset — un preset del sintetizador: un nombre + el valor normalizado (0..1)
//  de cada parametro. Pura C++ (sin JUCE, sin I/O).
// ============================================================================
struct Preset
{
    std::string name;
    std::map<std::string, float> parameters;   // id -> valor normalizado 0..1
};

// Parametros del patch por defecto (id -> normalizado).
std::map<std::string, float> makeDefaultParameters();

// Presets de fabrica: Default + biblioteca v1 (7 familias, 35 presets).
std::vector<Preset> makeFactoryPresets();

} // namespace aeonix
