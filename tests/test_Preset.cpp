#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <set>
#include <string>

#include "domain/Preset.h"

using namespace aeonix;

TEST_CASE ("Factory presets have the expected names", "[preset]")
{
    const auto presets = makeFactoryPresets();

    REQUIRE (presets.size() >= 7);
    REQUIRE (presets[0].name == "Default");
    REQUIRE (presets[1].name == "Pluck");

    // Debe haber varios pads.
    int padCount = 0;
    for (const auto& p : presets)
        if (p.name.find ("Pad") != std::string::npos)
            ++padCount;

    REQUIRE (padCount >= 5);
}

TEST_CASE ("Factory presets include three leads and three strings", "[preset]")
{
    int leadCount = 0;
    int stringCount = 0;

    for (const auto& p : makeFactoryPresets())
    {
        if (p.name.rfind ("Lead ", 0) == 0)
            ++leadCount;

        if (p.name.rfind ("String ", 0) == 0)
            ++stringCount;
    }

    REQUIRE (leadCount == 3);
    REQUIRE (stringCount == 3);
}

TEST_CASE ("Factory preset names are unique", "[preset]")
{
    std::set<std::string> names;

    for (const auto& p : makeFactoryPresets())
        names.insert (p.name);

    REQUIRE (names.size() == makeFactoryPresets().size());
}

TEST_CASE ("Factory preset values are valid and normalised", "[preset]")
{
    const auto defaults = makeDefaultParameters();

    for (const auto& p : makeFactoryPresets())
    {
        for (const auto& [id, value] : p.parameters)
        {
            REQUIRE (defaults.count (id) == 1);   // sin ids desconocidos
            REQUIRE (value >= 0.0f);
            REQUIRE (value <= 1.0f);
        }
    }
}

TEST_CASE ("Leads are fast and strings are slow and wide", "[preset]")
{
    for (const auto& p : makeFactoryPresets())
    {
        const bool isLead   = p.name.rfind ("Lead ", 0) == 0;
        const bool isString = p.name.rfind ("String ", 0) == 0;

        if (isLead)
            REQUIRE (p.parameters.at ("amp.attack") < 0.3f);

        if (isString)
        {
            REQUIRE (p.parameters.at ("amp.attack") > 0.5f);
            REQUIRE (p.parameters.at ("amp.release") > 0.6f);
            REQUIRE (p.parameters.at ("unison.voices") > 0.0f);
            REQUIRE (p.parameters.at ("unison.width") > 0.5f);
        }
    }
}

TEST_CASE ("Default parameters contain all expected ids", "[preset]")
{
    const auto p = makeDefaultParameters();

    REQUIRE (p.count ("master.volume") == 1);
    REQUIRE (p.count ("filter.cutoff") == 1);
    REQUIRE (p.count ("filter.envAmount") == 1);
    REQUIRE (p.count ("filter.attack") == 1);
    REQUIRE (p.count ("amp.attack") == 1);
    REQUIRE (p.count ("fx.delay.time") == 1);
    REQUIRE (p.count ("fx.reverb.mix") == 1);
    REQUIRE (p.count ("unison.width") == 1);
}

TEST_CASE ("Pluck preset differs from the default", "[preset]")
{
    const auto def = makeDefaultParameters();

    std::map<std::string, float> pluck;
    for (const auto& p : makeFactoryPresets())
        if (p.name == "Pluck")
            pluck = p.parameters;

    REQUIRE_FALSE (pluck.empty());
    REQUIRE (pluck.at ("amp.attack") != Catch::Approx (def.at ("amp.attack")));
}

TEST_CASE ("Pads use unison, width and LFO for movement", "[preset]")
{
    for (const auto& p : makeFactoryPresets())
    {
        if (p.name.find ("Pad") != std::string::npos)
        {
            // unison activo (> 1 voz), pan estereo y LFO activo (depth > 0).
            REQUIRE (p.parameters.at ("unison.voices") > 0.0f);
            REQUIRE (p.parameters.at ("unison.width") > 0.0f);
            REQUIRE (p.parameters.at ("lfo.depth") > 0.0f);
        }
    }
}

TEST_CASE ("Pluck and Acid Bass use the filter envelope", "[preset]")
{
    for (const auto& p : makeFactoryPresets())
    {
        if (p.name == "Pluck" || p.name == "Acid Bass")
            REQUIRE (p.parameters.at ("filter.envAmount") > 0.0f);
    }
}
