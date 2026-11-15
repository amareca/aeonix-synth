#pragma once

#include <initializer_list>

#include <juce_gui_basics/juce_gui_basics.h>

namespace aeonix
{

// ============================================================================
//  Layout helpers for the section views.
//
//  The content of a section is split into equal cells; each view places its
//  controls in the same order as before so the F11 layout report stays
//  unchanged.
// ============================================================================
inline int sectionCellWidth (const juce::Rectangle<int>& content, int cells)
{
    return content.getWidth() / juce::jmax (1, cells);
}

inline void layOutCells (juce::Rectangle<int> content, int cellWidth,
                         std::initializer_list<juce::Component*> controls)
{
    for (auto* control : controls)
        if (control != nullptr)
            control->setBounds (content.removeFromLeft (cellWidth));
}

} // namespace aeonix
