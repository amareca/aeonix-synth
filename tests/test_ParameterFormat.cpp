#include <catch2/catch_test_macros.hpp>

#include "domain/ParameterFormat.h"

using namespace aeonix;

TEST_CASE ("ParameterFormat shows real units", "[format]")
{
    // Frecuencias.
    REQUIRE (formatParameterValue ("filter.cutoff", 0.0f) == "20 Hz");
    REQUIRE (formatParameterValue ("filter.cutoff", 1.0f) == "20 kHz");

    // Tiempos.
    REQUIRE (formatParameterValue ("amp.attack", 0.0f) == "1 ms");
    REQUIRE (formatParameterValue ("fx.delay.time", 0.0f) == "50 ms");

    // Cents y porcentajes.
    REQUIRE (formatParameterValue ("unison.detune", 0.2f) == "10 ct");
    REQUIRE (formatParameterValue ("master.volume", 0.5f) == "50%");
    REQUIRE (formatParameterValue ("filter.envAmount", 0.65f) == "65%");

    // Formas de onda y voces.
    REQUIRE (formatParameterValue ("osc.waveform", 0.0f) == "Saw");
    REQUIRE (formatParameterValue ("osc.waveform", 1.0f) == "Triangle");
    REQUIRE (formatParameterValue ("unison.voices", 1.0f) == "8");
}

TEST_CASE ("ParameterFormat returns empty for unknown ids", "[format]")
{
    REQUIRE (formatParameterValue ("no.existe", 0.5f).empty());
}
