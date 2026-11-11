#include "I18n.h"

#include <iterator>
#include <memory>

#include "presentation/core/Utf8.h"

namespace aeonix
{

namespace
{

struct TextEntry
{
    const char* es;
    const char* en;
};

constexpr TextEntry entries[] =
{
#define AEONIX_TEXT(id, es, en) { es, en },
#include "presentation/core/StringCatalog.inc"
#undef AEONIX_TEXT
};

static_assert (std::size (entries) == static_cast<size_t> (TextId::count),
               "StringCatalog.inc y TextId no coinciden");

Language currentLanguage = Language::english;

juce::String entryText (const TextEntry& entry, Language language)
{
    return utf8 (language == Language::spanish ? entry.es : entry.en);
}

// Cadenas que JUCE usa en AudioDeviceSelectorComponent. Las claves son ASCII
// en ingles; los valores van en UTF-8 y LocalisedStrings los interpreta bien.
const char* const juceSpanishDictionary = R"TEXT(language: Spanish
countries: es
"none" = "ninguno"
"Show advanced settings..." = "Mostrar ajustes avanzados..."
"Error when trying to open audio device!" = "¡No se pudo abrir el dispositivo de audio!"
"OK" = "OK"
"(no audio output channels found)" = "(no se encontraron canales de salida)"
"Active output channels:" = "Canales de salida activos:"
"(no audio input channels found)" = "(no se encontraron canales de entrada)"
"Active input channels:" = "Canales de entrada activos:"
"Control Panel" = "Panel de control"
"Opens the device's own control panel" = "Abre el panel de control propio del dispositivo"
"Reset Device" = "Reiniciar dispositivo"
"Resets the audio interface - sometimes needed after changing a device's properties in its custom control panel" = "Reinicia la interfaz de audio; a veces es necesario tras cambiar propiedades en su panel de control"
"Output:" = "Salida:"
"Device:" = "Dispositivo:"
"Test" = "Probar"
"Plays a test tone" = "Reproduce un tono de prueba"
"Input:" = "Entrada:"
"Sample rate:" = "Frecuencia de muestreo:"
"Audio buffer size:" = "Tamaño del búfer de audio:"
"Audio device type:" = "Tipo de dispositivo de audio:"
"No MIDI inputs available" = "No hay entradas MIDI disponibles"
"Active MIDI inputs:" = "Entradas MIDI activas:"
"Bluetooth MIDI" = "MIDI Bluetooth"
"Scan for bluetooth MIDI devices" = "Buscar dispositivos MIDI Bluetooth"
"MIDI Output:" = "Salida MIDI:"
)TEXT";

} // namespace

juce::String tr (TextId id)
{
    const auto index = static_cast<size_t> (id);

    if (index >= std::size (entries))
        return {};

    return entryText (entries[index], currentLanguage);
}

juce::String trf (TextId id, std::initializer_list<juce::String> args)
{
    auto text = tr (id);
    int index = 0;

    for (const auto& arg : args)
        text = text.replace ("{" + juce::String (index++) + "}", arg);

    return text;
}

void setLanguage (Language language)
{
    currentLanguage = language;
}

Language getLanguage()
{
    return currentLanguage;
}

void installJuceLocalisation (Language language)
{
    static std::unique_ptr<juce::LocalisedStrings> spanish;

    if (language == Language::spanish)
    {
        if (spanish == nullptr)
            spanish = std::make_unique<juce::LocalisedStrings> (
                juce::String (juce::CharPointer_UTF8 (juceSpanishDictionary)), false);

        juce::LocalisedStrings::setCurrentMappings (spanish.get());
    }
    else
    {
        juce::LocalisedStrings::setCurrentMappings (nullptr);
    }
}

} // namespace aeonix
