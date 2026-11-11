#pragma once

#include <algorithm>
#include <cmath>
#include <string>
#include <utility>

namespace aeonix
{

// ============================================================================
//  Parameter — define un parametro del sintetizador.
//
//  - Tiene un id unico (texto), un nombre legible y un rango [min, max].
//  - Internamente guarda un valor NORMALIZADO en [0, 1]. getValue() lo convierte
//    al rango real.
//  - Soporta mapeo LOGARITMICO (para frecuencias y tiempos), que hace que un
//    slider 0..1 sea intuitivo cuando el rango abarca varias decadas.
//
//  Es un objeto de valor: sin logica de audio, sin dependencias.
// ============================================================================
class Parameter
{
public:
    Parameter (std::string id,
               std::string name,
               float min,
               float max,
               float defaultNormalised = 0.0f,
               bool logarithmic = false)
        : id_ (std::move (id)),
          name_ (std::move (name)),
          min_ (min),
          max_ (max),
          logarithmic_ (logarithmic),
          defaultNormalised_ (clamp01 (defaultNormalised)),
          normalised_ (defaultNormalised_)
    {
    }

    [[nodiscard]] const std::string& getId()   const noexcept { return id_; }
    [[nodiscard]] const std::string& getName() const noexcept { return name_; }
    [[nodiscard]] float getMin()               const noexcept { return min_; }
    [[nodiscard]] float getMax()               const noexcept { return max_; }
    [[nodiscard]] bool  isLogarithmic()        const noexcept { return logarithmic_; }
    [[nodiscard]] float getDefaultNormalised() const noexcept { return defaultNormalised_; }

    // Valor normalizado (0..1).
    [[nodiscard]] float getNormalised() const noexcept { return normalised_; }
    void setNormalised (float n) noexcept { normalised_ = clamp01 (n); }

    // Valor real (en el rango [min, max]).
    [[nodiscard]] float getValue() const noexcept
    {
        if (logarithmic_)
            return min_ * std::pow (max_ / min_, normalised_);

        return min_ + normalised_ * (max_ - min_);
    }

    void setValue (float v) noexcept
    {
        v = std::clamp (v, min_, max_);

        if (max_ == min_ || (logarithmic_ && min_ <= 0.0f))
        {
            normalised_ = 0.0f;
            return;
        }

        if (logarithmic_)
            normalised_ = std::log (v / min_) / std::log (max_ / min_);
        else
            normalised_ = (v - min_) / (max_ - min_);
    }

private:
    static float clamp01 (float x) noexcept { return std::clamp (x, 0.0f, 1.0f); }

    std::string id_;
    std::string name_;
    float min_;
    float max_;
    bool logarithmic_;
    float defaultNormalised_;
    float normalised_;
};

} // namespace aeonix
