#pragma once

#include <initializer_list>

#include <juce_gui_basics/juce_gui_basics.h>

namespace aeonix
{

// ============================================================================
//  Utilidades de layout de las vistas de seccion.
//
//  El contenido de una seccion se reparte en celdas iguales; cada vista
//  coloca sus controles en el mismo orden que antes para conservar el
//  informe de layout F11.
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
