#pragma once

#include <juce_core/juce_core.h>

#include "presentation/core/BuildFlavour.h"

namespace aeonix
{

// ============================================================================
//  ConfigManager — persistencia de ajustes de la aplicacion (infraestructura).
//
//  Guarda un JSON pequeno en %APPDATA%\Aeonix\config.json con lo que debe
//  recordarse entre sesiones: dispositivo MIDI, estado de audio, octava del
//  teclado del ordenador, ultimo preset y geometria de la ventana.
//
//  Se usa solo desde el hilo de mensajes (nunca desde el hilo de audio).
// ============================================================================
class ConfigManager
{
public:
    ConfigManager()
        : file_ (defaultFile())
    {
        load();
    }

    // Instancia unica compartida por toda la app: evita que dos objetos con
    // snapshots distintos se sobrescriban al guardar.
    static ConfigManager& getInstance()
    {
        static ConfigManager instance;
        return instance;
    }

    [[nodiscard]] static juce::File defaultFile()
    {
        return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                   .getChildFile (build::dataFolder)
                   .getChildFile (build::configFile);
    }

    [[nodiscard]] juce::String getString (const juce::String& key,
                                          const juce::String& fallback = {}) const
    {
        const auto value = get (key);
        return value.isString() ? value.toString() : fallback;
    }

    void setString (const juce::String& key, const juce::String& value)
    {
        set (key, value);
    }

    [[nodiscard]] int getInt (const juce::String& key, int fallback = 0) const
    {
        const auto value = get (key);
        return (value.isInt() || value.isDouble()) ? static_cast<int> (value) : fallback;
    }

    void setInt (const juce::String& key, int value)
    {
        set (key, value);
    }

    // Guarda el JSON en disco (crea el directorio si hace falta).
    void save() const
    {
        file_.getParentDirectory().createDirectory();
        file_.replaceWithText (juce::JSON::toString (root_, true));
    }

private:
    [[nodiscard]] juce::var get (const juce::String& key) const
    {
        if (auto* obj = root_.getDynamicObject())
            return obj->getProperty (key);

        return {};
    }

    void set (const juce::String& key, const juce::var& value)
    {
        if (auto* obj = root_.getDynamicObject())
            obj->setProperty (key, value);
    }

    void load()
    {
        if (file_.existsAsFile())
            root_ = juce::JSON::parse (file_);

        if (root_.getDynamicObject() == nullptr)
            root_ = juce::var (new juce::DynamicObject());
    }

    juce::File file_;
    juce::var root_;
};

} // namespace aeonix
