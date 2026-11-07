#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include <map>

class AnthemParamDial;
class AnthemChoiceControl;
class AnthemPresetGrid;
class AnthemPerformanceKeyboard;
class AnthemVectorPad;

class AnthemAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                        private juce::Timer
{
public:
    explicit AnthemAudioProcessorEditor (AnthemAudioProcessor&);
    ~AnthemAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    enum class Page { browser, community, designer, effects };
    enum class DesignerMode { synth, seq, mod, global };

    AnthemAudioProcessor& processor;
    Page currentPage = Page::designer;
    DesignerMode designerMode = DesignerMode::synth;
    juce::String selectedCategory = "All";
    juce::String generatorStyle = "Atlanta";
    juce::String searchText;
    int keyboardBaseNote = 48;

    juce::TextButton browserButton { "BROWSER" }, communityButton { "COMMUNITY" }, designerButton { "DESIGNER" }, effectsButton { "EFFECTS" };
    juce::TextButton previousButton { "‹" }, nextButton { "›" }, presetMenuButton { "⌄" }, generateButton { "GENERATE" }, generatorStyleButton { "ATLANTA  ⌄" };
    juce::TextButton modeSynthButton { "SYNTH" }, modeSeqButton { "SEQ" }, modeModButton { "MOD" }, modeGlobalButton { "GLOBAL" };
    juce::TextButton savePatchButton { "EXPORT PATCH" }, importPatchButton { "IMPORT PATCH" };
    juce::Label wordmark, currentPresetLabel;
    juce::TextEditor searchBox;
    std::vector<std::unique_ptr<juce::TextButton>> categoryButtons;
    std::vector<std::unique_ptr<juce::TextButton>> filterModeButtons;
    std::vector<std::unique_ptr<AnthemParamDial>> dials;
    std::vector<std::unique_ptr<AnthemChoiceControl>> choices;
    std::map<juce::String, AnthemParamDial*> dialById;
    std::map<juce::String, AnthemChoiceControl*> choiceById;
    std::unique_ptr<AnthemPresetGrid> presetGrid;
    std::unique_ptr<juce::Viewport> presetViewport;
    std::unique_ptr<AnthemPerformanceKeyboard> performanceKeyboard;
    std::unique_ptr<AnthemVectorPad> vectorPad;
    std::unique_ptr<juce::FileChooser> fileChooser;
    juce::TextButton padUp { "▲" }, padDown { "▼" }, padLeft { "◀" }, padRight { "▶" };

    class Look;
    std::unique_ptr<Look> look;

    AnthemParamDial* addDial (const juce::String& id, const juce::String& title);
    AnthemChoiceControl* addChoice (const juce::String& id, const juce::String& title, const juce::StringArray& items);
    void setPage (Page);
    void setDesignerMode (DesignerMode);
    void setCategory (const juce::String&);
    void refreshPresets();
    void refreshPresetTitle();
    void updateFilterModeButtons();
    void selectRelativePreset (int delta);
    void showPresetMenu();
    void chooseGeneratorStyle();
    void exportPatch();
    void importPatch();
    void positionDial (const juce::String& id, juce::Rectangle<int> bounds);
    void positionChoice (const juce::String& id, juce::Rectangle<int> bounds);
    void hideAllControls();
    void drawCard (juce::Graphics&, juce::Rectangle<int>, const juce::String&, juce::Colour accent = juce::Colour (0xff36d5d6));
    void drawWave (juce::Graphics&, juce::Rectangle<int>, int oscillator, juce::Colour);
    void drawFilterCurve (juce::Graphics&, juce::Rectangle<int>);
    void drawEnvelope (juce::Graphics&, juce::Rectangle<int>, juce::Colour, bool filterEnvelope = false);
    void drawLfo (juce::Graphics&, juce::Rectangle<int>);
    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnthemAudioProcessorEditor)
};
