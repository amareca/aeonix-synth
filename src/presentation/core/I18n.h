#pragma once

#include <juce_core/juce_core.h>

#include <initializer_list>

#include "presentation/core/Language.h"

namespace aeonix
{

// ============================================================================
//  I18n - catalogue of interface texts (Spanish / English).
//
//  TextId enumerates every visible string; the table lives in I18n.cpp and is
//  generated from StringCatalog.inc, so a forgotten translation does not
//  compile. tr() returns the text for the active language and trf() replaces
//  the {0}, {1}... placeholders.
//
//  The language is fixed at startup (Main.cpp) and changed in Ajustes; the
//  change is applied on restart, so there is no hot retranslation.
// ============================================================================
enum class TextId : int
{
#define AEONIX_TEXT(id, en, es) id,
#include "presentation/core/StringCatalog.inc"
#undef AEONIX_TEXT

    count
};

juce::String tr (TextId id);
juce::String trf (TextId id, std::initializer_list<juce::String> args);

void setLanguage (Language language);
Language getLanguage();

// Installs (or removes) the translations for the strings JUCE puts in the
// audio tab (AudioDeviceSelectorComponent). It is independent from our own
// catalogue: JUCE translates by English text.
void installJuceLocalisation (Language language);

} // namespace aeonix
