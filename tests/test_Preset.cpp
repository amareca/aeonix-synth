#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include <algorithm>
#include <cmath>
#include <set>
#include <string>

#include "domain/Preset.h"

using namespace aeonix;

namespace
{

bool endsWith (const std::string& text, const std::string& suffix)
{
    return text.size() >= suffix.size()
           && text.compare (text.size() - suffix.size(), suffix.size(), suffix) == 0;
}

int countWithSuffix (const std::vector<Preset>& presets, const std::string& suffix)
{
    int count = 0;

    for (const auto& preset : presets)
        if (endsWith (preset.name, suffix))
            ++count;

    return count;
}

const Preset* findPreset (const std::vector<Preset>& presets, const std::string& name)
{
    for (const auto& preset : presets)
        if (preset.name == name)
            return &preset;

    return nullptr;
}

} // namespace

TEST_CASE ("Factory presets have the expected names", "[preset]")
{
    const auto presets = makeFactoryPresets();

    REQUIRE (presets.size() == 36);
    REQUIRE (presets[0].name == "Default");
    REQUIRE (presets[1].name == "Aurora Pad");
}

TEST_CASE ("Factory library has five presets per family", "[preset]")
{
    const auto presets = makeFactoryPresets();

    REQUIRE (countWithSuffix (presets, " Pad") == 5);
    REQUIRE (countWithSuffix (presets, " Pluck") == 5);
    REQUIRE (countWithSuffix (presets, " Lead") == 5);
    REQUIRE (countWithSuffix (presets, " Bass") == 5);
    REQUIRE (countWithSuffix (presets, " Brass") == 5);
    REQUIRE (countWithSuffix (presets, " Piano") == 5);
    REQUIRE (countWithSuffix (presets, " Strings") == 5);
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
        REQUIRE (p.parameters.size() == defaults.size());

        for (const auto& [id, value] : p.parameters)
        {
            REQUIRE (defaults.count (id) == 1);   // sin ids desconocidos
            REQUIRE (value >= 0.0f);
            REQUIRE (value <= 1.0f);
        }
    }
}

TEST_CASE ("Factory library keeps one note below the limiter", "[preset]")
{
    for (const auto& p : makeFactoryPresets())
    {
        if (p.name == "Default")
            continue;

        const int voices = 1 + static_cast<int> (
            std::lround (p.parameters.at ("unison.voices") * 7.0f));
        const float expected = std::min (0.8f, 1.1f / static_cast<float> (voices));

        REQUIRE (p.parameters.at ("osc.level") == Catch::Approx (expected).margin (0.001f));
    }
}

TEST_CASE ("Leads are fast and strings are slow and wide", "[preset]")
{
    for (const auto& p : makeFactoryPresets())
    {
        if (endsWith (p.name, " Lead"))
            REQUIRE (p.parameters.at ("amp.attack") < 0.3f);

        if (endsWith (p.name, " Strings"))
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

    REQUIRE (p.size() == 25);
    REQUIRE (p.count ("master.volume") == 1);
    REQUIRE (p.count ("filter.cutoff") == 1);
    REQUIRE (p.count ("filter.envAmount") == 1);
    REQUIRE (p.count ("filter.attack") == 1);
    REQUIRE (p.count ("amp.attack") == 1);
    REQUIRE (p.count ("fx.delay.time") == 1);
    REQUIRE (p.count ("fx.reverb.mix") == 1);
    REQUIRE (p.count ("unison.width") == 1);
}

TEST_CASE ("Glass Pluck differs from the default", "[preset]")
{
    const auto defaults = makeDefaultParameters();
    const auto presets = makeFactoryPresets();
    const auto* pluck = findPreset (presets, "Glass Pluck");

    REQUIRE (pluck != nullptr);
    REQUIRE (pluck->parameters.at ("amp.attack") != Catch::Approx (defaults.at ("amp.attack")));
}

TEST_CASE ("Pads use unison, width and LFO for movement", "[preset]")
{
    for (const auto& p : makeFactoryPresets())
    {
        if (endsWith (p.name, " Pad"))
        {
            REQUIRE (p.parameters.at ("unison.voices") > 0.0f);
            REQUIRE (p.parameters.at ("unison.width") > 0.0f);
            REQUIRE (p.parameters.at ("lfo.depth") > 0.0f);
        }
    }
}

TEST_CASE ("Plucks and Acid Line Bass use the filter envelope", "[preset]")
{
    for (const auto& p : makeFactoryPresets())
    {
        if (endsWith (p.name, " Pluck") || p.name == "Acid Line Bass")
            REQUIRE (p.parameters.at ("filter.envAmount") > 0.0f);
    }
}
