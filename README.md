# ANTHEM

ANTHEM is a playable, polyphonic VST3 instrument and standalone synth for modern trap production. The interface follows the supplied dark studio design: preset browser, two-oscillator designer with filter, insert effects, global controls, XY vector pad, playable keyboard and performance pad.

## Included

- Two mipmapped additive wavetable oscillators, sub oscillator, noise source, up to seven unison voices and 16-note polyphony.
- Two envelopes, a resonant multimode filter, LFO modulation and master stereo width.
- Saturation, chorus, stereo delay and reverb in the audio path.
- 49 original factory patches across Atlanta, 808, bells, keys, plucks, leads and pads.
- A constrained patch generator. It starts from a curated factory patch and applies small, musical changes within that patch family.
- VST3 and standalone CMake targets. No third-party samples are required.

The oscillator called **GRANDE** in the reference design is represented by the first harmonic-rich wavetable oscillator. Oscillator 2 is independently shaped; oscillator 3 is the sub/noise layer. This release synthesizes its sounds from original tables rather than bundling commercial sample content.

## Build on Windows

Requirements: Visual Studio 2022 with the Desktop development with C++ workload, CMake 3.22 or later, Git, and an internet connection on the first configure (CMake fetches JUCE 9.0.5).

From a Developer PowerShell in this directory:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --target ANTHEM_VST3 ANTHEM_Standalone
```

The VST3 bundle is created under `build/ANTHEM_artefacts/Release/VST3/`. The standalone app is under `build/ANTHEM_artefacts/Release/Standalone/`.

## Build the VST3 on GitHub

1. Create an empty repository on GitHub.
2. Extract this project archive and upload its contents to the repository root. Check that `CMakeLists.txt` and `.github/workflows/build.yml` are at the top level.
3. Push or commit the files. GitHub Actions starts **Build ANTHEM VST3** automatically. You can also start it from the **Actions** tab with **Run workflow**.
4. When the run finishes, open it and download the **ANTHEM-Windows-VST3** artifact. Extract the downloaded artifact; it contains the `ANTHEM.vst3` bundle.

The GitHub workflow builds the Windows VST3 and retains its downloadable artifact for 14 days. To rebuild, push another commit or run the workflow again.

## Build on macOS or Linux

Install CMake 3.22+, a C++17 compiler and Git. On Linux, install the usual JUCE audio/GUI development packages for your distribution.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --target ANTHEM_VST3 ANTHEM_Standalone
```

## Using the instrument

- **BROWSER** selects and searches factory patches. Use the category tiles to narrow the list.
- **COMMUNITY** imports and exports `.anthem` patch files for sharing. The **SEQ** tab is a host-arrangement guide; sequencing remains in the DAW piano roll.
- **DESIGNER** edits OSC 1, OSC 2, the filter, envelopes and LFO. The vector pad and keyboard at the bottom send MIDI performance data.
- **EFFECTS** adjusts the four active insert processors: Drive, Chorus, Delay and Reverb.
- **GENERATE** creates a related patch from the current sound family. Use its menu to choose Atlanta, Dark, Melodic or Bounce character.
- The host can automate all exposed synth and effects parameters. MIDI notes are received on all channels.

Factory sounds are original starting points intended for further production work. As with any synthesizer, final loudness, mix fit and perceived quality depend on monitoring, arrangement and effects choices.

## Licensing

This project source is licensed under AGPL-3.0-only; see [LICENSE](LICENSE). Building it fetches JUCE 9.0.5 as a separate dependency. JUCE 9 is offered under AGPLv3 or a JUCE commercial licence. The current Starter plan is free up to its published revenue limit; check the [official JUCE licensing page](https://juce.com/get-juce/) and current terms before distributing binaries.
