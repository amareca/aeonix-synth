#pragma once

#include <juce_core/juce_core.h>

#include <initializer_list>

#include "presentation/core/Language.h"

namespace aeonix
{

// ============================================================================
//  I18n — catalogo de textos de la interfaz (espanol / ingles).
//
//  TextId enumera todas las cadenas visibles; la tabla vive en I18n.cpp y se
//  genera desde StringCatalog.inc, de modo que una traduccion olvidada no
//  compila. tr() devuelve el texto del idioma activo y trf() sustituye los
//  marcadores {0}, {1}...
//
//  El idioma se fija al arrancar (Main.cpp) y se cambia en Ajustes; el cambio
//  se aplica al reiniciar, asi que no hay retraduccion en caliente.
// ============================================================================
enum class TextId : int
{
#define AEONIX_TEXT(id, es, en) id,
#include "presentation/core/StringCatalog.inc"
#undef AEONIX_TEXT

    count
};

juce::String tr (TextId id);
juce::String trf (TextId id, std::initializer_list<juce::String> args);

void setLanguage (Language language);
Language getLanguage();

// Instala (o retira) las traducciones de las cadenas que pone JUCE en la
// pestana de audio (AudioDeviceSelectorComponent). Es independiente del
// catalogo propio: JUCE traduce por texto en ingles.
void installJuceLocalisation (Language language);

} // namespace aeonix
