#pragma once

#include <juce_core/juce_core.h>

namespace aeonix
{

// ============================================================================
//  utf8 - converts a UTF-8 literal to juce::String.
//
//  The juce::String(const char*) constructor assumes ASCII: bytes > 127 are
//  interpreted as Latin-1 and accents come out as mojibake. For visible texts
//  we always use this helper.
// ============================================================================
inline juce::String utf8 (const char* text)
{
    return juce::String (juce::CharPointer_UTF8 (text));
}

} // namespace aeonix
