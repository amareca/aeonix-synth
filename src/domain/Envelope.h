#pragma once

namespace aeonix
{

// ============================================================================
//  Envelope — envolvente ADSR (Attack, Decay, Sustain, Release).
//
//  Maquina de estados:
//      Idle -> Attack -> Decay -> Sustain -> Release -> Idle
//
//  noteOn() arranca el ataque desde el nivel actual. En una voz nueva el nivel
//  es 0, asi que suena igual; al reciclar una voz (voice stealing) se evita el
//  salto brusco a cero que producia un clic.
//  noteOff() pasa a release (desde el valor actual).
//
//  La forma es LINEAL (suficiente para esta fase). nextSample() devuelve el
//  valor actual y avanza un paso. Sin asignaciones en caliente (RT-safe).
// ============================================================================
class Envelope
{
public:
    enum class Stage
    {
        Idle,
        Attack,
        Decay,
        Sustain,
        Release
    };

    Envelope() = default;

    void prepare (double sampleRate)
    {
        sampleRate_ = sampleRate;
        recomputeIncrements();
    }

    void noteOn()
    {
        // No se fuerza value_ = 0: al robar una voz en release conviene subir
        // desde su nivel actual para no introducir un escalon (clic).
        stage_ = Stage::Attack;
    }

    void noteOff()
    {
        if (stage_ == Stage::Idle)
            return;

        stage_ = Stage::Release;
    }

    // Valor actual y avanza un paso. Rango [0, 1].
    float nextSample()
    {
        switch (stage_)
        {
            case Stage::Attack:
                value_ += attackInc_;
                if (value_ >= 1.0)
                {
                    value_ = 1.0;
                    stage_ = Stage::Decay;
                }
                break;

            case Stage::Decay:
                value_ -= decayInc_;
                if (value_ <= sustain_)
                {
                    value_ = sustain_;
                    stage_ = Stage::Sustain;
                }
                break;

            case Stage::Sustain:
                break;

            case Stage::Release:
                value_ -= releaseInc_;
                if (value_ <= 0.0)
                {
                    value_ = 0.0;
                    stage_ = Stage::Idle;
                }
                break;

            case Stage::Idle:
                value_ = 0.0;
                break;
        }

        return static_cast<float> (value_);
    }

    [[nodiscard]] bool isActive() const noexcept { return stage_ != Stage::Idle; }
    [[nodiscard]] Stage getStage() const noexcept { return stage_; }
    [[nodiscard]] double getValue() const noexcept { return value_; }

    void setAttack (double seconds)  { attack_ = seconds;  recomputeIncrements(); }
    void setDecay (double seconds)   { decay_ = seconds;   recomputeIncrements(); }
    void setRelease (double seconds) { release_ = seconds; recomputeIncrements(); }
    void setSustain (double level)   { sustain_ = level;   recomputeIncrements(); }

    void reset()
    {
        value_ = 0.0;
        stage_ = Stage::Idle;
    }

private:
    void recomputeIncrements()
    {
        // Segundos -> incremento por muestra. 0 segundos = cambio instantaneo.
        auto toInc = [this] (double seconds)
        {
            if (seconds <= 0.0 || sampleRate_ <= 0.0)
                return 1e9;
            return 1.0 / (seconds * sampleRate_);
        };

        attackInc_  = toInc (attack_);
        decayInc_   = (sustain_ >= 1.0 || decay_ <= 0.0)
                        ? 1e9
                        : (1.0 - sustain_) / (decay_ * sampleRate_);
        releaseInc_ = toInc (release_);
    }

    double sampleRate_ = 44100.0;

    double attack_  = 0.001;
    double decay_   = 0.2;
    double sustain_ = 0.7;
    double release_ = 0.2;

    double attackInc_  = 0.0;
    double decayInc_   = 0.0;
    double releaseInc_ = 0.0;

    double value_ = 0.0;
    Stage stage_  = Stage::Idle;
};

} // namespace aeonix
