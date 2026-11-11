#pragma once

#include <juce_core/juce_core.h>

#include "presentation/core/Utf8.h"

namespace aeonix
{

// ============================================================================
//  Language — idiomas soportados por la interfaz.
//
//  Se guarda en config.json con la clave "ui.language" ("es" / "en"). Si la
//  clave no existe, la app detecta el idioma del sistema; el ingles es el
//  idioma de reserva.
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

// Nombre del idioma en su propia lengua (endonimo), como en cualquier selector
// de idioma: "Español" / "English".
inline juce::String languageDisplayName (Language language)
{
    return language == Language::spanish ? utf8 ("Español")
                                         : juce::String ("English");
}

// Detecta el idioma del sistema. Cualquier variante "es*" se trata como
// espanol de Espana (unico espanol soportado por ahora); el resto, ingles.
inline Language detectSystemLanguage()
{
    const auto code = juce::SystemStats::getUserLanguage();
    return code.startsWithIgnoreCase ("es") ? Language::spanish
                                            : Language::english;
}

} // namespace aeonix
