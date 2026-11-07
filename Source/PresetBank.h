#pragma once

#include <juce_core/juce_core.h>
#include <map>
#include <vector>

namespace anthem
{
struct Preset
{
    juce::String name;
    juce::String category;
    juce::String description;
    std::map<juce::String, float> values;
};

inline Preset patch (const char* name, const char* category, const char* description,
                     std::initializer_list<std::pair<const char*, float>> settings)
{
    Preset p { name, category, description, {} };
    for (const auto& setting : settings)
        p.values.emplace (setting.first, setting.second);
    return p;
}

inline std::vector<Preset> factoryPresets()
{
    using namespace juce;
    return {
        patch ("Apex 808", "808", "Clean mono sub with a short, controlled knock.", {{"osc1Wave",0},{"osc1Oct",-1},{"osc1Level",0.28f},{"osc2Level",0.04f},{"subLevel",0.82f},{"cutoff",520},{"resonance",0.08f},{"ampAttack",0.002f},{"ampDecay",0.44f},{"ampSustain",0.32f},{"ampRelease",0.2f},{"filterEnv",0.45f},{"fxDrive",0.34f},{"fxReverb",0.02f}}),
        patch ("Southside Glide", "808", "Bright, driven low end with a tight knock and long sustain.", {{"osc1Wave",2},{"osc1Oct",-1},{"osc1Level",0.38f},{"subLevel",0.72f},{"cutoff",740},{"resonance",0.12f},{"ampDecay",0.78f},{"ampSustain",0.47f},{"ampRelease",0.28f},{"filterEnv",0.52f},{"fxDrive",0.48f},{"stereoWidth",0.2f}}),
        patch ("Casket Sub", "808", "Deep round sine, restrained upper harmonics.", {{"osc1Wave",0},{"osc1Oct",-1},{"osc1Level",0.18f},{"subLevel",0.92f},{"cutoff",250},{"ampDecay",0.62f},{"ampSustain",0.44f},{"ampRelease",0.22f},{"fxDrive",0.2f},{"fxReverb",0.0f}}),
        patch ("Riot Knock", "808", "Aggressive distorted 808 with a percussive filter snap.", {{"osc1Wave",3},{"osc1Oct",-1},{"osc1Level",0.62f},{"subLevel",0.46f},{"cutoff",1250},{"resonance",0.2f},{"ampDecay",0.72f},{"ampSustain",0.55f},{"ampRelease",0.25f},{"filterEnv",0.65f},{"fxDrive",0.72f}}),
        patch ("Peachtree 808", "808", "Warm, centered low end with a gentle tail.", {{"osc1Wave",1},{"osc1Oct",-1},{"osc1Level",0.35f},{"subLevel",0.78f},{"cutoff",460},{"ampDecay",0.85f},{"ampSustain",0.38f},{"ampRelease",0.3f},{"fxDrive",0.3f},{"fxReverb",0.03f}}),
        patch ("Chrome Bell", "Bells", "Wide, glassy FM-like bell impression from additive partials.", {{"osc1Wave",4},{"osc1Oct",1},{"osc1Level",0.68f},{"osc2Wave",1},{"osc2Oct",2},{"osc2Fine",7},{"osc2Level",0.28f},{"subLevel",0.0f},{"cutoff",9800},{"ampAttack",0.004f},{"ampDecay",2.4f},{"ampSustain",0.0f},{"ampRelease",2.8f},{"fxDelay",0.25f},{"delayTime",0.39f},{"fxReverb",0.3f},{"stereoWidth",1.0f}}),
        patch ("Rainmaker", "Bells", "Soft, descending bell tone with a long stereo bloom.", {{"osc1Wave",4},{"osc1Oct",1},{"osc1Level",0.56f},{"osc2Wave",0},{"osc2Oct",2},{"osc2Fine",-9},{"osc2Level",0.23f},{"cutoff",7200},{"ampDecay",3.2f},{"ampSustain",0.0f},{"ampRelease",4.0f},{"fxDelay",0.3f},{"delayTime",0.52f},{"fxReverb",0.42f},{"stereoWidth",1.2f}}),
        patch ("Halo Bells", "Bells", "Airy, layered bell chord for melodic hooks.", {{"osc1Wave",4},{"osc1Oct",1},{"osc1Unison",3},{"osc1Detune",9},{"osc1Level",0.58f},{"osc2Wave",1},{"osc2Oct",1},{"osc2Semi",7},{"osc2Level",0.2f},{"cutoff",6400},{"ampDecay",2.2f},{"ampSustain",0.1f},{"ampRelease",3.2f},{"fxChorus",0.18f},{"fxDelay",0.2f},{"fxReverb",0.32f},{"stereoWidth",1.25f}}),
        patch ("Velvet Keys", "Keys", "Rounded electric key with a lightly chorused body.", {{"osc1Wave",1},{"osc1Level",0.55f},{"osc2Wave",0},{"osc2Oct",-1},{"osc2Level",0.18f},{"cutoff",3100},{"resonance",0.1f},{"ampAttack",0.008f},{"ampDecay",0.8f},{"ampSustain",0.46f},{"ampRelease",0.72f},{"fxChorus",0.17f},{"fxReverb",0.18f}}),
        patch ("After Hours EP", "Keys", "Dusky electric piano color with a mellow top.", {{"osc1Wave",1},{"osc1Level",0.5f},{"osc2Wave",4},{"osc2Oct",1},{"osc2Level",0.16f},{"cutoff",2250},{"ampDecay",1.2f},{"ampSustain",0.3f},{"ampRelease",0.9f},{"fxChorus",0.22f},{"fxReverb",0.2f}}),
        patch ("Noir Organ", "Keys", "Stable organ-like tone for sustained chords.", {{"osc1Wave",2},{"osc1Level",0.48f},{"osc2Wave",2},{"osc2Oct",1},{"osc2Level",0.19f},{"osc2Pan",0.4f},{"cutoff",5600},{"ampAttack",0.01f},{"ampDecay",0.08f},{"ampSustain",0.92f},{"ampRelease",0.22f},{"fxChorus",0.12f},{"fxReverb",0.12f}}),
        patch ("Motel Grand", "Keys", "Bright grand-style pluck with a soft release.", {{"osc1Wave",4},{"osc1Level",0.62f},{"osc2Wave",1},{"osc2Oct",-1},{"osc2Level",0.16f},{"cutoff",4900},{"ampDecay",1.05f},{"ampSustain",0.13f},{"ampRelease",0.84f},{"fxReverb",0.16f}}),
        patch ("Blue Hour", "Pads", "Wide, slow analog pad with gentle pitch motion.", {{"osc1Wave",2},{"osc1Unison",5},{"osc1Detune",14},{"osc1Level",0.52f},{"osc2Wave",1},{"osc2Oct",-1},{"osc2Unison",3},{"osc2Detune",11},{"osc2Level",0.29f},{"cutoff",2600},{"ampAttack",1.1f},{"ampDecay",1.8f},{"ampSustain",0.78f},{"ampRelease",3.4f},{"lfoRate",0.18f},{"lfoDepth",0.08f},{"fxChorus",0.28f},{"fxReverb",0.36f},{"stereoWidth",1.3f}}),
        patch ("Midnight Choir", "Pads", "Soft, vocal-like layered pad with a hazy tail.", {{"osc1Wave",4},{"osc1Unison",4},{"osc1Detune",8},{"osc1Level",0.46f},{"osc2Wave",1},{"osc2Semi",7},{"osc2Level",0.23f},{"cutoff",3400},{"ampAttack",0.7f},{"ampDecay",1.6f},{"ampSustain",0.75f},{"ampRelease",4.2f},{"fxChorus",0.34f},{"fxReverb",0.4f},{"stereoWidth",1.4f}}),
        patch ("Glass Cathedral", "Pads", "Luminous, slow pad with long stereo reflections.", {{"osc1Wave",4},{"osc1Oct",1},{"osc1Unison",4},{"osc1Detune",12},{"osc1Level",0.48f},{"osc2Wave",2},{"osc2Oct",0},{"osc2Fine",-5},{"osc2Level",0.22f},{"cutoff",7100},{"ampAttack",1.5f},{"ampSustain",0.67f},{"ampRelease",4.8f},{"fxDelay",0.25f},{"fxReverb",0.52f},{"stereoWidth",1.4f}}),
        patch ("Backseat Pluck", "Plucks", "Short, muted pluck that leaves room for vocals.", {{"osc1Wave",1},{"osc1Level",0.61f},{"osc2Wave",4},{"osc2Oct",1},{"osc2Level",0.15f},{"cutoff",4100},{"filterEnv",0.32f},{"ampDecay",0.52f},{"ampSustain",0.0f},{"ampRelease",0.34f},{"fxDelay",0.1f},{"fxReverb",0.12f}}),
        patch ("Eastside Pluck", "Plucks", "Bright two-oscillator pluck with syncopated delay space.", {{"osc1Wave",2},{"osc1Level",0.52f},{"osc2Wave",1},{"osc2Oct",1},{"osc2Level",0.25f},{"cutoff",4800},{"filterEnv",0.44f},{"ampDecay",0.68f},{"ampSustain",0.05f},{"ampRelease",0.42f},{"fxDelay",0.22f},{"delayTime",0.31f},{"fxReverb",0.13f}}),
        patch ("Silk Road", "Plucks", "Rounded pluck with a floating, wide tail.", {{"osc1Wave",4},{"osc1Level",0.54f},{"osc2Wave",0},{"osc2Oct",1},{"osc2Fine",8},{"osc2Level",0.22f},{"cutoff",5400},{"ampDecay",1.1f},{"ampSustain",0.08f},{"ampRelease",1.2f},{"fxDelay",0.25f},{"fxReverb",0.28f},{"stereoWidth",1.2f}}),
        patch ("Cartier Lead", "Leads", "Focused lead tone with a controlled upper edge.", {{"osc1Wave",2},{"osc1Unison",3},{"osc1Detune",7},{"osc1Level",0.58f},{"osc2Wave",3},{"osc2Oct",1},{"osc2Level",0.17f},{"cutoff",4300},{"resonance",0.15f},{"ampAttack",0.015f},{"ampDecay",0.55f},{"ampSustain",0.72f},{"ampRelease",0.3f},{"lfoRate",5.2f},{"lfoDepth",0.035f},{"fxDelay",0.12f}}),
        patch ("Redline Lead", "Leads", "Biting saw lead with drive and a tight stereo image.", {{"osc1Wave",2},{"osc1Unison",5},{"osc1Detune",10},{"osc1Level",0.57f},{"osc2Wave",3},{"osc2Oct",-1},{"osc2Level",0.18f},{"cutoff",3500},{"resonance",0.18f},{"ampAttack",0.01f},{"ampSustain",0.77f},{"ampRelease",0.26f},{"fxDrive",0.35f},{"stereoWidth",0.82f}}),
        patch ("Melted Chrome", "Leads", "Expressive, bright lead with a subtle chorus halo.", {{"osc1Wave",4},{"osc1Unison",3},{"osc1Detune",5},{"osc1Level",0.58f},{"osc2Wave",1},{"osc2Oct",1},{"osc2Fine",-6},{"osc2Level",0.22f},{"cutoff",6200},{"ampAttack",0.025f},{"ampSustain",0.72f},{"ampRelease",0.46f},{"fxChorus",0.2f},{"fxDelay",0.12f}}),
        patch ("Nights Like This", "Atlanta", "Melodic Atlanta pluck with a soft, vocal-friendly top.", {{"osc1Wave",4},{"osc1Level",0.56f},{"osc2Wave",1},{"osc2Oct",1},{"osc2Level",0.17f},{"cutoff",4600},{"filterEnv",0.25f},{"ampDecay",1.1f},{"ampSustain",0.13f},{"ampRelease",1.1f},{"fxDelay",0.18f},{"fxReverb",0.22f}}),
        patch ("Zone 6 Bell", "Atlanta", "High-register bell with roomy, measured delay.", {{"osc1Wave",4},{"osc1Oct",1},{"osc1Level",0.64f},{"osc2Wave",0},{"osc2Oct",2},{"osc2Level",0.17f},{"cutoff",8000},{"ampDecay",2.1f},{"ampSustain",0.02f},{"ampRelease",2.6f},{"fxDelay",0.24f},{"delayTime",0.42f},{"fxReverb",0.22f}}),
        patch ("Piedmont Keys", "Atlanta", "Warm minor-key electric keys with a restrained chorus.", {{"osc1Wave",1},{"osc1Level",0.55f},{"osc2Wave",0},{"osc2Oct",-1},{"osc2Level",0.13f},{"cutoff",2900},{"ampDecay",0.88f},{"ampSustain",0.32f},{"ampRelease",0.8f},{"fxChorus",0.14f},{"fxReverb",0.18f}}),
        patch ("Bankroll Brass", "Atlanta", "Bright brass stack for short, hard chord stabs.", {{"osc1Wave",4},{"osc1Unison",4},{"osc1Detune",6},{"osc1Level",0.6f},{"osc2Wave",2},{"osc2Oct",-1},{"osc2Level",0.28f},{"cutoff",3000},{"resonance",0.11f},{"ampAttack",0.015f},{"ampDecay",0.4f},{"ampSustain",0.52f},{"ampRelease",0.3f},{"filterEnv",0.35f},{"fxDrive",0.18f}}),
        patch ("Hush Money", "Dark", "Subdued bell-pluck with a shadowed, low-pass tone.", {{"osc1Wave",4},{"osc1Level",0.51f},{"osc2Wave",1},{"osc2Oct",-1},{"osc2Level",0.18f},{"cutoff",2150},{"resonance",0.08f},{"ampDecay",1.4f},{"ampSustain",0.08f},{"ampRelease",1.8f},{"fxDelay",0.18f},{"fxReverb",0.25f}}),
        patch ("Black Ice", "Dark", "Cold, sustained analog layer with restrained movement.", {{"osc1Wave",2},{"osc1Unison",4},{"osc1Detune",9},{"osc1Level",0.46f},{"osc2Wave",1},{"osc2Oct",-1},{"osc2Fine",-4},{"osc2Level",0.23f},{"cutoff",1900},{"ampAttack",0.42f},{"ampSustain",0.7f},{"ampRelease",2.3f},{"lfoRate",0.23f},{"lfoDepth",0.1f},{"fxChorus",0.24f},{"fxReverb",0.2f}}),
        patch ("Curbside", "Dark", "Low, grainy pluck with a small amount of drive.", {{"osc1Wave",3},{"osc1Oct",-1},{"osc1Level",0.42f},{"osc2Wave",1},{"osc2Level",0.13f},{"subLevel",0.44f},{"cutoff",1700},{"resonance",0.16f},{"ampDecay",0.44f},{"ampSustain",0.1f},{"ampRelease",0.36f},{"fxDrive",0.4f}}),
        patch ("Tunnel Vision", "Dark", "Steady dark lead with a narrow, centered focus.", {{"osc1Wave",2},{"osc1Level",0.57f},{"osc2Wave",3},{"osc2Oct",1},{"osc2Level",0.14f},{"cutoff",2250},{"resonance",0.2f},{"ampAttack",0.02f},{"ampSustain",0.7f},{"ampRelease",0.34f},{"fxDrive",0.22f},{"stereoWidth",0.52f}}),
        patch ("Candlelight", "Melodic", "Intimate, rounded keyboard sound with a long tail.", {{"osc1Wave",1},{"osc1Level",0.47f},{"osc2Wave",4},{"osc2Oct",1},{"osc2Level",0.16f},{"cutoff",2500},{"ampAttack",0.04f},{"ampDecay",1.5f},{"ampSustain",0.25f},{"ampRelease",1.6f},{"fxChorus",0.16f},{"fxReverb",0.3f}}),
        patch ("Blue Tint", "Melodic", "Soft, slightly detuned pluck for emotional progressions.", {{"osc1Wave",4},{"osc1Level",0.53f},{"osc1Unison",3},{"osc1Detune",5},{"osc2Wave",1},{"osc2Oct",1},{"osc2Fine",-5},{"osc2Level",0.2f},{"cutoff",3600},{"ampDecay",1.3f},{"ampSustain",0.18f},{"ampRelease",1.5f},{"fxDelay",0.17f},{"fxReverb",0.24f}}),
        patch ("Sunday Service", "Melodic", "Gentle organ-key blend with a soft stereo cushion.", {{"osc1Wave",2},{"osc1Level",0.44f},{"osc2Wave",0},{"osc2Oct",1},{"osc2Level",0.14f},{"cutoff",4300},{"ampAttack",0.04f},{"ampDecay",0.48f},{"ampSustain",0.78f},{"ampRelease",1.2f},{"fxChorus",0.2f},{"fxReverb",0.23f}}),
        patch ("Satin Strings", "Melodic", "Slow string-like layer for sustained minor chords.", {{"osc1Wave",2},{"osc1Unison",5},{"osc1Detune",13},{"osc1Level",0.43f},{"osc2Wave",1},{"osc2Oct",0},{"osc2Unison",3},{"osc2Detune",9},{"osc2Level",0.25f},{"cutoff",3200},{"ampAttack",0.64f},{"ampDecay",1.1f},{"ampSustain",0.74f},{"ampRelease",2.7f},{"fxChorus",0.25f},{"fxReverb",0.28f}}),
        patch ("Bounce Theory", "Bounce", "Short, springy two-oscillator synth stab.", {{"osc1Wave",3},{"osc1Level",0.53f},{"osc2Wave",1},{"osc2Oct",1},{"osc2Level",0.24f},{"cutoff",3900},{"filterEnv",0.5f},{"ampAttack",0.002f},{"ampDecay",0.29f},{"ampSustain",0.13f},{"ampRelease",0.22f},{"fxDrive",0.18f}}),
        patch ("Turbo Keys", "Bounce", "Punchy digital key with a fast envelope and short room.", {{"osc1Wave",4},{"osc1Level",0.57f},{"osc2Wave",3},{"osc2Oct",1},{"osc2Level",0.18f},{"cutoff",5200},{"ampAttack",0.001f},{"ampDecay",0.36f},{"ampSustain",0.18f},{"ampRelease",0.28f},{"fxReverb",0.12f}}),
        patch ("Kickback", "Bounce", "Brassy, syncopated stab with controlled drive.", {{"osc1Wave",2},{"osc1Unison",3},{"osc1Detune",5},{"osc1Level",0.56f},{"osc2Wave",4},{"osc2Oct",1},{"osc2Level",0.2f},{"cutoff",2850},{"filterEnv",0.52f},{"ampDecay",0.32f},{"ampSustain",0.22f},{"ampRelease",0.24f},{"fxDrive",0.24f}}),
        patch ("Peachtree Pluck", "Bounce", "Tight muted pluck with a clean transient.", {{"osc1Wave",1},{"osc1Level",0.6f},{"osc2Wave",4},{"osc2Oct",1},{"osc2Level",0.12f},{"cutoff",4300},{"filterEnv",0.42f},{"ampDecay",0.42f},{"ampSustain",0.0f},{"ampRelease",0.28f},{"fxDelay",0.08f}}),
        patch ("Velour Vox", "Vocal", "Formant-leaning additive tone for synthetic vocal hooks.", {{"osc1Wave",4},{"osc1Level",0.55f},{"osc2Wave",4},{"osc2Oct",-1},{"osc2Fine",8},{"osc2Level",0.17f},{"cutoff",3300},{"resonance",0.22f},{"ampAttack",0.06f},{"ampSustain",0.74f},{"ampRelease",0.8f},{"lfoRate",0.32f},{"lfoDepth",0.08f},{"fxChorus",0.24f},{"fxReverb",0.18f}}),
        patch ("Stadium Choir", "Vocal", "Wide, airy synthetic choir layer for held notes.", {{"osc1Wave",4},{"osc1Unison",5},{"osc1Detune",10},{"osc1Level",0.45f},{"osc2Wave",1},{"osc2Semi",7},{"osc2Level",0.22f},{"cutoff",5000},{"ampAttack",0.88f},{"ampSustain",0.76f},{"ampRelease",3.4f},{"fxChorus",0.31f},{"fxReverb",0.38f},{"stereoWidth",1.4f}}),
        patch ("Neon Marimba", "Keys", "Perky, mallet-like upper partials with a compact decay.", {{"osc1Wave",4},{"osc1Oct",1},{"osc1Level",0.62f},{"osc2Wave",1},{"osc2Oct",2},{"osc2Level",0.16f},{"cutoff",6900},{"ampDecay",0.72f},{"ampSustain",0.0f},{"ampRelease",0.48f},{"fxReverb",0.14f}}),
        patch ("Dusk Bell", "Bells", "Smoky bell with a filtered attack and slow chorus.", {{"osc1Wave",4},{"osc1Level",0.54f},{"osc2Wave",0},{"osc2Oct",1},{"osc2Fine",-8},{"osc2Level",0.19f},{"cutoff",3900},{"resonance",0.12f},{"ampDecay",1.8f},{"ampSustain",0.08f},{"ampRelease",2.0f},{"fxChorus",0.16f},{"fxReverb",0.24f}}),
        patch ("Gutter Synth", "Leads", "Dirty, mid-forward oscillator stack for aggressive riffs.", {{"osc1Wave",3},{"osc1Unison",4},{"osc1Detune",12},{"osc1Level",0.53f},{"osc2Wave",2},{"osc2Oct",-1},{"osc2Level",0.24f},{"cutoff",2100},{"resonance",0.23f},{"ampSustain",0.69f},{"ampRelease",0.28f},{"fxDrive",0.57f}}),
        patch ("South Beach Pad", "Pads", "Smooth, wide bed with a mellow high end.", {{"osc1Wave",1},{"osc1Unison",5},{"osc1Detune",12},{"osc1Level",0.49f},{"osc2Wave",2},{"osc2Oct",1},{"osc2Level",0.2f},{"cutoff",2450},{"ampAttack",0.92f},{"ampSustain",0.7f},{"ampRelease",3.1f},{"fxChorus",0.3f},{"fxReverb",0.32f},{"stereoWidth",1.35f}}),
        patch ("Airplane Mode", "Plucks", "High, narrow pluck with dotted-feel delay space.", {{"osc1Wave",4},{"osc1Oct",1},{"osc1Level",0.53f},{"osc2Wave",1},{"osc2Oct",1},{"osc2Level",0.18f},{"cutoff",6100},{"ampDecay",0.74f},{"ampSustain",0.06f},{"ampRelease",0.64f},{"fxDelay",0.23f},{"delayTime",0.36f},{"fxReverb",0.17f}}),
        patch ("Tape Dust", "Texture", "Lo-fi leaning, noisy texture with a soft filter edge.", {{"osc1Wave",4},{"osc1Level",0.49f},{"osc2Wave",5},{"osc2Level",0.09f},{"noiseLevel",0.08f},{"cutoff",3000},{"ampAttack",0.03f},{"ampSustain",0.68f},{"ampRelease",1.3f},{"lfoRate",0.12f},{"lfoDepth",0.12f},{"fxDrive",0.15f},{"fxChorus",0.2f},{"fxReverb",0.22f}}),
        patch ("Night Shift", "Atlanta", "Compact dark bell and sub blend for minor-key loops.", {{"osc1Wave",4},{"osc1Level",0.55f},{"osc2Wave",1},{"osc2Oct",-1},{"osc2Level",0.14f},{"subLevel",0.2f},{"cutoff",3900},{"ampDecay",1.45f},{"ampSustain",0.11f},{"ampRelease",1.5f},{"fxDelay",0.14f},{"fxReverb",0.2f}}),
        patch ("Jet Lag Lead", "Melodic", "Smooth upper lead with controlled vibrato and space.", {{"osc1Wave",2},{"osc1Unison",3},{"osc1Detune",5},{"osc1Level",0.54f},{"osc2Wave",4},{"osc2Oct",1},{"osc2Level",0.18f},{"cutoff",4900},{"ampAttack",0.03f},{"ampSustain",0.68f},{"ampRelease",0.55f},{"lfoRate",4.1f},{"lfoDepth",0.04f},{"fxDelay",0.13f},{"fxReverb",0.12f}}),
        patch ("Silencer", "808", "Quiet, clean sub layer designed to sit under a kick.", {{"osc1Wave",0},{"osc1Oct",-1},{"osc1Level",0.12f},{"subLevel",0.72f},{"cutoff",210},{"ampDecay",0.4f},{"ampSustain",0.27f},{"ampRelease",0.12f},{"fxDrive",0.1f},{"masterLevel",0.82f}}),
        patch ("Cathedral Keys", "Keys", "Slow key attack with a luminous stereo release.", {{"osc1Wave",1},{"osc1Level",0.45f},{"osc2Wave",4},{"osc2Oct",1},{"osc2Fine",5},{"osc2Level",0.22f},{"cutoff",4700},{"ampAttack",0.14f},{"ampDecay",1.2f},{"ampSustain",0.38f},{"ampRelease",2.2f},{"fxChorus",0.2f},{"fxReverb",0.35f}})
    };
}

inline const std::vector<Preset>& getFactoryPresets()
{
    static const auto bank = factoryPresets();
    return bank;
}
}
