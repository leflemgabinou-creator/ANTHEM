#pragma once

#include <JuceHeader.h>
#include "PresetBank.h"
#include <array>
#include <cstddef>
#include <random>

class AnthemAudioProcessor final : public juce::AudioProcessor
{
public:
    AnthemAudioProcessor();
    ~AnthemAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "ANTHEM"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 8.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}
    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    using APVTS = juce::AudioProcessorValueTreeState;
    APVTS parameters;
    juce::MidiKeyboardState keyboardState;
    static APVTS::ParameterLayout createParameterLayout();

    const std::vector<anthem::Preset>& getPresets() const { return anthem::getFactoryPresets(); }
    void loadPreset (const anthem::Preset& preset);
    void loadPreset (const juce::String& name);
    void generatePreset (const juce::String& style);
    juce::String getCurrentPresetName() const;
    juce::String getCurrentCategory() const;
    enum ParamIndex : std::size_t
    {
        masterLevel, masterPan, osc1Wave, osc1Oct, osc1Semi, osc1Fine, osc1Level, osc1Pan, osc1Unison, osc1Detune,
        osc2Wave, osc2Oct, osc2Semi, osc2Fine, osc2Level, osc2Pan, osc2Unison, osc2Detune,
        subLevel, noiseLevel, cutoff, resonance, filterDrive, filterMode, filterEnv,
        ampAttack, ampDecay, ampSustain, ampRelease,
        filterAttack, filterDecay, filterSustain, filterRelease,
        lfoRate, lfoDepth, fxDrive, fxChorus, fxDelay, fxReverb, delayTime, delayFeedback, stereoWidth,
        parameterCount
    };
    using ParameterSnapshot = std::array<float, parameterCount>;
    ParameterSnapshot getParameterSnapshot() const;

private:
    juce::Synthesiser synth;
    juce::dsp::Chorus<float> chorus;
    juce::dsp::Reverb reverb;
    std::array<std::vector<float>, 2> delayBuffer;
    std::size_t delayWritePosition = 0;
    double currentSampleRate = 44100.0;
    juce::Random random;

    void applyParameter (const juce::String& id, float value);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnthemAudioProcessor)
};
