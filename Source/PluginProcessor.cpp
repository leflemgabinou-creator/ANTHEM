#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <algorithm>
#include <cmath>
#include <cstddef>

namespace
{
constexpr std::array<const char*, AnthemAudioProcessor::parameterCount> parameterIds {
    "masterLevel", "masterPan", "osc1Wave", "osc1Oct", "osc1Semi", "osc1Fine", "osc1Level", "osc1Pan", "osc1Unison", "osc1Detune",
    "osc2Wave", "osc2Oct", "osc2Semi", "osc2Fine", "osc2Level", "osc2Pan", "osc2Unison", "osc2Detune",
    "subLevel", "noiseLevel", "cutoff", "resonance", "filterDrive", "filterMode", "filterEnv",
    "ampAttack", "ampDecay", "ampSustain", "ampRelease", "filterAttack", "filterDecay", "filterSustain", "filterRelease",
    "lfoRate", "lfoDepth", "fxDrive", "fxChorus", "fxDelay", "fxReverb", "delayTime", "delayFeedback", "stereoWidth"
};

class WavetableBank
{
public:
    static constexpr int tableSize = 2048;
    static constexpr int bankCount = 11;

    WavetableBank()
    {
        for (int wave = 0; wave < 5; ++wave)
            for (int bank = 0; bank < bankCount; ++bank)
            {
                const int harmonics = juce::jmax (1, 1024 >> bank);
                auto& table = tables[(std::size_t) wave][(std::size_t) bank];
                float peak = 0.0f;
                for (int i = 0; i <= tableSize; ++i)
                {
                    const double phase = juce::MathConstants<double>::twoPi * (double) i / (double) tableSize;
                    double sum = 0.0;
                    for (int h = 1; h <= harmonics; ++h)
                    {
                        double amplitude = 0.0;
                        if (wave == 0) amplitude = h == 1 ? 1.0 : 0.0;
                        if (wave == 1 && (h & 1) != 0)
                        {
                            const int partial = (h - 1) / 2;
                            amplitude = ((partial & 1) == 0 ? 1.0 : -1.0) / ((double) h * (double) h);
                        }
                        if (wave == 2) amplitude = ((h & 1) != 0 ? 1.0 : -1.0) / (double) h;
                        if (wave == 3 && (h & 1) != 0) amplitude = 1.0 / (double) h;
                        if (wave == 4) amplitude = std::exp (-0.36 * (double) (h - 1)) / std::sqrt ((double) h);
                        if (amplitude != 0.0)
                        {
                            const double partialPhase = wave == 4 && h % 4 == 0 ? 0.22 : 0.0;
                            sum += amplitude * std::sin (phase * (double) h + partialPhase);
                        }
                    }
                    table[(std::size_t) i] = (float) sum;
                    peak = juce::jmax (peak, std::abs ((float) sum));
                }
                if (peak > 0.0001f)
                    for (auto& sample : table) sample /= peak;
            }
    }

    float sample (int wave, double phase, double frequency, double sampleRate) const
    {
        wave = juce::jlimit (0, 4, wave);
        const double allowedHarmonics = juce::jmax (1.0, (sampleRate * 0.47) / juce::jmax (1.0, frequency));
        const int bank = juce::jlimit (0, bankCount - 1, (int) std::ceil (std::log2 (1024.0 / allowedHarmonics)));
        const double position = phase * (double) tableSize;
        const int index = ((int) position) & (tableSize - 1);
        const float fraction = (float) (position - std::floor (position));
        const auto& table = tables[(std::size_t) wave][(std::size_t) bank];
        return table[(std::size_t) index] + (table[(std::size_t) index + 1] - table[(std::size_t) index]) * fraction;
    }

private:
    std::array<std::array<std::array<float, tableSize + 1>, bankCount>, 5> tables {};
};

const WavetableBank& getWavetableBank()
{
    static const WavetableBank bank;
    return bank;
}

struct TptLowPass
{
    struct Response { float low, band, high, notch; };
    double ic1 = 0.0, ic2 = 0.0;
    void reset() { ic1 = ic2 = 0.0; }
    Response process (float input, float cutoff, float resonance, double sampleRate)
    {
        const double frequency = juce::jlimit (25.0, sampleRate * 0.45, (double) cutoff);
        const double g = std::tan (juce::MathConstants<double>::pi * frequency / sampleRate);
        const double k = 2.0 - (double) resonance * 1.94;
        const double a1 = 1.0 / (1.0 + g * (g + k));
        const double a2 = g * a1;
        const double a3 = g * a2;
        const double v3 = (double) input - ic2;
        const double v1 = a1 * ic1 + a2 * v3;
        const double v2 = ic2 + a2 * ic1 + a3 * v3;
        ic1 = 2.0 * v1 - ic1;
        ic2 = 2.0 * v2 - ic2;
        return { (float) v2, (float) v1, (float) ((double) input - k * v1 - v2), (float) ((double) input - k * v1) };
    }
};

class AnthemVoice final : public juce::SynthesiserVoice
{
public:
    bool canPlaySound (juce::SynthesiserSound* sound) override { return dynamic_cast<juce::SynthesiserSound*> (sound) != nullptr; }
    void setCurrentPlaybackSampleRate (double rate) override
    {
        juce::SynthesiserVoice::setCurrentPlaybackSampleRate (rate);
        sampleRate = juce::jmax (1.0, rate);
        ampEnvelope.setSampleRate (sampleRate);
        filterEnvelope.setSampleRate (sampleRate);
    }

    void startNote (int midiNote, float velocity, juce::SynthesiserSound*, int) override
    {
        note = midiNote;
        noteVelocity = velocity;
        const double seed = (double) (midiNote * 37 + getCurrentlyPlayingChannel() * 13 + 97);
        for (int i = 0; i < 7; ++i)
        {
            phase1[(std::size_t) i] = std::fmod (seed * 0.037 + i * 0.173, 1.0);
            phase2[(std::size_t) i] = std::fmod (seed * 0.071 + i * 0.293 + 0.19, 1.0);
        }
        subPhase = std::fmod (seed * 0.113 + 0.37, 1.0);
        noiseState = 0x9137a5bdu ^ (uint32_t) (midiNote * 719 + getCurrentlyPlayingChannel() * 1543);
        if (noiseState == 0) noiseState = 1;
        filterL1.reset(); filterL2.reset(); filterR1.reset(); filterR2.reset();
        ampEnvelope.reset();
        filterEnvelope.reset();
        const auto p = owner.getParameterSnapshot();
        ampEnvelope.setParameters ({ p[AnthemAudioProcessor::ampAttack], p[AnthemAudioProcessor::ampDecay],
                                     p[AnthemAudioProcessor::ampSustain], p[AnthemAudioProcessor::ampRelease] });
        filterEnvelope.setParameters ({ p[AnthemAudioProcessor::filterAttack], p[AnthemAudioProcessor::filterDecay],
                                        p[AnthemAudioProcessor::filterSustain], p[AnthemAudioProcessor::filterRelease] });
        ampEnvelope.noteOn();
        filterEnvelope.noteOn();
        lfoPhase = 0.0;
    }

    void stopNote (float, bool allowTailOff) override
    {
        if (allowTailOff)
        {
            ampEnvelope.noteOff();
            filterEnvelope.noteOff();
        }
        else
        {
            ampEnvelope.reset();
            filterEnvelope.reset();
            clearCurrentNote();
        }
    }

    void pitchWheelMoved (int value) override
    {
        pitchBendSemitones = ((double) value - 8192.0) / 8192.0 * 2.0;
    }
    void controllerMoved (int, int) override {}

    void renderNextBlock (juce::AudioBuffer<float>& output, int start, int count) override
    {
        const auto p = owner.getParameterSnapshot();
        const auto& tables = getWavetableBank();
        const double baseFrequency = juce::MidiMessage::getMidiNoteInHertz (note) * std::pow (2.0, pitchBendSemitones / 12.0);
        const double lfoStep = p[AnthemAudioProcessor::lfoRate] / sampleRate;
        const int wave1 = juce::roundToInt (p[AnthemAudioProcessor::osc1Wave]);
        const int wave2 = juce::roundToInt (p[AnthemAudioProcessor::osc2Wave]);
        const int voices1 = juce::jlimit (1, 7, juce::roundToInt (p[AnthemAudioProcessor::osc1Unison]));
        const int voices2 = juce::jlimit (1, 7, juce::roundToInt (p[AnthemAudioProcessor::osc2Unison]));
        const float resonance = juce::jlimit (0.0f, 0.94f, p[AnthemAudioProcessor::resonance]);
        const int filterMode = juce::roundToInt (p[AnthemAudioProcessor::filterMode]);

        for (int s = 0; s < count; ++s)
        {
            if (! ampEnvelope.isActive()) { clearCurrentNote(); break; }
            const float amp = ampEnvelope.getNextSample();
            const float fEnv = filterEnvelope.getNextSample();
            const double vibrato = std::sin (lfoPhase * juce::MathConstants<double>::twoPi) * (double) p[AnthemAudioProcessor::lfoDepth] * 0.12;
            lfoPhase += lfoStep;
            if (lfoPhase >= 1.0) lfoPhase -= 1.0;

            double left = 0.0, right = 0.0;
            renderOscillator (tables, wave1, voices1, baseFrequency, p[AnthemAudioProcessor::osc1Oct], p[AnthemAudioProcessor::osc1Semi], p[AnthemAudioProcessor::osc1Fine], p[AnthemAudioProcessor::osc1Detune], p[AnthemAudioProcessor::osc1Level], p[AnthemAudioProcessor::osc1Pan], phase1, vibrato, left, right);
            renderOscillator (tables, wave2, voices2, baseFrequency, p[AnthemAudioProcessor::osc2Oct], p[AnthemAudioProcessor::osc2Semi], p[AnthemAudioProcessor::osc2Fine], p[AnthemAudioProcessor::osc2Detune], p[AnthemAudioProcessor::osc2Level], p[AnthemAudioProcessor::osc2Pan], phase2, vibrato, left, right);

            const double subFrequency = baseFrequency * 0.5;
            const float sub = tables.sample (0, subPhase, subFrequency, sampleRate) * p[AnthemAudioProcessor::subLevel];
            subPhase += subFrequency / sampleRate;
            if (subPhase >= 1.0) subPhase -= 1.0;
            const float noise = nextNoise() * p[AnthemAudioProcessor::noiseLevel];
            left += sub + noise;
            right += sub + noise;

            const float envelopeCutoff = p[AnthemAudioProcessor::cutoff] * std::pow (2.0f, fEnv * p[AnthemAudioProcessor::filterEnv] * 4.0f);
            const float drive = p[AnthemAudioProcessor::filterDrive];
            float inputL = std::tanh ((float) left * (1.0f + drive * 5.0f));
            float inputR = std::tanh ((float) right * (1.0f + drive * 5.0f));
            const auto stageL = filterL1.process (inputL, envelopeCutoff, resonance, sampleRate);
            const auto stageR = filterR1.process (inputR, envelopeCutoff, resonance, sampleRate);
            float filteredL = stageL.low, filteredR = stageR.low;
            if (filterMode == 0)
            {
                filteredL = filterL2.process (stageL.low, envelopeCutoff, resonance, sampleRate).low;
                filteredR = filterR2.process (stageR.low, envelopeCutoff, resonance, sampleRate).low;
            }
            else if (filterMode == 2) { filteredL = stageL.high; filteredR = stageR.high; }
            else if (filterMode == 3) { filteredL = stageL.band; filteredR = stageR.band; }
            else if (filterMode == 4) { filteredL = stageL.notch; filteredR = stageR.notch; }
            const float velocity = std::pow (noteVelocity, 0.72f);
            output.addSample (0, start + s, filteredL * amp * velocity);
            if (output.getNumChannels() > 1)
                output.addSample (1, start + s, filteredR * amp * velocity);
        }
    }

    explicit AnthemVoice (AnthemAudioProcessor& processor) : owner (processor) {}

private:
    void renderOscillator (const WavetableBank& tables, int wave, int count, double rootFrequency,
                          float octave, float semitone, float cents, float detune, float level, float pan,
                          std::array<double, 7>& phases, double vibrato, double& outL, double& outR)
    {
        if (level <= 0.00001f) return;
        const double base = rootFrequency * std::pow (2.0, ((double) octave * 12.0 + semitone + cents / 100.0 + vibrato) / 12.0);
        const double gain = (double) level / std::sqrt ((double) count);
        for (int i = 0; i < count; ++i)
        {
            const double spread = count == 1 ? 0.0 : ((double) i / (double) (count - 1) - 0.5);
            const double frequency = base * std::pow (2.0, spread * (double) detune / 1200.0);
            const double phase = phases[(std::size_t) i];
            const float sample = wave == 5 ? nextNoise() : tables.sample (wave, phase, frequency, sampleRate);
            phases[(std::size_t) i] += frequency / sampleRate;
            phases[(std::size_t) i] -= std::floor (phases[(std::size_t) i]);
            const double layerPan = juce::jlimit (-1.0, 1.0, (double) pan + spread * 1.35);
            const double angle = (layerPan + 1.0) * juce::MathConstants<double>::pi * 0.25;
            outL += (double) sample * gain * std::cos (angle) * 1.41421356237;
            outR += (double) sample * gain * std::sin (angle) * 1.41421356237;
        }
    }

    float nextNoise()
    {
        noiseState ^= noiseState << 13;
        noiseState ^= noiseState >> 17;
        noiseState ^= noiseState << 5;
        return (float) ((double) noiseState / 2147483648.0 - 1.0);
    }

    AnthemAudioProcessor& owner;
    int note = 60;
    float noteVelocity = 0.8f;
    double sampleRate = 44100.0, lfoPhase = 0.0;
    double subPhase = 0.0;
    double pitchBendSemitones = 0.0;
    uint32_t noiseState = 0x9137a5bdu;
    std::array<double, 7> phase1 {}, phase2 {};
    juce::ADSR ampEnvelope, filterEnvelope;
    TptLowPass filterL1, filterL2, filterR1, filterR2;
};

class AnthemSound final : public juce::SynthesiserSound
{
public:
    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

juce::String generatedName (const juce::String& style, juce::Random& rng)
{
    static const juce::StringArray starts { "Velvet", "Afterglow", "Nightfall", "Gold", "Blue", "Satin", "Metro", "Lowkey", "Chrome", "Late" };
    static const juce::StringArray ends { "Signal", "Dream", "Motion", "Run", "Season", "Memory", "Bounce", "Theory", "State", "Vision" };
    return starts[rng.nextInt (starts.size())] + " " + ends[rng.nextInt (ends.size())] + " · " + style;
}
}

AnthemAudioProcessor::AnthemAudioProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "ANTHEM_STATE", createParameterLayout())
{
    for (int i = 0; i < 16; ++i) synth.addVoice (new AnthemVoice (*this));
    synth.addSound (new AnthemSound());
    if (parameters.state.getProperty ("currentPreset").toString().isEmpty())
        loadPreset ("Nights Like This");
}

AnthemAudioProcessor::APVTS::ParameterLayout AnthemAudioProcessor::createParameterLayout()
{
    APVTS::ParameterLayout layout;
    const auto addFloat = [&layout] (const char* id, const char* name, float minimum, float maximum, float initial, float step = 0.0f)
    {
        layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { id, 1 }, name,
                    juce::NormalisableRange<float> (minimum, maximum, step), initial));
    };
    const juce::StringArray waves { "Sine", "Triangle", "Saw", "Pulse", "Grande", "Noise" };
    layout.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { "osc1Wave", 1 }, "OSC 1 Wave", waves, 2));
    addFloat ("masterLevel", "Master", 0.0f, 1.0f, 0.8f);
    addFloat ("masterPan", "Master Pan", -1.0f, 1.0f, 0.0f);
    addFloat ("osc1Oct", "OSC 1 Octave", -2.0f, 2.0f, 0.0f, 1.0f);
    addFloat ("osc1Semi", "OSC 1 Semitone", -12.0f, 12.0f, 0.0f, 1.0f);
    addFloat ("osc1Fine", "OSC 1 Fine", -100.0f, 100.0f, 0.0f);
    addFloat ("osc1Level", "OSC 1 Level", 0.0f, 1.0f, 0.62f);
    addFloat ("osc1Pan", "OSC 1 Pan", -1.0f, 1.0f, 0.0f);
    addFloat ("osc1Unison", "OSC 1 Unison", 1.0f, 7.0f, 1.0f, 1.0f);
    addFloat ("osc1Detune", "OSC 1 Detune", 0.0f, 50.0f, 7.0f);
    layout.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { "osc2Wave", 1 }, "OSC 2 Wave", waves, 1));
    addFloat ("osc2Oct", "OSC 2 Octave", -2.0f, 2.0f, 0.0f, 1.0f);
    addFloat ("osc2Semi", "OSC 2 Semitone", -12.0f, 12.0f, 0.0f, 1.0f);
    addFloat ("osc2Fine", "OSC 2 Fine", -100.0f, 100.0f, 0.0f);
    addFloat ("osc2Level", "OSC 2 Level", 0.0f, 1.0f, 0.25f);
    addFloat ("osc2Pan", "OSC 2 Pan", -1.0f, 1.0f, 0.0f);
    addFloat ("osc2Unison", "OSC 2 Unison", 1.0f, 7.0f, 1.0f, 1.0f);
    addFloat ("osc2Detune", "OSC 2 Detune", 0.0f, 50.0f, 5.0f);
    addFloat ("subLevel", "Sub Level", 0.0f, 1.0f, 0.12f);
    addFloat ("noiseLevel", "Noise Level", 0.0f, 0.5f, 0.0f);
    addFloat ("cutoff", "Filter Cutoff", 30.0f, 18000.0f, 10000.0f);
    addFloat ("resonance", "Filter Resonance", 0.0f, 0.92f, 0.12f);
    addFloat ("filterDrive", "Filter Drive", 0.0f, 1.0f, 0.05f);
    layout.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { "filterMode", 1 }, "Filter Mode",
                juce::StringArray { "Classic LP24", "Soft LP12", "Bright HP12", "Band Pass", "Notch" }, 0));
    addFloat ("filterEnv", "Filter Env Amount", 0.0f, 1.0f, 0.3f);
    addFloat ("ampAttack", "Amp Attack", 0.001f, 8.0f, 0.01f);
    addFloat ("ampDecay", "Amp Decay", 0.001f, 8.0f, 0.4f);
    addFloat ("ampSustain", "Amp Sustain", 0.0f, 1.0f, 0.72f);
    addFloat ("ampRelease", "Amp Release", 0.005f, 12.0f, 0.35f);
    addFloat ("filterAttack", "Filter Attack", 0.001f, 8.0f, 0.01f);
    addFloat ("filterDecay", "Filter Decay", 0.001f, 8.0f, 0.5f);
    addFloat ("filterSustain", "Filter Sustain", 0.0f, 1.0f, 0.2f);
    addFloat ("filterRelease", "Filter Release", 0.005f, 12.0f, 0.4f);
    addFloat ("lfoRate", "LFO Rate", 0.05f, 20.0f, 4.0f);
    addFloat ("lfoDepth", "LFO Depth", 0.0f, 1.0f, 0.0f);
    addFloat ("fxDrive", "Drive", 0.0f, 1.0f, 0.08f);
    addFloat ("fxChorus", "Chorus", 0.0f, 1.0f, 0.07f);
    addFloat ("fxDelay", "Delay", 0.0f, 1.0f, 0.1f);
    addFloat ("fxReverb", "Reverb", 0.0f, 1.0f, 0.12f);
    addFloat ("delayTime", "Delay Time", 0.04f, 1.0f, 0.375f);
    addFloat ("delayFeedback", "Delay Feedback", 0.0f, 0.88f, 0.34f);
    addFloat ("stereoWidth", "Stereo Width", 0.0f, 1.5f, 1.0f);
    return layout;
}

void AnthemAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    (void) getWavetableBank();
    currentSampleRate = sampleRate;
    synth.setCurrentPlaybackSampleRate (sampleRate);
    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) juce::jmax (1, samplesPerBlock), 2 };
    chorus.prepare (spec);
    chorus.reset();
    reverb.reset();
    delayBuffer[0].assign ((std::size_t) (sampleRate * 2.0), 0.0f);
    delayBuffer[1].assign ((std::size_t) (sampleRate * 2.0), 0.0f);
    delayWritePosition = 0;
}

void AnthemAudioProcessor::releaseResources()
{
    chorus.reset();
    reverb.reset();
    for (auto& channel : delayBuffer) std::fill (channel.begin(), channel.end(), 0.0f);
}

bool AnthemAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void AnthemAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
    keyboardState.processNextMidiBuffer (midi, 0, buffer.getNumSamples(), true);
    synth.renderNextBlock (buffer, midi, 0, buffer.getNumSamples());
    if (buffer.getNumChannels() < 2 || delayBuffer[0].empty()) return;

    const auto p = getParameterSnapshot();
    const int samples = buffer.getNumSamples();
    const std::size_t capacity = delayBuffer[0].size();
        const std::size_t delaySamples = juce::jlimit ((std::size_t) 1, capacity - 1, (std::size_t) (p[delayTime] * currentSampleRate));
    const float drive = p[fxDrive];
    const float driveGain = 1.0f + drive * 10.0f;
    const float driveMix = drive * 0.62f;
    const float norm = std::tanh (driveGain);

    for (int i = 0; i < samples; ++i)
    {
        const float dryL = buffer.getSample (0, i);
        const float dryR = buffer.getSample (1, i);
        const float drivenL = (1.0f - driveMix) * dryL + driveMix * std::tanh (dryL * driveGain) / norm;
        const float drivenR = (1.0f - driveMix) * dryR + driveMix * std::tanh (dryR * driveGain) / norm;
        const std::size_t readPosition = (delayWritePosition + capacity - delaySamples) % capacity;
        const float echoL = delayBuffer[0][readPosition];
        const float echoR = delayBuffer[1][readPosition];
        delayBuffer[0][delayWritePosition] = drivenL + echoR * p[delayFeedback];
        delayBuffer[1][delayWritePosition] = drivenR + echoL * p[delayFeedback];
        delayWritePosition = (delayWritePosition + 1) % capacity;
        const float wet = p[fxDelay];
        buffer.setSample (0, i, drivenL + echoL * wet * 0.55f);
        buffer.setSample (1, i, drivenR + echoR * wet * 0.55f);
    }

    if (p[fxChorus] > 0.001f)
    {
        chorus.setRate (0.18f + p[fxChorus] * 0.9f);
        chorus.setDepth (0.12f + p[fxChorus] * 0.5f);
        chorus.setCentreDelay (8.0f + p[fxChorus] * 10.0f);
        chorus.setFeedback (0.08f);
        chorus.setMix (p[fxChorus] * 0.5f);
        juce::dsp::AudioBlock<float> block (buffer);
        chorus.process (juce::dsp::ProcessContextReplacing<float> (block));
    }

    if (p[fxReverb] > 0.001f)
    {
        juce::dsp::Reverb::Parameters settings;
        settings.roomSize = 0.42f + p[fxReverb] * 0.52f;
        settings.damping = 0.58f;
        settings.wetLevel = p[fxReverb] * 0.5f;
        settings.dryLevel = 1.0f;
        settings.width = 0.85f + p[stereoWidth] * 0.1f;
        settings.freezeMode = 0.0f;
        reverb.setParameters (settings);
        juce::dsp::AudioBlock<float> block (buffer);
        reverb.process (juce::dsp::ProcessContextReplacing<float> (block));
    }

    const float master = p[masterLevel];
    const float width = p[stereoWidth];
    for (int i = 0; i < samples; ++i)
    {
        const float left = buffer.getSample (0, i), right = buffer.getSample (1, i);
        const float mid = 0.5f * (left + right), side = 0.5f * (left - right) * width;
        const float pan = p[masterPan];
        const float leftGain = pan > 0.0f ? 1.0f - pan : 1.0f;
        const float rightGain = pan < 0.0f ? 1.0f + pan : 1.0f;
        buffer.setSample (0, i, std::tanh ((mid + side) * master * leftGain));
        buffer.setSample (1, i, std::tanh ((mid - side) * master * rightGain));
    }
}

void AnthemAudioProcessor::loadPreset (const anthem::Preset& preset)
{
    for (const auto* id : parameterIds)
    {
        auto* parameter = parameters.getParameter (id);
        if (parameter != nullptr)
            parameter->setValueNotifyingHost (parameter->getDefaultValue());
    }
    for (const auto& value : preset.values)
        applyParameter (value.first, value.second);
    parameters.state.setProperty ("currentPreset", preset.name, nullptr);
    parameters.state.setProperty ("currentCategory", preset.category, nullptr);
}

void AnthemAudioProcessor::loadPreset (const juce::String& name)
{
    for (const auto& preset : getPresets())
        if (preset.name == name) { loadPreset (preset); return; }
}

void AnthemAudioProcessor::generatePreset (const juce::String& requestedStyle)
{
    const auto& bank = getPresets();
    juce::String style = requestedStyle;
    juce::String category = style;
    if (style == "Atlanta") category = "Atlanta";
    else if (style == "Dark") category = "Dark";
    else if (style == "Melodic") category = "Melodic";
    else if (style == "Bounce") category = "Bounce";
    else if (style == "Current") category = getCurrentCategory();
    else category = getCurrentCategory();

    std::vector<const anthem::Preset*> candidates;
    for (const auto& preset : bank)
        if (preset.category == category) candidates.push_back (&preset);
    if (candidates.empty())
        for (const auto& preset : bank) candidates.push_back (&preset);

    auto generated = *candidates[(std::size_t) random.nextInt ((int) candidates.size())];
    generated.name = generatedName (style == "Current" ? category : style, random);
    generated.description = "Generated from a curated " + category + " sound family.";
    const auto vary = [this, &generated] (const char* id, float amount, float minimum, float maximum)
    {
        float base = 0.0f;
        if (generated.values.count (id))
            base = generated.values[id];
        else if (auto* parameter = parameters.getParameter (id))
            base = parameter->convertFrom0to1 (parameter->getDefaultValue());
        const float offset = (random.nextFloat() * 2.0f - 1.0f) * amount;
        generated.values[id] = juce::jlimit (minimum, maximum, base + offset);
    };
    vary ("osc2Fine", 7.0f, -100.0f, 100.0f);
    vary ("osc2Level", 0.045f, 0.0f, 1.0f);
    const float cutoff = generated.values.count ("cutoff") ? generated.values["cutoff"] : 8000.0f;
    generated.values["cutoff"] = juce::jlimit (30.0f, 18000.0f, cutoff * (0.9f + random.nextFloat() * 0.2f));
    vary ("ampDecay", 0.14f, 0.001f, 8.0f);
    vary ("fxDelay", 0.05f, 0.0f, 1.0f);
    vary ("filterEnv", 0.08f, 0.0f, 1.0f);
    if (generated.category == "Pads" || generated.category == "Melodic")
    {
        vary ("osc1Detune", 2.5f, 0.0f, 50.0f);
        vary ("fxReverb", 0.06f, 0.0f, 1.0f);
        vary ("stereoWidth", 0.1f, 0.0f, 1.5f);
    }
    else if (generated.category == "808")
        vary ("fxDrive", 0.06f, 0.0f, 1.0f);
    else
    {
        vary ("fxReverb", 0.055f, 0.0f, 1.0f);
        vary ("fxChorus", 0.05f, 0.0f, 1.0f);
    }
    loadPreset (generated);
}

juce::String AnthemAudioProcessor::getCurrentPresetName() const
{
    return parameters.state.getProperty ("currentPreset", "Apex 808").toString();
}

juce::String AnthemAudioProcessor::getCurrentCategory() const
{
    return parameters.state.getProperty ("currentCategory", "Atlanta").toString();
}

AnthemAudioProcessor::ParameterSnapshot AnthemAudioProcessor::getParameterSnapshot() const
{
    ParameterSnapshot result {};
    for (std::size_t i = 0; i < parameterIds.size(); ++i)
        if (const auto* value = parameters.getRawParameterValue (parameterIds[i])) result[i] = value->load();
    return result;
}

void AnthemAudioProcessor::applyParameter (const juce::String& id, float value)
{
    if (auto* parameter = parameters.getParameter (id))
        parameter->setValueNotifyingHost (parameter->convertTo0to1 (value));
}

void AnthemAudioProcessor::getStateInformation (juce::MemoryBlock& destination)
{
    if (auto xml = parameters.copyState().createXml()) copyXmlToBinary (*xml, destination);
}

void AnthemAudioProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (parameters.state.getType())) parameters.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessorEditor* AnthemAudioProcessor::createEditor()
{
    return new AnthemAudioProcessorEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AnthemAudioProcessor();
}
