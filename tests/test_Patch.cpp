#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "domain/Patch.h"
#include "domain/ParameterIds.h"

using namespace aeonix;

TEST_CASE ("Default patch contains the expected parameters", "[patch]")
{
    auto patch = makeDefaultPatch();

    REQUIRE (patch.size() >= 9);
    REQUIRE (patch.contains (param::masterVolume));
    REQUIRE (patch.contains (param::filterCutoff));
    REQUIRE (patch.contains (param::ampAttack));
}

TEST_CASE ("Patch set/get round-trips normalised values", "[patch]")
{
    auto patch = makeDefaultPatch();

    REQUIRE (patch.set (param::filterCutoff, 0.25f));
    REQUIRE (patch.get (param::filterCutoff) == Catch::Approx (0.25f));
}

TEST_CASE ("Patch set returns false for an unknown id", "[patch]")
{
    auto patch = makeDefaultPatch();

    REQUIRE_FALSE (patch.set ("no.existe", 0.5f));
    REQUIRE_FALSE (patch.contains ("no.existe"));
}

TEST_CASE ("Patch getValue returns the real (denormalised) value", "[patch]")
{
    auto patch = makeDefaultPatch();

    patch.set (param::masterVolume, 0.5f);
    REQUIRE (patch.getValue (param::masterVolume) == Catch::Approx (0.5f));
}
