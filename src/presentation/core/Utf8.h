#pragma once

#include <juce_core/juce_core.h>

namespace aeonix
{

// ============================================================================
//  utf8 — convierte un literal UTF-8 a juce::String.
//
//  El constructor juce::String(const char*) asume ASCII: los bytes > 127 se
//  interpretan como Latin-1 y los acentos salen como mojibake. Para los textos
//  visibles usamos siempre este helper.
// ============================================================================
inline juce::String utf8 (const char* text)
{
    return juce::String (juce::CharPointer_UTF8 (text));
}

} // namespace aeonix
