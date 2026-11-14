#include <catch2/catch_test_macros.hpp>

#include "infrastructure/PresetLibrary.h"

using namespace aeonix;

namespace
{

juce::File makeTempRoot (const juce::String& name)
{
    auto root = juce::File::getSpecialLocation (juce::File::tempDirectory)
                    .getChildFile ("AeonixTests")
                    .getChildFile (name);

    root.deleteRecursively();
    root.createDirectory();
    return root;
}

} // namespace

TEST_CASE ("Preset categories round-trip their stable keys", "[preset][library]")
{
    const PresetCategory all[] = {
        PresetCategory::initiation,
        PresetCategory::bass,
        PresetCategory::lead,
        PresetCategory::pad,
        PresetCategory::strings,
        PresetCategory::keys,
        PresetCategory::fx,
        PresetCategory::other
    };

    REQUIRE (std::size (all) == static_cast<std::size_t> (presetCategoryCount));

    for (const auto category : all)
        REQUIRE (PresetLibrary::categoryFromKey (PresetLibrary::categoryKey (category)) == category);

    REQUIRE (PresetLibrary::categoryKey (PresetCategory::initiation) == "iniciacion");
    REQUIRE (PresetLibrary::categoryKey (PresetCategory::bass) == "bajo");
    REQUIRE (PresetLibrary::categoryKey (PresetCategory::strings) == "cuerdas");
    REQUIRE (PresetLibrary::categoryKey (PresetCategory::keys) == "teclas");
    REQUIRE (PresetLibrary::categoryKey (PresetCategory::other) == "otros");

    REQUIRE (PresetLibrary::categoryFromKey ("desconocida") == PresetCategory::other);
    REQUIRE (PresetLibrary::categoryFromKey ("") == PresetCategory::other);
}

TEST_CASE ("Factory presets have a static category", "[preset][library]")
{
    REQUIRE (factoryCategoryFor ("Default") == PresetCategory::initiation);
    REQUIRE (factoryCategoryFor ("Glass Pluck") == PresetCategory::keys);
    REQUIRE (factoryCategoryFor ("Acid Line Bass") == PresetCategory::bass);

    REQUIRE (factoryCategoryFor ("Copper Lead") == PresetCategory::lead);
    REQUIRE (factoryCategoryFor ("Hollow Lead") == PresetCategory::lead);
    REQUIRE (factoryCategoryFor ("Velvet Lead") == PresetCategory::lead);

    REQUIRE (factoryCategoryFor ("Warm Strings") == PresetCategory::strings);
    REQUIRE (factoryCategoryFor ("Soft Strings") == PresetCategory::strings);
    REQUIRE (factoryCategoryFor ("Bright Strings") == PresetCategory::strings);

    REQUIRE (factoryCategoryFor ("Aurora Pad") == PresetCategory::pad);
    REQUIRE (factoryCategoryFor ("Nocturne Pad") == PresetCategory::pad);
    REQUIRE (factoryCategoryFor ("Frost Pad") == PresetCategory::pad);

    REQUIRE (factoryCategoryFor ("Soft Brass") == PresetCategory::other);
    REQUIRE (factoryCategoryFor ("Felt Piano") == PresetCategory::keys);

    REQUIRE (factoryCategoryFor ("Lush Pad43") == PresetCategory::other);
    REQUIRE (factoryCategoryFor ("") == PresetCategory::other);
}

TEST_CASE ("Missing sidecar falls back to factory categories", "[preset][library]")
{
    const auto root = makeTempRoot ("library-defaults");
    PresetLibrary library (root.getChildFile ("library.json"));

    library.load();

    REQUIRE (library.factoryEntries().empty());
    REQUIRE (library.userEntries().empty());

    const auto lead = library.metadataFor ("Copper Lead", true);
    REQUIRE (lead.category == PresetCategory::lead);
    REQUIRE_FALSE (lead.favorite);
    REQUIRE (lead.lastUsed == 0);
    REQUIRE_FALSE (lead.orphan);

    const auto user = library.metadataFor ("Lush Pad43", false);
    REQUIRE (user.category == PresetCategory::other);
    REQUIRE_FALSE (user.favorite);
}

TEST_CASE ("Sidecar keeps categories, favourites and last use", "[preset][library]")
{
    const auto root = makeTempRoot ("library-round-trip");
    const auto file = root.getChildFile ("library.json");

    PresetLibrary library (file);
    library.load();

    library.setCategory ("Lush Pad43", false, PresetCategory::pad);
    library.setFavorite ("Lush Pad43", false, true);
    library.markUsed ("Lush Pad43", false, 1234567890123LL);
    library.setFavorite ("Copper Lead", true, true);

    REQUIRE (library.save());
    REQUIRE (file.existsAsFile());

    PresetLibrary reloaded (file);
    reloaded.load();

    const auto user = reloaded.metadataFor ("Lush Pad43", false);
    REQUIRE (user.category == PresetCategory::pad);
    REQUIRE (user.favorite);
    REQUIRE (user.lastUsed == 1234567890123LL);

    // Un favorito de fabrica sin categoria explicita conserva la tabla estatica.
    const auto factory = reloaded.metadataFor ("Copper Lead", true);
    REQUIRE (factory.category == PresetCategory::lead);
    REQUIRE (factory.favorite);
}

TEST_CASE ("Corrupt sidecar is ignored without losing factory table", "[preset][library]")
{
    const auto root = makeTempRoot ("library-corrupt");
    const auto file = root.getChildFile ("library.json");
    file.replaceWithText ("{ esto no es json valido");

    PresetLibrary library (file);
    library.load();

    REQUIRE (library.factoryEntries().empty());
    REQUIRE (library.userEntries().empty());
    REQUIRE (library.metadataFor ("Glass Pluck", true).category == PresetCategory::keys);
    REQUIRE (library.metadataFor ("Cualquiera", false).category == PresetCategory::other);
}

TEST_CASE ("Unknown category keys fall back to other", "[preset][library]")
{
    const auto root = makeTempRoot ("library-unknown-category");
    const auto file = root.getChildFile ("library.json");

    file.replaceWithText (
        R"({"version":1,"factory":{"Copper Lead":{"favorite":true}},)"
        R"("user":{"Raro":{"category":"desconocida","favorite":true}}})");

    PresetLibrary library (file);
    library.load();

    REQUIRE (library.metadataFor ("Raro", false).category == PresetCategory::other);
    REQUIRE (library.metadataFor ("Raro", false).favorite);
    REQUIRE (library.metadataFor ("Copper Lead", true).category == PresetCategory::lead);
    REQUIRE (library.metadataFor ("Copper Lead", true).favorite);
}

TEST_CASE ("Reconcile adds new presets and flags orphans", "[preset][library]")
{
    const auto root = makeTempRoot ("library-reconcile");
    const auto presets = root.getChildFile ("Presets");
    presets.createDirectory();

    presets.getChildFile ("Nuevo.json").replaceWithText ("{}");
    presets.getChildFile ("Viejo.json").replaceWithText ("{}");

    PresetLibrary library (root.getChildFile ("library.json"));
    library.load();
    library.setCategory ("Viejo", false, PresetCategory::lead);

    library.reconcile (presets);

    REQUIRE_FALSE (library.metadataFor ("Nuevo", false).orphan);
    REQUIRE (library.metadataFor ("Nuevo", false).category == PresetCategory::other);
    REQUIRE_FALSE (library.metadataFor ("Viejo", false).orphan);
    REQUIRE (library.metadataFor ("Viejo", false).category == PresetCategory::lead);

    presets.getChildFile ("Viejo.json").deleteFile();
    library.reconcile (presets);

    REQUIRE (library.metadataFor ("Viejo", false).orphan);
    REQUIRE (library.metadataFor ("Viejo", false).category == PresetCategory::lead);
    REQUIRE_FALSE (library.metadataFor ("Nuevo", false).orphan);
}

TEST_CASE ("Rename and delete move or remove user entries", "[preset][library]")
{
    const auto root = makeTempRoot ("library-rename");
    PresetLibrary library (root.getChildFile ("library.json"));
    library.load();

    library.setCategory ("Viejo", false, PresetCategory::bass);
    library.setFavorite ("Viejo", false, true);
    library.renameUser ("Viejo", "Renombrado");

    REQUIRE (library.metadataFor ("Renombrado", false).category == PresetCategory::bass);
    REQUIRE (library.metadataFor ("Renombrado", false).favorite);
    REQUIRE (library.userEntries().count ("Viejo") == 0);

    library.removeUser ("Renombrado");
    REQUIRE (library.userEntries().empty());
}

TEST_CASE ("Saving twice replaces the sidecar without losing entries", "[preset][library]")
{
    const auto root = makeTempRoot ("library-atomic");
    const auto file = root.getChildFile ("library.json");

    PresetLibrary library (file);
    library.load();

    library.setFavorite ("Primero", false, true);
    REQUIRE (library.save());

    library.setFavorite ("Segundo", false, true);
    REQUIRE (library.save());

    PresetLibrary reloaded (file);
    reloaded.load();

    REQUIRE (reloaded.metadataFor ("Primero", false).favorite);
    REQUIRE (reloaded.metadataFor ("Segundo", false).favorite);

    // El var debe seguir vivo: getDynamicObject() devuelve un puntero al objeto
    // que posee, no una copia.
    const auto parsed = juce::JSON::parse (file);
    const auto rootObject = parsed.getDynamicObject();
    REQUIRE (rootObject != nullptr);
    REQUIRE (static_cast<int> (rootObject->getProperty ("version")) == 1);
}

TEST_CASE ("Library never rewrites preset JSON files", "[preset][library]")
{
    const auto root = makeTempRoot ("library-presets-untouched");
    const auto presets = root.getChildFile ("Presets");
    presets.createDirectory();

    const auto presetFile = presets.getChildFile ("Lead Propio.json");
    const juce::String content = R"({"name":"Lead Propio","parameters":{"osc.level":0.7}})";
    presetFile.replaceWithText (content);

    PresetLibrary library (root.getChildFile ("library.json"));
    library.load();
    library.reconcile (presets);
    library.setCategory ("Lead Propio", false, PresetCategory::lead);
    library.setFavorite ("Lead Propio", false, true);
    REQUIRE (library.save());

    REQUIRE (presetFile.loadFileAsString() == content);
}
