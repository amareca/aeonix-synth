#pragma once

#include <string>
#include <utility>
#include <vector>

#include "Parameter.h"

namespace aeonix
{

// ============================================================================
//  Patch — el "preset": una coleccion de Parameter (definicion + valor).
//
//  Guarda el conjunto de parametros del sintetizador y su valor normalizado
//  actual. Es un snapshot del estado del instrumento.
// ============================================================================
class Patch
{
public:
    Patch() = default;

    void add (Parameter p) { params_.push_back (std::move (p)); }

    // Devuelve true si el parametro existe.
    [[nodiscard]] bool contains (const std::string& id) const
    {
        return find (id) != nullptr;
    }

    // Valor normalizado de un parametro (0.0 si no existe).
    [[nodiscard]] float get (const std::string& id) const
    {
        const auto* p = find (id);
        return p != nullptr ? p->getNormalised() : 0.0f;
    }

    // Valor real (en el rango del parametro). Devuelve min si no existe.
    [[nodiscard]] float getValue (const std::string& id) const
    {
        const auto* p = find (id);
        return p != nullptr ? p->getValue() : p->getMin();
    }

    // Establece el valor normalizado. Devuelve false si el id no existe.
    bool set (const std::string& id, float normalised)
    {
        auto* p = find (id);
        if (p == nullptr)
            return false;

        p->setNormalised (normalised);
        return true;
    }

    [[nodiscard]] size_t size() const { return params_.size(); }

    // Acceso a todos los parametros (para serializar presets, etc.).
    [[nodiscard]] const std::vector<Parameter>& parameters() const { return params_; }

    [[nodiscard]] const Parameter* find (const std::string& id) const
    {
        for (const auto& p : params_)
            if (p.getId() == id)
                return &p;

        return nullptr;
    }

    // Version no-const: devuelve un puntero modificable (lo usa el motor para
    // cachear los punteros a los parametros).
    [[nodiscard]] Parameter* find (const std::string& id)
    {
        for (auto& p : params_)
            if (p.getId() == id)
                return &p;

        return nullptr;
    }

private:
    std::vector<Parameter> params_;
};

// Crea el Patch por defecto con los parametros usados en la Fase 1.
Patch makeDefaultPatch();

} // namespace aeonix
