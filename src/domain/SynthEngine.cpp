#include "SynthEngine.h"

#include <algorithm>
#include <cmath>
#include <cstring>

#include "ParameterIds.h"

namespace aeonix
{

namespace
{

Waveform waveformFromIndex (int index)
{
    switch (index)
    {
        case 0:  return Waveform::Saw;
        case 1:  return Waveform::Square;
        case 2:  return Waveform::Sine;
        default: return Waveform::Triangle;
    }
}

// Resonancia normalizada (0..1) -> factor Q (0.707 Butterworth .. 8.0).
float resonanceToQ (float normalised)
{
    return 0.707f + normalised * (8.0f - 0.707f);
}

} // namespace

void SynthEngine::prepare (double sampleRate, int blockSize, int polyphony)
{
    // blockSize queda reservado para los buffers de efectos (Fase 4).
    (void) blockSize;

    sampleRate_ = sampleRate;

    const int capacity = std::clamp (polyphony, 1, maxVoiceCapacity);
    voiceCapacity_.store (capacity);
    voices_.assign (static_cast<size_t> (capacity), Voice {});

    for (auto& v : voices_)
        v.prepare (sampleRate);

    voiceLimit_.store (std::min (requestedPolyphony_.load(), capacity));

    // Resuelve los punteros a los parametros (una vez, fuera del hilo RT).
    pMasterVolume_ = patch_.find (param::masterVolume);
    pWaveform_     = patch_.find (param::oscWaveform);
    pOscLevel_     = patch_.find (param::oscLevel);
    pCutoff_       = patch_.find (param::filterCutoff);
    pResonance_    = patch_.find (param::filterResonance);
    pFilterEnvAmount_ = patch_.find (param::filterEnvAmount);
    pFilterAttack_ = patch_.find (param::filterAttack);
    pFilterDecay_  = patch_.find (param::filterDecay);
    pFilterSustain_ = patch_.find (param::filterSustain);
    pFilterRelease_ = patch_.find (param::filterRelease);
    pAttack_       = patch_.find (param::ampAttack);
    pDecay_        = patch_.find (param::ampDecay);
    pSustain_      = patch_.find (param::ampSustain);
    pRelease_      = patch_.find (param::ampRelease);
    pDelayTime_    = patch_.find (param::delayTime);
    pDelayFeedback_ = patch_.find (param::delayFeedback);
    pDelayMix_     = patch_.find (param::delayMix);
    pReverbSize_   = patch_.find (param::reverbSize);
    pReverbMix_    = patch_.find (param::reverbMix);
    pLfoRate_      = patch_.find (param::lfoRate);
    pLfoDepth_     = patch_.find (param::lfoDepth);
    pLfoWaveform_  = patch_.find (param::lfoWaveform);
    pUnisonVoices_ = patch_.find (param::unisonVoices);
    pUnisonDetune_ = patch_.find (param::unisonDetune);
    pUnisonWidth_  = patch_.find (param::unisonWidth);

    delay_.prepare (sampleRate);
    reverb_.prepare (sampleRate);
    lfo_.prepare (sampleRate);
    limiter_.prepare (sampleRate);

    applyParameters();
}

// --- Lado productor ---------------------------------------------------------

void SynthEngine::noteOn (int midiNote, float velocity)
{
    if (velocity <= 0.0f)
    {
        // Algunos teclados mandan NoteOn con velocity=0 en vez de NoteOff.
        pushNoteEvent ({ NoteEvent::Type::NoteOff, midiNote, 0.0f });
        return;
    }

    pushNoteEvent ({ NoteEvent::Type::NoteOn, midiNote, velocity });
}

void SynthEngine::noteOff (int midiNote)
{
    pushNoteEvent ({ NoteEvent::Type::NoteOff, midiNote, 0.0f });
}

void SynthEngine::allNotesOff()
{
    pushNoteEvent ({ NoteEvent::Type::AllNotesOff, 0, 0.0f });
}

void SynthEngine::setSustain (bool on)
{
    pushNoteEvent ({ on ? NoteEvent::Type::SustainOn
                        : NoteEvent::Type::SustainOff, 0, 0.0f });
}

void SynthEngine::setPitchBend (float value)
{
    NoteEvent e;
    e.type  = NoteEvent::Type::PitchBend;
    e.value = std::clamp (value, -1.0f, 1.0f);
    pushNoteEvent (e);
}

void SynthEngine::setModWheel (float value)
{
    NoteEvent e;
    e.type  = NoteEvent::Type::ModWheel;
    e.value = std::clamp (value, 0.0f, 1.0f);
    pushNoteEvent (e);
}

void SynthEngine::setParameter (const std::string& id, float normalised)
{
    ParamEvent e;
    std::strncpy (e.id, id.c_str(), sizeof (e.id) - 1);
    e.id[sizeof (e.id) - 1] = '\0';
    e.normalised = normalised;

    pushParamEvent (e);
}

void SynthEngine::pushNoteEvent (const NoteEvent& e)
{
    const std::lock_guard<std::mutex> lock (producerMutex_);
    noteEvents_.tryPush (e);
}

void SynthEngine::pushParamEvent (const ParamEvent& e)
{
    const std::lock_guard<std::mutex> lock (producerMutex_);
    paramEvents_.tryPush (e);
}

// --- Lado consumidor (hilo de audio) ----------------------------------------

void SynthEngine::render (float* left, float* right, int numSamples)
{
    processPendingEvents();

    // LFO: modulacion del cutoff a "control rate" (una vez por bloque).
    // La profundidad efectiva suma la base del preset y la rueda de modulacion.
    const float lfoDepth = effectiveLfoDepth (lfoDepth_, modWheel_);
    lfo_.setDepth (lfoDepth);
    lfo_.advance (numSamples);

    const float lfoValue = lfo_.getValue();   // [-depth, +depth]

    for (auto& v : voices_)
        v.setLfoModulation (lfoValue);

    std::fill (left,  left  + numSamples, 0.0f);
    std::fill (right, right + numSamples, 0.0f);

    for (auto& v : voices_)
        v.renderAdd (left, right, numSamples);

    for (int i = 0; i < numSamples; ++i)
    {
        left[i]  *= masterGain_;
        right[i] *= masterGain_;
    }

    // Efectos sobre el bus master.
    delay_.process (left, right, numSamples);
    reverb_.process (left, right, numSamples);

    // Seguridad final: evita que acordes grandes o FX al maximo clipen.
    limiter_.process (left, right, numSamples);
}

Voice* SynthEngine::findVoiceToSteal (int voiceLimit)
{
    Voice* released = nullptr;
    Voice* quietest = nullptr;
    Voice* oldest   = nullptr;

    for (int slot = 0; slot < voiceLimit; ++slot)
    {
        auto& v = voices_[static_cast<size_t> (slot)];

        if (! v.isActive())
            return &v;

        // 1) Una voz ya en release apenas se percibe: es la mejor candidata.
        if (v.isReleasing()
            && (released == nullptr || v.getNoteId() < released->getNoteId()))
            released = &v;

        // 2) Si no hay release, la de menor nivel de amplitud.
        if (quietest == nullptr || v.getAmpLevel() < quietest->getAmpLevel())
            quietest = &v;

        // 3) Desempate final: la mas antigua.
        if (oldest == nullptr || v.getNoteId() < oldest->getNoteId())
            oldest = &v;
    }

    if (released != nullptr)
        return released;

    return quietest != nullptr ? quietest : oldest;
}

void SynthEngine::setPolyphony (int voices)
{
    const int requested = std::clamp (voices, 1, maxVoiceCapacity);
    requestedPolyphony_.store (requested);
    voiceLimit_.store (std::min (requested, voiceCapacity_.load()));
}

int SynthEngine::getActiveVoiceCount() const
{
    int count = 0;
    for (const auto& v : voices_)
        if (v.isActive())
            ++count;

    return count;
}

int SynthEngine::getPolyphony() const { return voiceLimit_.load(); }

int SynthEngine::getSlotNoteNumber (int slot) const
{
    if (slot < 0 || slot >= static_cast<int> (voices_.size()))
        return -1;

    const auto& v = voices_[static_cast<size_t> (slot)];
    return v.isActive() ? v.getNoteNumber() : -1;
}

// --- internos ---------------------------------------------------------------

void SynthEngine::processPendingEvents()
{
    // 1) Parametros primero, para que las notas nuevas usen los valores nuevos.
    ParamEvent p;
    while (paramEvents_.tryPop (p))
        applyParamEvent (p);

    if (paramsDirty_)
    {
        applyParameters();
        paramsDirty_ = false;
    }

    // 2) Notas.
    NoteEvent n;
    while (noteEvents_.tryPop (n))
        applyNoteEvent (n);
}

void SynthEngine::applyNoteEvent (const NoteEvent& e)
{
    switch (e.type)
    {
        case NoteEvent::Type::NoteOn:
        {
            const std::uint64_t noteId = nextNoteId_++;
            const int voiceLimit = voiceLimit_.load();
            const int numUnison = std::clamp (unisonVoices_, 1, voiceLimit);

            for (int i = 0; i < numUnison; ++i)
            {
                Voice* voice = nullptr;

                for (int slot = 0; slot < voiceLimit; ++slot)
                {
                    auto& v = voices_[static_cast<size_t> (slot)];
                    if (! v.isActive()) { voice = &v; break; }
                }

                if (voice == nullptr)
                    voice = findVoiceToSteal (voiceLimit);

                if (voice == nullptr)
                    break;

                applyVoiceParams (*voice);

                double detuneCents = 0.0;
                if (numUnison > 1)
                {
                    const double step = unisonDetuneCents_ / (numUnison - 1);
                    detuneCents = -unisonDetuneCents_ * 0.5 + i * step;
                }

                voice->noteOn (e.noteNumber, e.velocity, noteId, detuneCents);
                voice->setPitchBend (pitchBend_);

                // Reparte el pan de las voces de unison de izquierda a derecha.
                float pan = 0.0f;
                if (numUnison > 1)
                    pan = unisonWidth_ * (-1.0f + 2.0f * static_cast<float> (i)
                                                      / static_cast<float> (numUnison - 1));

                voice->setPan (pan);
            }
            break;
        }

        case NoteEvent::Type::NoteOff:
        {
            for (auto& v : voices_)
            {
                if (v.isActive() && v.getNoteNumber() == e.noteNumber)
                {
                    if (sustain_)
                        v.holdNote();   // con pedal: la nota queda sostenida
                    else
                        v.noteOff();    // sin pedal: entra en release
                }
            }
            break;
        }

        case NoteEvent::Type::AllNotesOff:
        {
            for (auto& v : voices_)
                v.noteOff();
            break;
        }

        case NoteEvent::Type::SustainOn:
        {
            sustain_ = true;
            break;
        }

        case NoteEvent::Type::SustainOff:
        {
            if (sustain_)
            {
                sustain_ = false;

                for (auto& v : voices_)
                    if (v.isSustainHeld())
                        v.noteOff();
            }
            break;
        }

        case NoteEvent::Type::PitchBend:
        {
            pitchBend_ = e.value;

            for (auto& v : voices_)
                v.setPitchBend (pitchBend_);
            break;
        }

        case NoteEvent::Type::ModWheel:
        {
            modWheel_ = e.value;
            break;
        }
    }
}

void SynthEngine::applyParamEvent (const ParamEvent& e)
{
    Parameter* target = nullptr;

    if (std::strcmp (e.id, param::masterVolume) == 0)       target = pMasterVolume_;
    else if (std::strcmp (e.id, param::oscWaveform) == 0)   target = pWaveform_;
    else if (std::strcmp (e.id, param::oscLevel) == 0)      target = pOscLevel_;
    else if (std::strcmp (e.id, param::filterCutoff) == 0)  target = pCutoff_;
    else if (std::strcmp (e.id, param::filterResonance) == 0) target = pResonance_;
    else if (std::strcmp (e.id, param::filterEnvAmount) == 0) target = pFilterEnvAmount_;
    else if (std::strcmp (e.id, param::filterAttack) == 0)  target = pFilterAttack_;
    else if (std::strcmp (e.id, param::filterDecay) == 0)   target = pFilterDecay_;
    else if (std::strcmp (e.id, param::filterSustain) == 0) target = pFilterSustain_;
    else if (std::strcmp (e.id, param::filterRelease) == 0) target = pFilterRelease_;
    else if (std::strcmp (e.id, param::ampAttack) == 0)     target = pAttack_;
    else if (std::strcmp (e.id, param::ampDecay) == 0)      target = pDecay_;
    else if (std::strcmp (e.id, param::ampSustain) == 0)    target = pSustain_;
    else if (std::strcmp (e.id, param::ampRelease) == 0)    target = pRelease_;
    else if (std::strcmp (e.id, param::delayTime) == 0)     target = pDelayTime_;
    else if (std::strcmp (e.id, param::delayFeedback) == 0) target = pDelayFeedback_;
    else if (std::strcmp (e.id, param::delayMix) == 0)      target = pDelayMix_;
    else if (std::strcmp (e.id, param::reverbSize) == 0)    target = pReverbSize_;
    else if (std::strcmp (e.id, param::reverbMix) == 0)     target = pReverbMix_;
    else if (std::strcmp (e.id, param::lfoRate) == 0)       target = pLfoRate_;
    else if (std::strcmp (e.id, param::lfoDepth) == 0)      target = pLfoDepth_;
    else if (std::strcmp (e.id, param::lfoWaveform) == 0)   target = pLfoWaveform_;
    else if (std::strcmp (e.id, param::unisonVoices) == 0)  target = pUnisonVoices_;
    else if (std::strcmp (e.id, param::unisonDetune) == 0)  target = pUnisonDetune_;
    else if (std::strcmp (e.id, param::unisonWidth) == 0)   target = pUnisonWidth_;

    if (target != nullptr)
    {
        target->setNormalised (e.normalised);
        paramsDirty_ = true;
    }
}

void SynthEngine::applyParameters()
{
    if (pMasterVolume_ != nullptr)
        masterGain_ = pMasterVolume_->getValue();

    if (pDelayTime_ != nullptr)     delay_.setTime (pDelayTime_->getValue());
    if (pDelayFeedback_ != nullptr) delay_.setFeedback (pDelayFeedback_->getValue());
    if (pDelayMix_ != nullptr)      delay_.setMix (pDelayMix_->getValue());
    if (pReverbSize_ != nullptr)    reverb_.setRoomSize (pReverbSize_->getValue());
    if (pReverbMix_ != nullptr)     reverb_.setMix (pReverbMix_->getValue());

    if (pLfoRate_ != nullptr)
        lfo_.setRate (pLfoRate_->getValue());
    if (pLfoDepth_ != nullptr)
    {
        lfoDepth_ = pLfoDepth_->getValue();
        lfo_.setDepth (lfoDepth_);
    }
    if (pLfoWaveform_ != nullptr)
    {
        const int index = static_cast<int> (std::lround (pLfoWaveform_->getValue()));
        lfo_.setWaveform (waveformFromIndex (index));
    }
    if (pUnisonVoices_ != nullptr)
        unisonVoices_ = std::clamp (static_cast<int> (std::lround (pUnisonVoices_->getValue())), 1, 8);
    if (pUnisonDetune_ != nullptr)
        unisonDetuneCents_ = pUnisonDetune_->getValue();
    if (pUnisonWidth_ != nullptr)
        unisonWidth_ = pUnisonWidth_->getValue();

    for (auto& v : voices_)
        applyVoiceParams (v);
}

void SynthEngine::applyVoiceParams (Voice& v)
{
    if (pWaveform_ != nullptr)
    {
        const int index = static_cast<int> (std::lround (pWaveform_->getValue()));
        v.setWaveform (waveformFromIndex (index));
    }

    if (pOscLevel_ != nullptr)
        v.setOscLevel (pOscLevel_->getValue());

    if (pCutoff_ != nullptr)
        v.setFilterCutoff (pCutoff_->getValue());

    if (pResonance_ != nullptr)
        v.setFilterResonance (resonanceToQ (pResonance_->getNormalised()));

    if (pAttack_ != nullptr && pDecay_ != nullptr && pSustain_ != nullptr && pRelease_ != nullptr)
        v.setAmpEnvelope (pAttack_->getValue(),
                          pDecay_->getValue(),
                          pSustain_->getValue(),
                          pRelease_->getValue());

    if (pFilterAttack_ != nullptr && pFilterDecay_ != nullptr
        && pFilterSustain_ != nullptr && pFilterRelease_ != nullptr)
        v.setFilterEnvelope (pFilterAttack_->getValue(),
                             pFilterDecay_->getValue(),
                             pFilterSustain_->getValue(),
                             pFilterRelease_->getValue());

    if (pFilterEnvAmount_ != nullptr)
        v.setFilterEnvAmount (pFilterEnvAmount_->getValue());
}

} // namespace aeonix
