#include <catch2/catch_test_macros.hpp>

#include "presentation/core/TextSize.h"

using namespace aeonix;

TEST_CASE ("TextSize keys round-trip", "[textsize]")
{
    REQUIRE (textSizeKey (TextSize::normal) == "normal");
    REQUIRE (textSizeKey (TextSize::large) == "large");
    REQUIRE (textSizeKey (TextSize::extraLarge) == "xlarge");

    REQUIRE (textSizeFromKey ("normal") == TextSize::normal);
    REQUIRE (textSizeFromKey ("large") == TextSize::large);
    REQUIRE (textSizeFromKey ("xlarge") == TextSize::extraLarge);
    REQUIRE (textSizeFromKey ("desconocido") == TextSize::normal);
    REQUIRE (textSizeFromKey ("") == TextSize::normal);
}

TEST_CASE ("TextSize picks a readable default from physical screen height", "[textsize]")
{
    // 768 px se queda en Normal; 1080p arranca en Grande; 1440p en Muy grande.
    REQUIRE (defaultTextSizeForHeight (720) == TextSize::normal);
    REQUIRE (defaultTextSizeForHeight (768) == TextSize::normal);
    REQUIRE (defaultTextSizeForHeight (899) == TextSize::normal);
    REQUIRE (defaultTextSizeForHeight (900) == TextSize::large);
    REQUIRE (defaultTextSizeForHeight (1040) == TextSize::large);
    REQUIRE (defaultTextSizeForHeight (1299) == TextSize::large);
    REQUIRE (defaultTextSizeForHeight (1300) == TextSize::extraLarge);
    REQUIRE (defaultTextSizeForHeight (2160) == TextSize::extraLarge);
}

TEST_CASE ("TextSize scales fonts more than layout and limits monotonically", "[textsize]")
{
    REQUIRE (textFontScaleForSize (TextSize::normal) == 1.0f);
    REQUIRE (textFontScaleForSize (TextSize::large) == 15.0f / 12.0f);
    REQUIRE (textFontScaleForSize (TextSize::extraLarge) == 18.0f / 12.0f);

    REQUIRE (textLayoutScaleForSize (TextSize::normal) == 1.0f);
    REQUIRE (textLayoutScaleForSize (TextSize::large) == 1.07f);
    REQUIRE (textLayoutScaleForSize (TextSize::extraLarge) == 1.15f);

    REQUIRE (textFontScaleForSize (TextSize::large) > textLayoutScaleForSize (TextSize::large));
    REQUIRE (textFontScaleForSize (TextSize::extraLarge) > textLayoutScaleForSize (TextSize::extraLarge));

    REQUIRE (minWindowWidthForSize (TextSize::normal) == 1042);
    REQUIRE (minWindowWidthForSize (TextSize::large) == 1160);
    REQUIRE (minWindowWidthForSize (TextSize::extraLarge) == 1300);

    REQUIRE (minWindowHeightForSize (TextSize::normal) == 714);
    REQUIRE (minWindowHeightForSize (TextSize::large) == 724);
    REQUIRE (minWindowHeightForSize (TextSize::extraLarge) == 744);

    // Grande debe caber en 1366x768; Muy grande queda para 1080p.
    REQUIRE (minWindowWidthForSize (TextSize::large) <= 1366);
    REQUIRE (minWindowHeightForSize (TextSize::large) <= 730);
    REQUIRE (minWindowHeightForSize (TextSize::extraLarge) > 730);
}

TEST_CASE ("TextSize default window grows with the level", "[textsize]")
{
    REQUIRE (defaultWindowWidthForSize (TextSize::large) >= minWindowWidthForSize (TextSize::large));
    REQUIRE (defaultWindowHeightForSize (TextSize::large) >= minWindowHeightForSize (TextSize::large));
    REQUIRE (defaultWindowWidthForSize (TextSize::extraLarge) >= minWindowWidthForSize (TextSize::extraLarge));
    REQUIRE (defaultWindowHeightForSize (TextSize::extraLarge) >= minWindowHeightForSize (TextSize::extraLarge));
}
