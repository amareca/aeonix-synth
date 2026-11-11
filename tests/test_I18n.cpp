#include <catch2/catch_test_macros.hpp>

#include "presentation/core/I18n.h"
#include "presentation/core/Language.h"
#include "presentation/core/Utf8.h"

using namespace aeonix;

TEST_CASE ("Every catalog entry has both translations", "[i18n]")
{
    for (int i = 0; i < static_cast<int> (TextId::count); ++i)
    {
        const auto id = static_cast<TextId> (i);

        setLanguage (Language::spanish);
        REQUIRE (tr (id).isNotEmpty());

        setLanguage (Language::english);
        REQUIRE (tr (id).isNotEmpty());
    }

    setLanguage (Language::english);
}

TEST_CASE ("tr returns the active language", "[i18n]")
{
    setLanguage (Language::spanish);
    REQUIRE (tr (TextId::saveButton) == utf8 ("Guardar"));
    REQUIRE (tr (TextId::deleteButton) == utf8 ("Borrar"));
    REQUIRE (tr (TextId::renameButton) == utf8 ("Renombrar"));
    REQUIRE (tr (TextId::sectionOscillator) == utf8 ("OSCILADOR"));

    setLanguage (Language::english);
    REQUIRE (tr (TextId::saveButton) == "Save");
    REQUIRE (tr (TextId::deleteButton) == "Delete");
    REQUIRE (tr (TextId::renameButton) == "Rename");
    REQUIRE (tr (TextId::sectionOscillator) == "OSCILLATOR");
}

TEST_CASE ("trf replaces numbered placeholders", "[i18n]")
{
    setLanguage (Language::spanish);
    REQUIRE (trf (TextId::toastPresetSaved, { "Mi preset" })
             == utf8 ("Preset guardado: Mi preset"));

    setLanguage (Language::english);
    REQUIRE (trf (TextId::dialogOverwriteMessage, { "Pad" })
             == utf8 ("\"Pad\" already exists. Overwrite it?"));
}

TEST_CASE ("Language keys round-trip and fall back to English", "[i18n]")
{
    REQUIRE (languageKey (Language::spanish) == "es");
    REQUIRE (languageKey (Language::english) == "en");

    REQUIRE (languageFromKey ("es") == Language::spanish);
    REQUIRE (languageFromKey ("es-ES") == Language::spanish);
    REQUIRE (languageFromKey ("en") == Language::english);
    REQUIRE (languageFromKey ("fr") == Language::english);
    REQUIRE (languageFromKey ("fr", Language::spanish) == Language::spanish);

    REQUIRE (languageDisplayName (Language::spanish) == utf8 ("Español"));
    REQUIRE (languageDisplayName (Language::english) == "English");
}
