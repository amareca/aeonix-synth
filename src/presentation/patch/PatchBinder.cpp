#include "PatchBinder.h"

#include <cmath>

#include "domain/Preset.h"

namespace aeonix
{

std::map<std::string, float> PatchBinder::capture() const
{
    std::map<std::string, float> p;

    p["master.volume"]     = models_.synth.masterVolume.get();
    p["osc.waveform"]      = models_.osc.waveform.get() / 3.0f;
    p["osc.level"]         = models_.osc.level.get();
    p["filter.cutoff"]     = models_.filter.cutoff.get();
    p["filter.resonance"]  = models_.filter.resonance.get();
    p["filter.envAmount"]  = models_.filterEnv.amount.get();
    p["filter.attack"]     = models_.filterEnv.attack.get();
    p["filter.decay"]      = models_.filterEnv.decay.get();
    p["filter.sustain"]    = models_.filterEnv.sustain.get();
    p["filter.release"]    = models_.filterEnv.release.get();
    p["amp.attack"]        = models_.amp.attack.get();
    p["amp.decay"]         = models_.amp.decay.get();
    p["amp.sustain"]       = models_.amp.sustain.get();
    p["amp.release"]       = models_.amp.release.get();
    p["fx.delay.time"]     = models_.fx.delayTime.get();
    p["fx.delay.feedback"] = models_.fx.delayFeedback.get();
    p["fx.delay.mix"]      = models_.fx.delayMix.get();
    p["fx.reverb.size"]    = models_.fx.reverbSize.get();
    p["fx.reverb.mix"]     = models_.fx.reverbMix.get();
    p["lfo.rate"]          = models_.lfo.rate.get();
    p["lfo.depth"]         = models_.lfo.depth.get();
    p["lfo.waveform"]      = models_.lfo.waveform.get() / 3.0f;
    p["unison.voices"]     = (models_.unison.voices.get() - 1) / 7.0f;
    p["unison.detune"]     = models_.unison.detune.get();
    p["unison.width"]      = models_.unison.width.get();

    return p;
}

void PatchBinder::apply (const std::map<std::string, float>& p)
{
    // Parte de los valores por defecto: los presets antiguos (guardados antes
    // de anadir parametros nuevos) quedan completos y predecibles.
    std::map<std::string, float> values = makeDefaultParameters();
    for (const auto& [key, value] : p)
        values[key] = value;

    auto apply = [&values] (const char* key, const std::function<void (float)>& setter)
    {
        const auto it = values.find (key);
        if (it != values.end())
            setter (it->second);
    };

    apply ("master.volume",     [this] (float n) { models_.synth.setMasterVolume (n); });
    apply ("osc.level",         [this] (float n) { models_.osc.setLevel (n); });
    apply ("filter.cutoff",     [this] (float n) { models_.filter.setCutoff (n); });
    apply ("filter.resonance",  [this] (float n) { models_.filter.setResonance (n); });
    apply ("filter.envAmount",  [this] (float n) { models_.filterEnv.setAmount (n); });
    apply ("filter.attack",     [this] (float n) { models_.filterEnv.setAttack (n); });
    apply ("filter.decay",      [this] (float n) { models_.filterEnv.setDecay (n); });
    apply ("filter.sustain",    [this] (float n) { models_.filterEnv.setSustain (n); });
    apply ("filter.release",    [this] (float n) { models_.filterEnv.setRelease (n); });
    apply ("amp.attack",        [this] (float n) { models_.amp.setAttack (n); });
    apply ("amp.decay",         [this] (float n) { models_.amp.setDecay (n); });
    apply ("amp.sustain",       [this] (float n) { models_.amp.setSustain (n); });
    apply ("amp.release",       [this] (float n) { models_.amp.setRelease (n); });
    apply ("fx.delay.time",     [this] (float n) { models_.fx.setDelayTime (n); });
    apply ("fx.delay.feedback", [this] (float n) { models_.fx.setDelayFeedback (n); });
    apply ("fx.delay.mix",      [this] (float n) { models_.fx.setDelayMix (n); });
    apply ("fx.reverb.size",    [this] (float n) { models_.fx.setReverbSize (n); });
    apply ("fx.reverb.mix",     [this] (float n) { models_.fx.setReverbMix (n); });
    apply ("lfo.rate",          [this] (float n) { models_.lfo.setRate (n); });
    apply ("lfo.depth",         [this] (float n) { models_.lfo.setDepth (n); });
    apply ("unison.detune",     [this] (float n) { models_.unison.setDetune (n); });
    apply ("unison.width",      [this] (float n) { models_.unison.setWidth (n); });

    const auto rounded = [] (float value)
    {
        return static_cast<int> (std::lround (value));
    };

    const auto oscWaveIt = values.find ("osc.waveform");
    if (oscWaveIt != values.end())
        models_.osc.setWaveform (rounded (oscWaveIt->second * 3.0f));

    const auto lfoWaveIt = values.find ("lfo.waveform");
    if (lfoWaveIt != values.end())
        models_.lfo.setWaveform (rounded (lfoWaveIt->second * 3.0f));

    const auto unisonIt = values.find ("unison.voices");
    if (unisonIt != values.end())
        models_.unison.setVoices (1 + rounded (unisonIt->second * 7.0f));
}

} // namespace aeonix
