#pragma once

#include <juce_core/juce_core.h>

#include "presentation/core/Utf8.h"

namespace aeonix
{

// ============================================================================
//  Language - languages supported by the interface.
//
//  Stored in config.json under the key "ui.language" ("es" / "en"). If the
//  key does not exist, the app detects the system language; English is the
//  fallback language.
// ============================================================================
enum class Language
{
    spanish,
    english
};

inline juce::String languageKey (Language language)
{
    return language == Language::spanish ? "es" : "en";
}

inline Language languageFromKey (const juce::String& key,
                                 Language fallback = Language::english)
{
    if (key.startsWithIgnoreCase ("es"))
        return Language::spanish;

    if (key.startsWithIgnoreCase ("en"))
        return Language::english;

    return fallback;
}

// Language name in its own tongue (endonym), as in any language selector:
// "Español" / "English".
inline juce::String languageDisplayName (Language language)
{
    return language == Language::spanish ? utf8 ("Español")
                                         : juce::String ("English");
}

// Detects the system language. Any "es*" variant is treated as
// Spanish from Spain (the only Spanish supported for now); the rest, English.
inline Language detectSystemLanguage()
{
    const auto code = juce::SystemStats::getUserLanguage();
    return code.startsWithIgnoreCase ("es") ? Language::spanish
                                            : Language::english;
}

} // namespace aeonix
