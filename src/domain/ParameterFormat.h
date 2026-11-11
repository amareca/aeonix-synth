#pragma once

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

#include "ParameterIds.h"
#include "Patch.h"

namespace aeonix
{

// ============================================================================
//  ParameterFormat — convierte un valor normalizado (0..1) al valor real del
//  parametro y lo formatea con unidades legibles (Hz, ms/s, cents, %).
//
//  Reutiliza makeDefaultPatch() como unica fuente de rangos y mapeo
//  logaritmico, para no duplicar esa informacion en la GUI.
//  C++ puro: se puede probar sin JUCE.
// ============================================================================

namespace detail
{

inline const Patch& displayPatch()
{
    static const Patch patch = makeDefaultPatch();
    return patch;
}

// Formatea con "decimals" decimales y quita ceros finales ("20.00" -> "20").
inline std::string formatNumber (double value, int decimals)
{
    char buffer[64] = {};
    std::snprintf (buffer, sizeof (buffer), "%.*f", decimals, value);

    std::string s (buffer);
    if (const auto dot = s.find ('.'); dot != std::string::npos)
    {
        while (! s.empty() && s.back() == '0')
            s.pop_back();

        if (! s.empty() && s.back() == '.')
            s.pop_back();
    }

    return s;
}

inline bool isTimeParameter (const std::string& id)
{
    return id == param::ampAttack   || id == param::ampDecay   || id == param::ampRelease
        || id == param::filterAttack || id == param::filterDecay || id == param::filterRelease
        || id == param::delayTime;
}

} // namespace detail

inline std::string formatParameterValue (const std::string& id, float normalised)
{
    const Parameter* definition = detail::displayPatch().find (id);
    if (definition == nullptr)
        return {};

    // Copia la definicion y aplica el valor normalizado recibido: getValue()
    // usa el valor guardado en el Parameter, no el argumento.
    Parameter p = *definition;
    p.setNormalised (normalised);
    const float value = p.getValue();

    // Formas de onda: indice -> nombre.
    if (id == param::oscWaveform || id == param::lfoWaveform)
    {
        static const char* const names[] = { "Saw", "Square", "Sine", "Triangle" };
        const int index = std::clamp (static_cast<int> (std::lround (value)), 0, 3);
        return names[index];
    }

    // Frecuencias.
    if (id == param::filterCutoff || id == param::lfoRate)
    {
        if (value >= 1000.0f)
            return detail::formatNumber (value / 1000.0f, 1) + " kHz";

        if (value >= 100.0f)
            return detail::formatNumber (value, 0) + " Hz";

        return detail::formatNumber (value, 1) + " Hz";
    }

    // Tiempos: ms por debajo de 1 s, segundos por encima.
    if (detail::isTimeParameter (id))
    {
        if (value < 1.0f)
        {
            const float ms = value * 1000.0f;
            return detail::formatNumber (ms, ms < 10.0f ? 1 : 0) + " ms";
        }

        return detail::formatNumber (value, 2) + " s";
    }

    // Desafinacion de unison (cents).
    if (id == param::unisonDetune)
        return detail::formatNumber (value, 1) + " ct";

    // Numero de voces de unison: entero sin unidad.
    if (id == param::unisonVoices)
        return detail::formatNumber (value, 0);

    // Todo lo demas es un nivel normalizado -> porcentaje.
    return detail::formatNumber (static_cast<double> (normalised) * 100.0, 0) + "%";
}

} // namespace aeonix
