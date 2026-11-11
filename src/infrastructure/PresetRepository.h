#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

#include <map>
#include <string>
#include <utility>

#include "domain/Preset.h"

namespace aeonix
{

// ============================================================================
//  PresetRepository — guarda y carga presets en archivos JSON (infraestructura).
//
//  Los presets de fabrica viven en el dominio (makeFactoryPresets); aqui solo
//  se gestionan los presets de USUARIO (un archivo .json por preset).
// ============================================================================
class PresetRepository
{
public:
    explicit PresetRepository (juce::File directory)
        : directory_ (std::move (directory))
    {
        directory_.createDirectory();
    }

    // Nombres de los presets de usuario (sin extension), ordenados.
    [[nodiscard]] juce::StringArray listPresets() const
    {
        juce::StringArray names;
        for (const auto& f : directory_.findChildFiles (juce::File::findFiles, false, "*.json"))
            names.add (f.getFileNameWithoutExtension());

        names.sort (true);
        return names;
    }

    // Comprueba si existe un preset de usuario con ese nombre.
    [[nodiscard]] bool exists (const juce::String& name) const
    {
        return fileFor (name).existsAsFile();
    }

    // Borra un preset de usuario. Devuelve true si se borro.
    bool remove (const juce::String& name) const
    {
        return fileFor (name).deleteFile();
    }

    // Renombra un preset de usuario. Devuelve true si se movio.
    bool rename (const juce::String& oldName, const juce::String& newName) const
    {
        const juce::File source = fileFor (oldName);
        const juce::File target = fileFor (newName);

        if (! source.existsAsFile() || target.existsAsFile())
            return false;

        return source.moveFileTo (target);
    }

    // Carga un preset de usuario. Devuelve el mapa vacio si no existe.
    [[nodiscard]] std::map<std::string, float> load (const juce::String& name) const
    {
        std::map<std::string, float> params;

        const juce::File file = fileFor (name);
        if (!file.existsAsFile())
            return params;

        const auto root = juce::JSON::parse (file);
        if (auto* obj = root.getDynamicObject())
        {
            if (auto* paramsVar = obj->getProperty ("parameters").getDynamicObject())
            {
                for (const auto& kv : paramsVar->getProperties())
                    if (kv.value.isDouble() || kv.value.isInt())
                        params[kv.name.toString().toStdString()] = static_cast<float> (kv.value);
            }
        }

        return params;
    }

    // Guarda un preset de usuario.
    void save (const juce::String& name, const std::map<std::string, float>& params) const
    {
        juce::DynamicObject::Ptr obj = new juce::DynamicObject();
        obj->setProperty ("name", name);

        juce::DynamicObject::Ptr paramsObj = new juce::DynamicObject();
        for (const auto& [id, value] : params)
            paramsObj->setProperty (juce::String (id), value);

        obj->setProperty ("parameters", paramsObj.get());

        const juce::File file = fileFor (name);
        file.replaceWithText (juce::JSON::toString (obj.get()));
    }

    // Ruta de los presets de usuario (para abrirla desde la GUI).
    [[nodiscard]] const juce::File& getDirectory() const noexcept { return directory_; }

private:
    [[nodiscard]] juce::File fileFor (const juce::String& name) const
    {
        return directory_.getChildFile (juce::File::createLegalFileName (name) + ".json");
    }

    juce::File directory_;
};

} // namespace aeonix
