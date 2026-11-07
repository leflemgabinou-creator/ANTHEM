#include "PluginEditor.h"
#include <cmath>

namespace
{
const juce::Colour background (0xff0b0d0f);
const juce::Colour panel (0xff111417);
const juce::Colour panelRaised (0xff171b1f);
const juce::Colour edge (0xff282e33);
const juce::Colour textMain (0xffe8ecef);
const juce::Colour textMuted (0xff828b92);
const juce::Colour cyan (0xff39d6d7);
const juce::Colour violet (0xffa78bfa);

void styleButton (juce::TextButton& button, bool active = false)
{
    button.setColour (juce::TextButton::buttonColourId, active ? juce::Colour (0xff20272b) : panel);
    button.setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xff20272b));
    button.setColour (juce::TextButton::textColourOffId, active ? textMain : textMuted);
    button.setColour (juce::TextButton::textColourOnId, textMain);
    button.setClickingTogglesState (false);
    button.setMouseCursor (juce::MouseCursor::PointingHandCursor);
}

bool isBlackNote (int note)
{
    const int pitch = note % 12;
    return pitch == 1 || pitch == 3 || pitch == 6 || pitch == 8 || pitch == 10;
}

juce::Rectangle<float> keyboardKeyBounds (int note, int baseNote, juce::Rectangle<float> bounds)
{
    int whiteIndex = 0;
    for (int n = baseNote; n < note; ++n) if (! isBlackNote (n)) ++whiteIndex;
    const int whiteCount = 28;
    const float whiteWidth = bounds.getWidth() / (float) whiteCount;
    if (! isBlackNote (note))
        return { bounds.getX() + (float) whiteIndex * whiteWidth, bounds.getY(), whiteWidth, bounds.getHeight() };

    const float blackWidth = whiteWidth * 0.62f;
    const float x = bounds.getX() + (float) whiteIndex * whiteWidth - blackWidth * 0.5f;
    return { x, bounds.getY(), blackWidth, bounds.getHeight() * 0.63f };
}
}

class AnthemAudioProcessorEditor::Look final : public juce::LookAndFeel_V4
{
public:
    juce::Font getComboBoxFont (juce::ComboBox&) override { return juce::Font (10.0f); }
    juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override
    {
        return juce::Font (juce::jlimit (9.0f, 14.0f, (float) buttonHeight * 0.38f));
    }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float position,
                           float, float, juce::Slider&) override
    {
        const float side = (float) juce::jmin (width, height) - 12.0f;
        const float radius = side * 0.5f;
        const float cx = (float) x + (float) width * 0.5f;
        const float cy = (float) y + (float) height * 0.5f - 1.0f;
        const float start = juce::MathConstants<float>::pi * 1.22f;
        const float end = juce::MathConstants<float>::pi * 2.78f;
        juce::Path track;
        track.addCentredArc (cx, cy, radius, radius, 0.0f, start, end, true);
        g.setColour (juce::Colour (0xff31383d));
        g.strokePath (track, juce::PathStrokeType (3.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        juce::Path active;
        active.addCentredArc (cx, cy, radius, radius, 0.0f, start, start + position * (end - start), true);
        g.setColour (cyan);
        g.strokePath (active, juce::PathStrokeType (3.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        const float angle = start + position * (end - start);
        juce::Path pointer;
        pointer.addRoundedRectangle (-1.2f, -radius + 4.0f, 2.4f, radius * 0.54f, 1.0f);
        g.setColour (textMain);
        g.fillPath (pointer, juce::AffineTransform::rotation (angle).translated (cx, cy));
        g.setColour (juce::Colour (0xff20262a));
        g.fillEllipse (cx - radius * 0.52f, cy - radius * 0.52f, radius * 1.04f, radius * 1.04f);
    }

    void drawComboBox (juce::Graphics& g, int width, int height, bool, int, int, int, int, juce::ComboBox&) override
    {
        g.setColour (panelRaised);
        g.fillRoundedRectangle (0.0f, 0.0f, (float) width, (float) height, 4.0f);
        g.setColour (edge);
        g.drawRoundedRectangle (0.5f, 0.5f, (float) width - 1.0f, (float) height - 1.0f, 4.0f, 1.0f);
        g.setColour (textMuted);
        juce::Path arrow;
        arrow.startNewSubPath ((float) width - 16.0f, (float) height * 0.42f);
        arrow.lineTo ((float) width - 11.0f, (float) height * 0.58f);
        arrow.lineTo ((float) width - 6.0f, (float) height * 0.42f);
        g.strokePath (arrow, juce::PathStrokeType (1.3f));
    }
};

class AnthemParamDial final : public juce::Component
{
public:
    AnthemParamDial (AnthemAudioProcessor::APVTS& state, const juce::String& id, const juce::String& title)
        : label (title), attachment (std::make_unique<AnthemAudioProcessor::APVTS::SliderAttachment> (state, id, slider))
    {
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        slider.setMouseDragSensitivity (170);
        slider.setScrollWheelEnabled (true);
        slider.setColour (juce::Slider::rotarySliderFillColourId, cyan);
        slider.setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (0xff30363b));
        slider.setColour (juce::Slider::thumbColourId, textMain);
        addAndMakeVisible (slider);
        label.setJustificationType (juce::Justification::centred);
        label.setColour (juce::Label::textColourId, textMuted);
        label.setFont (juce::Font (9.0f));
        label.setInterceptsMouseClicks (false, false);
        addAndMakeVisible (label);
    }

    void resized() override
    {
        label.setBounds (0, getHeight() - 14, getWidth(), 13);
        slider.setBounds (2, 0, getWidth() - 4, getHeight() - 12);
    }

private:
    juce::Slider slider;
    juce::Label label;
    std::unique_ptr<AnthemAudioProcessor::APVTS::SliderAttachment> attachment;
};

class AnthemChoiceControl final : public juce::Component
{
public:
    AnthemChoiceControl (AnthemAudioProcessor::APVTS& state, const juce::String& id, const juce::String& title,
                         const juce::StringArray& items)
        : label (title)
    {
        for (int i = 0; i < items.size(); ++i) combo.addItem (items[i], i + 1);
        attachment = std::make_unique<AnthemAudioProcessor::APVTS::ComboBoxAttachment> (state, id, combo);
        combo.setColour (juce::ComboBox::textColourId, textMain);
        combo.setColour (juce::ComboBox::arrowColourId, cyan);
        combo.setColour (juce::ComboBox::outlineColourId, edge);
        addAndMakeVisible (combo);
        label.setColour (juce::Label::textColourId, textMuted);
        label.setFont (juce::Font (8.0f));
        label.setJustificationType (juce::Justification::centredLeft);
        addAndMakeVisible (label);
    }

    void resized() override
    {
        label.setBounds (0, 0, getWidth(), 12);
        combo.setBounds (0, 13, getWidth(), juce::jmax (22, getHeight() - 13));
    }

private:
    juce::Label label;
    juce::ComboBox combo;
    std::unique_ptr<AnthemAudioProcessor::APVTS::ComboBoxAttachment> attachment;
};

class AnthemPresetGrid final : public juce::Component
{
public:
    explicit AnthemPresetGrid (AnthemAudioProcessor& p) : processor (p) {}
    std::function<void()> onSelection;

    void setFilter (const juce::String& category, const juce::String& search)
    {
        filtered.clear();
        for (const auto& preset : processor.getPresets())
        {
            const bool categoryMatch = category == "All" || preset.category.compareIgnoreCase (category) == 0;
            const bool searchMatch = search.isEmpty() || preset.name.containsIgnoreCase (search)
                || preset.category.containsIgnoreCase (search) || preset.description.containsIgnoreCase (search);
            if (categoryMatch && searchMatch) filtered.push_back (&preset);
        }
        setSize (juce::jmax (1, getParentWidth()), juce::jmax (164, ((int) filtered.size() + 2) / 3 * 44));
        repaint();
    }

    void setSelectedName (const juce::String& name) { selected = name; repaint(); }

    void paint (juce::Graphics& g) override
    {
        const int rowHeight = 44;
        const int colWidth = getWidth() / 3;
        for (int i = 0; i < (int) filtered.size(); ++i)
        {
            const int row = i / 3, col = i % 3;
            auto r = juce::Rectangle<int> (col * colWidth + 4, row * rowHeight + 3, colWidth - 8, rowHeight - 4);
            const bool isSelected = filtered[(std::size_t) i]->name == selected;
            if (isSelected)
            {
                g.setColour (juce::Colour (0xff1c292d));
                g.fillRoundedRectangle (r.toFloat(), 4.0f);
            }
            g.setColour (isSelected ? cyan : textMain);
            g.setFont (juce::Font (11.0f, juce::Font::bold));
            g.drawText (filtered[(std::size_t) i]->name, r.reduced (10, 3).withHeight (17), juce::Justification::centredLeft, true);
            g.setColour (textMuted);
            g.setFont (juce::Font (8.5f));
            g.drawText (filtered[(std::size_t) i]->category.toUpperCase(), r.reduced (10, 3).withY (r.getY() + 22).withHeight (12), juce::Justification::centredLeft, true);
            g.setColour (cyan.withAlpha (0.8f));
            g.fillEllipse ((float) r.getRight() - 17.0f, (float) r.getCentreY() - 2.0f, 4.0f, 4.0f);
        }
        if (filtered.empty())
        {
            g.setColour (textMuted);
            g.setFont (juce::Font (12.0f));
            g.drawText ("No sounds found. Try another search or category.", getLocalBounds(), juce::Justification::centred);
        }
    }

    void mouseDown (const juce::MouseEvent& event) override
    {
        const int colWidth = juce::jmax (1, getWidth() / 3);
        const int col = juce::jlimit (0, 2, event.x / colWidth);
        const int row = event.y / 44;
        const int index = row * 3 + col;
        if (index >= 0 && index < (int) filtered.size())
        {
            processor.loadPreset (*filtered[(std::size_t) index]);
            selected = filtered[(std::size_t) index]->name;
            repaint();
            if (onSelection) onSelection();
        }
    }

private:
    AnthemAudioProcessor& processor;
    std::vector<const anthem::Preset*> filtered;
    juce::String selected;
};

class AnthemPerformanceKeyboard final : public juce::Component
{
public:
    AnthemPerformanceKeyboard (juce::MidiKeyboardState& state, int firstNote) : keyboardState (state), baseNote (firstNote) {}
    void setBaseNote (int note) { if (pressedNote >= 0) release(); baseNote = juce::jlimit (24, 84, note); repaint(); }

    void paint (juce::Graphics& g) override
    {
        auto keys = getLocalBounds().reduced (1).toFloat();
        g.setColour (background);
        g.fillRoundedRectangle (keys, 4.0f);
        for (int note = baseNote; note < baseNote + 48; ++note)
        {
            if (isBlackNote (note)) continue;
            auto r = keyboardKeyBounds (note, baseNote, keys);
            g.setColour (note == pressedNote ? cyan.withAlpha (0.85f) : juce::Colour (0xff22262a));
            g.fillRect (r);
            g.setColour (juce::Colour (0xff343a3e));
            g.drawRect (r, 0.7f);
            if (note % 12 == 0)
            {
                g.setColour (textMuted);
                g.setFont (juce::Font (8.0f));
                g.drawText ("C" + juce::String (note / 12 - 1), r.removeFromBottom (13.0f), juce::Justification::centredLeft);
            }
        }
        for (int note = baseNote; note < baseNote + 48; ++note)
        {
            if (! isBlackNote (note)) continue;
            auto r = keyboardKeyBounds (note, baseNote, keys);
            g.setColour (note == pressedNote ? cyan : juce::Colour (0xff090b0d));
            g.fillRoundedRectangle (r, 2.0f);
            g.setColour (juce::Colour (0xff0b0d0f));
            g.drawRoundedRectangle (r, 2.0f, 1.0f);
        }
        g.setColour (edge);
        g.drawRoundedRectangle (keys, 4.0f, 1.0f);
    }

    void mouseDown (const juce::MouseEvent& e) override { playAt (e.position); }
    void mouseDrag (const juce::MouseEvent& e) override { playAt (e.position); }
    void mouseUp (const juce::MouseEvent&) override { release(); }

private:
    void playAt (juce::Point<float> point)
    {
        const auto bounds = getLocalBounds().reduced (1).toFloat();
        int hit = -1;
        for (int note = baseNote; note < baseNote + 48; ++note)
            if (isBlackNote (note) && keyboardKeyBounds (note, baseNote, bounds).contains (point)) { hit = note; break; }
        if (hit < 0)
            for (int note = baseNote; note < baseNote + 48; ++note)
                if (! isBlackNote (note) && keyboardKeyBounds (note, baseNote, bounds).contains (point)) { hit = note; break; }
        if (hit >= 0 && hit != pressedNote)
        {
            release();
            pressedNote = hit;
            keyboardState.noteOn (1, pressedNote, 0.86f);
            repaint();
        }
    }

    void release()
    {
        if (pressedNote >= 0) keyboardState.noteOff (1, pressedNote, 0.0f);
        pressedNote = -1;
        repaint();
    }

    juce::MidiKeyboardState& keyboardState;
    int baseNote = 48;
    int pressedNote = -1;
};

class AnthemVectorPad final : public juce::Component
{
public:
    explicit AnthemVectorPad (AnthemAudioProcessor& p) : processor (p) {}

    void paint (juce::Graphics& g) override
    {
        auto r = getLocalBounds().reduced (8).toFloat();
        g.setColour (juce::Colour (0xff0d1013));
        g.fillRoundedRectangle (r, 4.0f);
        g.setColour (edge);
        g.drawRoundedRectangle (r, 4.0f, 1.0f);
        g.setColour (juce::Colour (0xff30363a));
        g.drawLine (r.getX() + 10, r.getY() + 10, r.getRight() - 10, r.getBottom() - 10, 0.7f);
        g.drawLine (r.getX() + 10, r.getBottom() - 10, r.getRight() - 10, r.getY() + 10, 0.7f);
        g.setColour (textMuted);
        g.setFont (juce::Font (7.5f));
        g.drawText ("A", r.toNearestInt().reduced (4), juce::Justification::topLeft);
        g.drawText ("B", r.toNearestInt().reduced (4), juce::Justification::topRight);
        g.drawText ("C", r.toNearestInt().reduced (4), juce::Justification::bottomLeft);
        g.drawText ("D", r.toNearestInt().reduced (4), juce::Justification::bottomRight);
        const float x = r.getX() + 12.0f + position.x * (r.getWidth() - 24.0f);
        const float y = r.getY() + 12.0f + position.y * (r.getHeight() - 24.0f);
        g.setColour (cyan.withAlpha (0.2f));
        g.fillEllipse (x - 12.0f, y - 12.0f, 24.0f, 24.0f);
        g.setColour (cyan);
        g.fillEllipse (x - 5.0f, y - 5.0f, 10.0f, 10.0f);
        g.setColour (textMuted);
        g.setFont (juce::Font (8.0f));
        g.drawText ("VECTOR", 6, getHeight() - 12, getWidth() - 12, 10, juce::Justification::centred);
    }

    void mouseDown (const juce::MouseEvent& e) override { dragTo (e.position); }
    void mouseDrag (const juce::MouseEvent& e) override { dragTo (e.position); }

private:
    void dragTo (juce::Point<float> point)
    {
        auto r = getLocalBounds().reduced (20).toFloat();
        position.x = juce::jlimit (0.0f, 1.0f, (point.x - r.getX()) / juce::jmax (1.0f, r.getWidth()));
        position.y = juce::jlimit (0.0f, 1.0f, (point.y - r.getY()) / juce::jmax (1.0f, r.getHeight()));
        const float cutoff = 180.0f * std::pow (2.0f, position.x * 6.0f);
        setValue ("cutoff", cutoff);
        setValue ("osc2Level", 0.04f + (1.0f - position.y) * 0.48f);
        setValue ("stereoWidth", 0.45f + position.y * 0.9f);
        repaint();
    }
    void setValue (const juce::String& id, float value)
    {
        if (auto* parameter = processor.parameters.getParameter (id))
            parameter->setValueNotifyingHost (parameter->convertTo0to1 (value));
    }

    AnthemAudioProcessor& processor;
    juce::Point<float> position { 0.5f, 0.5f };
};

AnthemAudioProcessorEditor::AnthemAudioProcessorEditor (AnthemAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p), look (std::make_unique<Look>())
{
    setLookAndFeel (look.get());
    setSize (1180, 790);
    setResizable (true, true);
    setResizeLimits (1024, 700, 1720, 1080);

    wordmark.setText ("A N T H E M", juce::dontSendNotification);
    wordmark.setFont (juce::Font (13.0f, juce::Font::bold));
    wordmark.setColour (juce::Label::textColourId, textMain);
    addAndMakeVisible (wordmark);
    currentPresetLabel.setJustificationType (juce::Justification::centred);
    currentPresetLabel.setFont (juce::Font (12.0f, juce::Font::bold));
    currentPresetLabel.setColour (juce::Label::textColourId, textMain);
    addAndMakeVisible (currentPresetLabel);

    for (auto* button : { &browserButton, &communityButton, &designerButton, &effectsButton,
                          &previousButton, &nextButton, &presetMenuButton, &generateButton, &generatorStyleButton,
                          &modeSynthButton, &modeSeqButton, &modeModButton, &modeGlobalButton,
                          &savePatchButton, &importPatchButton, &padUp, &padDown, &padLeft, &padRight })
    {
        styleButton (*button);
        addAndMakeVisible (*button);
    }
    generateButton.setColour (juce::TextButton::buttonColourId, textMain);
    generateButton.setColour (juce::TextButton::textColourOffId, background);
    generateButton.setColour (juce::TextButton::textColourOnId, background);
    generateButton.setColour (juce::TextButton::buttonOnColourId, textMain);
    generatorStyleButton.setColour (juce::TextButton::textColourOffId, cyan);

    browserButton.onClick = [this] { setPage (Page::browser); };
    communityButton.onClick = [this] { setPage (Page::community); };
    designerButton.onClick = [this] { setPage (Page::designer); };
    effectsButton.onClick = [this] { setPage (Page::effects); };
    previousButton.onClick = [this] { selectRelativePreset (-1); };
    nextButton.onClick = [this] { selectRelativePreset (1); };
    presetMenuButton.onClick = [this] { showPresetMenu(); };
    generateButton.onClick = [this] { processor.generatePreset (generatorStyle); refreshPresetTitle(); };
    generatorStyleButton.onClick = [this] { chooseGeneratorStyle(); };
    modeSynthButton.onClick = [this] { setDesignerMode (DesignerMode::synth); };
    modeSeqButton.onClick = [this] { setDesignerMode (DesignerMode::seq); };
    modeModButton.onClick = [this] { setDesignerMode (DesignerMode::mod); };
    modeGlobalButton.onClick = [this] { setDesignerMode (DesignerMode::global); };
    savePatchButton.onClick = [this] { exportPatch(); };
    importPatchButton.onClick = [this] { importPatch(); };
    padUp.onClick = [this] { keyboardBaseNote += 12; performanceKeyboard->setBaseNote (keyboardBaseNote); };
    padDown.onClick = [this] { keyboardBaseNote -= 12; performanceKeyboard->setBaseNote (keyboardBaseNote); };
    padLeft.onClick = [this] { --keyboardBaseNote; performanceKeyboard->setBaseNote (keyboardBaseNote); };
    padRight.onClick = [this] { ++keyboardBaseNote; performanceKeyboard->setBaseNote (keyboardBaseNote); };

    searchBox.setColour (juce::TextEditor::backgroundColourId, panelRaised);
    searchBox.setColour (juce::TextEditor::textColourId, textMain);
    searchBox.setColour (juce::TextEditor::outlineColourId, edge);
    searchBox.setColour (juce::TextEditor::focusedOutlineColourId, cyan);
    searchBox.setColour (juce::TextEditor::highlightColourId, cyan.withAlpha (0.35f));
    searchBox.setFont (juce::Font (10.0f));
    searchBox.setTextToShowWhenEmpty ("Search sounds, tags or names", textMuted);
    searchBox.onTextChange = [this] { searchText = searchBox.getText(); refreshPresets(); };
    addAndMakeVisible (searchBox);

    const std::array<std::pair<const char*, const char*>, 13> categories {{
        {"ALL", "All"}, {"KEYS", "Keys"}, {"PADS", "Pads"}, {"PLUCK", "Plucks"}, {"LEAD", "Leads"},
        {"808", "808"}, {"BELLS", "Bells"}, {"ATLANTA", "Atlanta"}, {"DARK", "Dark"}, {"MELODIC", "Melodic"},
        {"BOUNCE", "Bounce"}, {"VOCAL", "Vocal"}, {"TEXTURE", "Texture"}
    }};
    for (const auto& category : categories)
    {
        auto button = std::make_unique<juce::TextButton> (category.first);
        styleButton (*button);
        button->setTooltip (juce::String ("Show ") + category.second + " sounds");
        button->onClick = [this, value = juce::String (category.second)] { setCategory (value); };
        addAndMakeVisible (*button);
        categoryButtons.push_back (std::move (button));
    }

    const std::array<std::pair<const char*, int>, 4> filterModes {{
        { "●  LOW PASS", 0 }, { "●  HIGH PASS", 2 }, { "●  BAND PASS", 3 }, { "●  NOTCH", 4 }
    }};
    for (const auto& mode : filterModes)
    {
        auto button = std::make_unique<juce::TextButton> (mode.first);
        styleButton (*button);
        button->setColour (juce::TextButton::buttonColourId, panel);
        button->onClick = [this, index = mode.second]
        {
            if (auto* parameter = processor.parameters.getParameter ("filterMode"))
                parameter->setValueNotifyingHost (parameter->convertTo0to1 ((float) index));
            updateFilterModeButtons();
        };
        addAndMakeVisible (*button);
        filterModeButtons.push_back (std::move (button));
    }

    presetViewport = std::make_unique<juce::Viewport>();
    presetGrid = std::make_unique<AnthemPresetGrid> (processor);
    presetGrid->onSelection = [this] { refreshPresetTitle(); };
    presetViewport->setViewedComponent (presetGrid.get(), false);
    presetViewport->setScrollBarsShown (true, false);
    presetViewport->setColour (juce::ScrollBar::thumbColourId, juce::Colour (0xff41484e));
    addAndMakeVisible (*presetViewport);
    refreshPresets();
    setCategory ("All");

    performanceKeyboard = std::make_unique<AnthemPerformanceKeyboard> (processor.keyboardState, keyboardBaseNote);
    addAndMakeVisible (*performanceKeyboard);
    vectorPad = std::make_unique<AnthemVectorPad> (processor);
    addAndMakeVisible (*vectorPad);

    addDial ("masterLevel", "LEVEL"); addDial ("masterPan", "PAN"); addDial ("stereoWidth", "WIDTH");
    addDial ("ampAttack", "ATTACK"); addDial ("ampDecay", "DECAY"); addDial ("ampSustain", "SUSTAIN"); addDial ("ampRelease", "RELEASE");
    addDial ("cutoff", "CUTOFF"); addDial ("resonance", "RES"); addDial ("filterDrive", "DRIVE"); addDial ("filterEnv", "ENV AMT");
    addDial ("osc1Level", "OUTPUT"); addDial ("osc1Pan", "PAN"); addDial ("osc1Unison", "UNISON"); addDial ("osc1Detune", "DETUNE"); addDial ("osc1Oct", "OCT"); addDial ("osc1Fine", "FINE");
    addDial ("osc2Level", "OUTPUT"); addDial ("osc2Pan", "PAN"); addDial ("osc2Unison", "UNISON"); addDial ("osc2Detune", "DETUNE"); addDial ("osc2Oct", "OCT"); addDial ("osc2Fine", "FINE");
    addDial ("filterAttack", "ATTACK"); addDial ("filterDecay", "DECAY"); addDial ("filterSustain", "SUSTAIN"); addDial ("filterRelease", "RELEASE");
    addDial ("lfoRate", "RATE"); addDial ("lfoDepth", "DEPTH");
    addDial ("fxDrive", "DRIVE"); addDial ("fxChorus", "CHORUS"); addDial ("fxDelay", "DELAY"); addDial ("fxReverb", "REVERB");
    addDial ("delayTime", "TIME"); addDial ("delayFeedback", "FEEDBACK"); addDial ("subLevel", "SUB"); addDial ("noiseLevel", "NOISE");
    addChoice ("osc1Wave", "OSC 1 WAVE", { "Sine", "Triangle", "Saw", "Pulse", "Grande", "Noise" });
    addChoice ("osc2Wave", "OSC 2 WAVE", { "Sine", "Triangle", "Saw", "Pulse", "Grande", "Noise" });
    addChoice ("filterMode", "FILTER", { "Classic LP24", "Soft LP12", "Bright HP12", "Band Pass", "Notch" });

    refreshPresetTitle();
    setPage (Page::designer);
    updateFilterModeButtons();
    startTimerHz (12);
}

AnthemAudioProcessorEditor::~AnthemAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

AnthemParamDial* AnthemAudioProcessorEditor::addDial (const juce::String& id, const juce::String& title)
{
    auto component = std::make_unique<AnthemParamDial> (processor.parameters, id, title);
    auto* result = component.get();
    result->setVisible (false);
    addAndMakeVisible (*result);
    dials.push_back (std::move (component));
    dialById[id] = result;
    return result;
}

AnthemChoiceControl* AnthemAudioProcessorEditor::addChoice (const juce::String& id, const juce::String& title, const juce::StringArray& items)
{
    auto component = std::make_unique<AnthemChoiceControl> (processor.parameters, id, title, items);
    auto* result = component.get();
    result->setVisible (false);
    addAndMakeVisible (*result);
    choices.push_back (std::move (component));
    choiceById[id] = result;
    return result;
}

void AnthemAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (background);
    const int width = getWidth(), height = getHeight();
    const int footerTop = height - 198;
    g.setColour (juce::Colour (0xff080a0c));
    g.fillRect (0, 0, width, 46);
    g.setColour (edge);
    g.drawLine (0.0f, 45.5f, (float) width, 45.5f, 1.0f);
    g.setColour (cyan);
    if (currentPage == Page::browser) g.fillRect (138, 43, 88, 2);
    if (currentPage == Page::community) g.fillRect (226, 43, 112, 2);
    if (currentPage == Page::designer) g.fillRect (338, 43, 91, 2);
    if (currentPage == Page::effects) g.fillRect (429, 43, 82, 2);
    g.setColour (panel);
    g.fillRect (0, 46, width, 38);
    g.setColour (edge);
    g.drawLine (0.0f, 83.5f, (float) width, 83.5f, 1.0f);

    const int x0 = 12, contentTop = 92;
    if (currentPage == Page::browser)
    {
        drawCard (g, { 10, contentTop, 150, footerTop - contentTop - 8 }, "FILTER");
        drawCard (g, { 166, contentTop, width - 370, footerTop - contentTop - 8 }, "FACTORY SOUNDS · 49 ORIGINAL PATCHES");
        drawCard (g, { width - 194, contentTop, 184, footerTop - contentTop - 8 }, "GLOBAL");
    }
    else if (currentPage == Page::designer)
    {
        if (designerMode == DesignerMode::synth)
        {
            const int gap = 9, totalW = width - 20, cardW = (totalW - 2 * gap) / 3;
            const int topY = contentTop, topH = 220, bottomY = topY + topH + 10, bottomH = footerTop - bottomY - 8;
            juce::Rectangle<int> osc1 (10, topY, cardW, topH), osc2 (10 + cardW + gap, topY, cardW, topH), filter (10 + 2 * (cardW + gap), topY, cardW, topH);
            drawCard (g, osc1, "01   OSCILLATOR A · GRANDE", cyan);
            drawCard (g, osc2, "02   OSCILLATOR B · TRIANGLE", violet);
            drawCard (g, filter, "03   CLASSIC FILTER", juce::Colour (0xffff657a));
            drawWave (g, osc1.reduced (18).withTop (osc1.getY() + 68).withHeight (53), 0, cyan);
            drawWave (g, osc2.reduced (18).withTop (osc2.getY() + 68).withHeight (53), 1, violet);
            drawFilterCurve (g, filter.reduced (18).withTop (filter.getY() + 68).withHeight (53));
            const int bottomW = (totalW - 2 * gap) / 3;
            juce::Rectangle<int> ampCard (10, bottomY, bottomW, bottomH), filterEnvCard (10 + bottomW + gap, bottomY, bottomW, bottomH), modCard (10 + 2 * (bottomW + gap), bottomY, bottomW, bottomH);
            drawCard (g, ampCard, "AMP ENV", violet);
            drawCard (g, filterEnvCard, "FILTER ENV", cyan);
            drawCard (g, modCard, "LFO · MODULATION", juce::Colour (0xffffbd66));
            drawEnvelope (g, ampCard.reduced (18).withTop (ampCard.getY() + 40).withHeight (64), violet);
            drawEnvelope (g, filterEnvCard.reduced (18).withTop (filterEnvCard.getY() + 40).withHeight (64), cyan, true);
            drawLfo (g, modCard.reduced (18).withTop (modCard.getY() + 40).withHeight (64));
        }
        else if (designerMode == DesignerMode::seq)
        {
            juce::Rectangle<int> card (10, contentTop, width - 20, footerTop - contentTop - 8);
            drawCard (g, card, "PERFORMANCE SEQUENCE · HOST MIDI", cyan);
            g.setColour (textMuted); g.setFont (juce::Font (11.0f));
            g.drawText ("Use your DAW piano roll or the keyboard below to play ANTHEM. The 16-step display is a visual guide for programming trap patterns in your host.", card.reduced (26).withTop (card.getY() + 44).withHeight (36), juce::Justification::centredLeft, true);
            const int stepY = card.getY() + 130, stepW = (card.getWidth() - 52) / 16;
            for (int i = 0; i < 16; ++i)
            {
                const int barHeight = 35 + ((i * 17 + (i / 4) * 13) % 105);
                auto r = juce::Rectangle<int> (card.getX() + 26 + i * stepW, stepY + 140 - barHeight, stepW - 7, barHeight);
                g.setColour ((i % 4 == 0) ? cyan : (i % 2 == 0 ? juce::Colour (0xff247f84) : juce::Colour (0xff1a3a3d)));
                g.fillRoundedRectangle (r.toFloat(), 3.0f);
                g.setColour (textMuted); g.setFont (juce::Font (8.0f));
                g.drawText (juce::String (i + 1).paddedLeft ('0', 2), r.getX() - 2, stepY + 148, stepW, 12, juce::Justification::centred);
            }
            g.setColour (edge); g.drawLine ((float) card.getX() + 20, (float) stepY + 142, (float) card.getRight() - 20, (float) stepY + 142, 1.0f);
            g.setColour (textMuted); g.setFont (juce::Font (9.0f));
            g.drawText ("KICK / 808", card.getX() + 26, stepY + 165, 90, 14, juce::Justification::centredLeft);
            g.drawText ("SNARE", card.getX() + 26, stepY + 183, 90, 14, juce::Justification::centredLeft);
            g.drawText ("HI-HAT", card.getX() + 26, stepY + 201, 90, 14, juce::Justification::centredLeft);
            g.setColour (textMuted); g.setFont (juce::Font (9.0f));
            g.drawText ("16-STEP ARRANGEMENT GUIDE  ·  MIDI NOTES ACCEPTED ON ALL CHANNELS", card.getX() + 26, card.getBottom() - 35, card.getWidth() - 50, 18, juce::Justification::centredLeft);
        }
        else if (designerMode == DesignerMode::mod)
        {
            const int cardW = (width - 30) / 2;
            drawCard (g, { 10, contentTop, cardW, footerTop - contentTop - 8 }, "LOW FREQUENCY OSCILLATOR", cyan);
            drawCard (g, { 20 + cardW, contentTop, cardW, footerTop - contentTop - 8 }, "ENVELOPES", violet);
            drawLfo (g, { 40, contentTop + 64, cardW - 60, 110 });
            drawEnvelope (g, { cardW + 44, contentTop + 64, cardW - 60, 110 }, violet, true);
            g.setColour (textMuted); g.setFont (juce::Font (9.0f));
            g.drawText ("LFO currently adds gentle pitch movement to held notes.", 35, contentTop + 184, cardW - 45, 20, juce::Justification::centredLeft);
            g.drawText ("AMP ENV and FILTER ENV each shape the sound from note-on through release.", cardW + 35, contentTop + 184, cardW - 45, 20, juce::Justification::centredLeft);
        }
        else
        {
            drawCard (g, { 10, contentTop, width - 20, footerTop - contentTop - 8 }, "GLOBAL PERFORMANCE CONTROLS", cyan);
            g.setColour (textMuted); g.setFont (juce::Font (10.0f));
            g.drawText ("Set output level, stereo position and width. These controls are available to automate from your host.", 34, contentTop + 48, width - 70, 22, juce::Justification::centredLeft);
        }
    }
    else if (currentPage == Page::effects)
    {
        const int gap = 10, cardW = (width - 30 - 3 * gap) / 4;
        const std::array<juce::String, 4> titles { "01   DRIVE", "02   CHORUS", "03   STEREO DELAY", "04   REVERB" };
        const std::array<juce::Colour, 4> colors { juce::Colour (0xffffbd66), cyan, violet, juce::Colour (0xffff657a) };
        for (int i = 0; i < 4; ++i)
            drawCard (g, { 10 + i * (cardW + gap), contentTop, cardW, footerTop - contentTop - 8 }, titles[(std::size_t) i], colors[(std::size_t) i]);
        g.setColour (textMuted); g.setFont (juce::Font (9.0f));
        g.drawText ("SOFT CLIP", 35, contentTop + 45, 130, 15, juce::Justification::centredLeft);
        g.drawText ("MODULATION", cardW + 56, contentTop + 45, 130, 15, juce::Justification::centredLeft);
        g.drawText ("PING-PONG", 2 * (cardW + gap) + 56, contentTop + 45, 130, 15, juce::Justification::centredLeft);
        g.drawText ("SPACE", 3 * (cardW + gap) + 56, contentTop + 45, 130, 15, juce::Justification::centredLeft);
    }
    else
    {
        drawCard (g, { 10, contentTop, width - 20, footerTop - contentTop - 8 }, "COMMUNITY · SHARE YOUR SOUND", cyan);
        g.setColour (textMain); g.setFont (juce::Font (20.0f, juce::Font::bold));
        g.drawText ("Made for the next session.", 40, contentTop + 48, width - 80, 34, juce::Justification::centredLeft);
        g.setColour (textMuted); g.setFont (juce::Font (11.0f));
        g.drawText ("Export an .anthem patch file to share your sound. Import a patch from another producer to add it to your session.", 40, contentTop + 88, width - 80, 28, juce::Justification::centredLeft, true);
        g.setColour (edge); g.drawLine (40.0f, (float) contentTop + 134.0f, (float) width - 40.0f, (float) contentTop + 134.0f, 1.0f);
        g.setColour (cyan); g.setFont (juce::Font (9.0f, juce::Font::bold));
        g.drawText ("FACTORY COLLECTION", 40, contentTop + 158, 180, 18, juce::Justification::centredLeft);
        g.setColour (textMuted); g.setFont (juce::Font (10.0f));
        g.drawText ("49 original patches  ·  Atlanta, melodic trap, dark textures, bells, keys, leads and 808s", 40, contentTop + 181, width - 80, 20, juce::Justification::centredLeft);
        g.drawText ("Your imported patch state stays local to the current project. Use the browser to explore the factory bank.", 40, contentTop + 209, width - 80, 20, juce::Justification::centredLeft);
    }

    g.setColour (panel);
    g.fillRoundedRectangle (8.0f, (float) footerTop, (float) width - 16.0f, (float) height - (float) footerTop - 8.0f, 9.0f);
    g.setColour (edge);
    g.drawRoundedRectangle (8.0f, (float) footerTop, (float) width - 16.0f, (float) height - (float) footerTop - 8.0f, 9.0f, 1.0f);
}

void AnthemAudioProcessorEditor::drawCard (juce::Graphics& g, juce::Rectangle<int> bounds, const juce::String& title, juce::Colour accent)
{
    g.setColour (panel);
    g.fillRoundedRectangle (bounds.toFloat(), 6.0f);
    g.setColour (edge);
    g.drawRoundedRectangle (bounds.toFloat().reduced (0.5f), 6.0f, 1.0f);
    g.setColour (accent);
    g.fillRoundedRectangle ((float) bounds.getX() + 13.0f, (float) bounds.getY() + 11.0f, 3.0f, 11.0f, 1.0f);
    g.setColour (textMain);
    g.setFont (juce::Font (9.5f, juce::Font::bold));
    g.drawText (title, bounds.getX() + 23, bounds.getY() + 8, bounds.getWidth() - 34, 16, juce::Justification::centredLeft, true);
}

void AnthemAudioProcessorEditor::drawWave (juce::Graphics& g, juce::Rectangle<int> bounds, int oscillator, juce::Colour color)
{
    g.setColour (juce::Colour (0xff0c1013));
    g.fillRoundedRectangle (bounds.toFloat(), 3.0f);
    for (int i = 1; i < 4; ++i)
    {
        g.setColour (juce::Colour (0xff22282d));
        g.drawLine ((float) bounds.getX(), (float) bounds.getY() + (float) i * bounds.getHeight() / 4.0f, (float) bounds.getRight(), (float) bounds.getY() + (float) i * bounds.getHeight() / 4.0f, 0.5f);
    }
    const auto p = processor.getParameterSnapshot();
    const int wave = juce::roundToInt (p[oscillator == 0 ? AnthemAudioProcessor::osc1Wave : AnthemAudioProcessor::osc2Wave]);
    juce::Path path;
    for (int i = 0; i <= bounds.getWidth(); ++i)
    {
        const double phase = (double) i / juce::jmax (1, bounds.getWidth());
        double y = 0.0;
        if (wave == 0) y = std::sin (phase * juce::MathConstants<double>::twoPi);
        else if (wave == 1) y = 2.0 * std::abs (2.0 * phase - std::floor (2.0 * phase + 0.5)) - 1.0;
        else if (wave == 2) y = 1.0 - 2.0 * phase;
        else if (wave == 3) y = phase < 0.5 ? 1.0 : -1.0;
        else if (wave == 4) y = 0.72 * std::sin (phase * 6.283) + 0.22 * std::sin (phase * 12.566 + 0.45) + 0.1 * std::sin (phase * 31.416);
        else y = std::sin (phase * 2.0 * juce::MathConstants<double>::pi * 19.0) * 0.12;
        const float px = (float) bounds.getX() + (float) i;
        const float py = (float) bounds.getCentreY() - (float) y * (float) bounds.getHeight() * 0.38f;
        if (i == 0) path.startNewSubPath (px, py); else path.lineTo (px, py);
    }
    g.setColour (color.withAlpha (0.14f));
    auto fill = path; fill.lineTo ((float) bounds.getRight(), (float) bounds.getBottom()); fill.lineTo ((float) bounds.getX(), (float) bounds.getBottom()); fill.closeSubPath();
    g.fillPath (fill);
    g.setColour (color);
    g.strokePath (path, juce::PathStrokeType (1.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour (textMuted); g.setFont (juce::Font (7.0f));
    g.drawText ("WAVEFORM", bounds.getX() + 5, bounds.getY() + 3, 70, 10, juce::Justification::centredLeft);
}

void AnthemAudioProcessorEditor::drawFilterCurve (juce::Graphics& g, juce::Rectangle<int> bounds)
{
    g.setColour (juce::Colour (0xff0c1013)); g.fillRoundedRectangle (bounds.toFloat(), 3.0f);
    const auto p = processor.getParameterSnapshot();
    const float cutoff = p[AnthemAudioProcessor::cutoff];
    const float normalized = juce::jlimit (0.06f, 0.95f, std::log (cutoff / 30.0f) / std::log (18000.0f / 30.0f));
    const float cx = (float) bounds.getX() + normalized * (float) bounds.getWidth();
    juce::Path curve;
    curve.startNewSubPath ((float) bounds.getX() + 3.0f, (float) bounds.getY() + 7.0f);
    curve.lineTo (cx - 18.0f, (float) bounds.getY() + 7.0f);
    curve.cubicTo (cx - 6.0f, (float) bounds.getY() + 8.0f, cx - 12.0f, (float) bounds.getBottom() - 2.0f, cx + 15.0f, (float) bounds.getBottom() - 5.0f);
    curve.lineTo ((float) bounds.getRight() - 2.0f, (float) bounds.getBottom() - 7.0f);
    g.setColour (juce::Colour (0xff343b40)); g.drawLine ((float) bounds.getX(), (float) bounds.getBottom() - 9.0f, (float) bounds.getRight(), (float) bounds.getBottom() - 9.0f, 0.7f);
    g.setColour (juce::Colour (0xffff657a)); g.strokePath (curve, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.fillEllipse (cx + 10.0f, (float) bounds.getBottom() - 10.0f, 7.0f, 7.0f);
}

void AnthemAudioProcessorEditor::drawEnvelope (juce::Graphics& g, juce::Rectangle<int> bounds, juce::Colour color, bool filterEnvelope)
{
    g.setColour (juce::Colour (0xff0c1013)); g.fillRoundedRectangle (bounds.toFloat(), 3.0f);
    const auto p = processor.getParameterSnapshot();
    const auto attackIndex = filterEnvelope ? AnthemAudioProcessor::filterAttack : AnthemAudioProcessor::ampAttack;
    const auto decayIndex = filterEnvelope ? AnthemAudioProcessor::filterDecay : AnthemAudioProcessor::ampDecay;
    const auto sustainIndex = filterEnvelope ? AnthemAudioProcessor::filterSustain : AnthemAudioProcessor::ampSustain;
    const auto releaseIndex = filterEnvelope ? AnthemAudioProcessor::filterRelease : AnthemAudioProcessor::ampRelease;
    const float x0 = (float) bounds.getX() + 6.0f, y0 = (float) bounds.getBottom() - 7.0f;
    const float attackSpan = 0.07f + 0.24f * std::sqrt (p[attackIndex] / 8.0f);
    const float decaySpan = 0.1f + 0.21f * std::sqrt (p[decayIndex] / 8.0f);
    const float x1 = x0 + (float) bounds.getWidth() * attackSpan;
    const float x2 = x1 + (float) bounds.getWidth() * decaySpan;
    const float releaseSpan = juce::jmin (0.3f, 0.1f + 0.22f * std::sqrt (p[releaseIndex] / 12.0f));
    const float x4 = (float) bounds.getRight() - 4.0f;
    const float x3 = juce::jmax (x2 + 4.0f, x4 - (float) bounds.getWidth() * releaseSpan);
    const float yTop = (float) bounds.getY() + 8.0f;
    const float ySus = y0 - p[sustainIndex] * ((float) bounds.getHeight() - 16.0f);
    juce::Path path;
    path.startNewSubPath (x0, y0); path.lineTo (x1, yTop); path.lineTo (x2, ySus); path.lineTo (x3, ySus); path.cubicTo (x4 - 8, ySus, x4 + 1, y0 + 1, (float) bounds.getRight() - 4, y0 + 1);
    g.setColour (color.withAlpha (0.12f)); auto fill = path; fill.lineTo ((float) bounds.getRight(), (float) bounds.getBottom()); fill.lineTo ((float) bounds.getX(), (float) bounds.getBottom()); fill.closeSubPath(); g.fillPath (fill);
    g.setColour (color); g.strokePath (path, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    for (auto point : { juce::Point<float> (x0,y0), {x1,yTop}, {x2,ySus}, {x3,ySus}, {(float) bounds.getRight() - 4,y0 + 1} })
        g.fillEllipse (point.x - 3.0f, point.y - 3.0f, 6.0f, 6.0f);
}

void AnthemAudioProcessorEditor::drawLfo (juce::Graphics& g, juce::Rectangle<int> bounds)
{
    g.setColour (juce::Colour (0xff0c1013)); g.fillRoundedRectangle (bounds.toFloat(), 3.0f);
    juce::Path path;
    for (int i = 0; i <= bounds.getWidth(); ++i)
    {
        const float x = (float) bounds.getX() + (float) i;
        const float phase = (float) i / (float) juce::jmax (1, bounds.getWidth());
        const float y = (float) bounds.getCentreY() - std::sin (phase * juce::MathConstants<float>::twoPi * 1.5f) * (float) bounds.getHeight() * 0.36f;
        if (i == 0) path.startNewSubPath (x, y); else path.lineTo (x, y);
    }
    g.setColour (juce::Colour (0xffffbd66)); g.strokePath (path, juce::PathStrokeType (1.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void AnthemAudioProcessorEditor::resized()
{
    const int w = getWidth(), h = getHeight(), footerTop = h - 198;
    wordmark.setBounds (16, 9, 105, 26);
    browserButton.setBounds (138, 4, 88, 38); communityButton.setBounds (226, 4, 112, 38);
    designerButton.setBounds (338, 4, 91, 38); effectsButton.setBounds (429, 4, 82, 38);
    previousButton.setBounds (w - 422, 49, 30, 29); nextButton.setBounds (w - 389, 49, 30, 29);
    currentPresetLabel.setBounds (w - 350, 49, 218, 29); presetMenuButton.setBounds (w - 130, 49, 24, 29);
    generatorStyleButton.setBounds (w - 102, 49, 98, 29);
    generateButton.setBounds (w - 112, 8, 96, 30);
    searchBox.setBounds (170, 96, w - 374, 28);

    const int contentTop = 92;
    if (currentPage == Page::browser)
    {
        for (std::size_t i = 0; i < categoryButtons.size(); ++i)
        {
            const int col = (int) (i % 7), row = (int) (i / 7);
            const int tileW = (w - 374 - 12) / 7;
            categoryButtons[i]->setBounds (170 + col * tileW, 130 + row * 31, tileW - 3, 27);
            categoryButtons[i]->setVisible (true);
        }
        for (std::size_t i = 0; i < filterModeButtons.size(); ++i)
        {
            filterModeButtons[i]->setBounds (20, 134 + (int) i * 16, 130, 15);
            filterModeButtons[i]->setVisible (true);
        }
        presetViewport->setBounds (170, 196, w - 374, footerTop - 204);
        presetGrid->setFilter (selectedCategory, searchText);
        searchBox.setVisible (true);
        presetViewport->setVisible (true);
        positionDial ("cutoff", { 22, 246, 63, 77 }); positionDial ("resonance", { 87, 246, 63, 77 });
        positionDial ("filterDrive", { 22, 326, 63, 77 });
        positionDial ("masterLevel", { w - 182, 133, 70, 74 }); positionDial ("masterPan", { w - 108, 133, 70, 74 });
        positionDial ("stereoWidth", { w - 182, 214, 70, 74 }); positionDial ("ampAttack", { w - 108, 214, 70, 74 });
        positionDial ("ampDecay", { w - 182, 295, 70, 74 }); positionDial ("ampSustain", { w - 108, 295, 70, 74 });
        positionDial ("ampRelease", { w - 145, 376, 70, 74 });
    }
    else
    {
        searchBox.setVisible (false);
        presetViewport->setVisible (false);
        for (auto& button : categoryButtons) button->setVisible (false);
        for (auto& button : filterModeButtons) button->setVisible (false);
    }

    if (currentPage == Page::designer)
    {
        modeSynthButton.setBounds (525, 8, 70, 30); modeSeqButton.setBounds (595, 8, 54, 30);
        modeModButton.setBounds (649, 8, 60, 30); modeGlobalButton.setBounds (709, 8, 72, 30);
        modeSynthButton.setVisible (true); modeSeqButton.setVisible (true); modeModButton.setVisible (true); modeGlobalButton.setVisible (true);
        if (designerMode == DesignerMode::synth)
        {
            const int gap = 9, totalW = w - 20, cardW = (totalW - 2 * gap) / 3;
            const int topY = contentTop, topH = 220, bottomY = topY + topH + 10, bottomH = footerTop - bottomY - 8;
            for (int osc = 0; osc < 2; ++osc)
            {
                const int x = 10 + osc * (cardW + gap);
                const juce::String waveId = osc == 0 ? "osc1Wave" : "osc2Wave";
                positionChoice (waveId, { x + cardW - 145, topY + 28, 128, 38 });
                const std::array<juce::String, 7> ids = osc == 0
                    ? std::array<juce::String, 7> { "osc1Level", "osc1Pan", "osc1Unison", "osc1Detune", "osc1Oct", "osc1Semi", "osc1Fine" }
                    : std::array<juce::String, 7> { "osc2Level", "osc2Pan", "osc2Unison", "osc2Detune", "osc2Oct", "osc2Semi", "osc2Fine" };
                const int dialW = cardW / 7;
                for (int i = 0; i < 7; ++i) positionDial (ids[(std::size_t) i], { x + 4 + i * dialW, topY + 135, dialW - 2, 76 });
            }
            const int filterX = 10 + 2 * (cardW + gap);
            positionChoice ("filterMode", { filterX + cardW - 150, topY + 28, 133, 38 });
            const std::array<juce::String, 4> filterIds { "cutoff", "resonance", "filterDrive", "filterEnv" };
            for (int i = 0; i < 4; ++i) positionDial (filterIds[(std::size_t) i], { filterX + 17 + i * (cardW - 30) / 4, topY + 137, (cardW - 30) / 4, 74 });
            const int bottomW = (totalW - 2 * gap) / 3;
            const std::array<juce::String, 4> ampIds { "ampAttack", "ampDecay", "ampSustain", "ampRelease" };
            const std::array<juce::String, 4> filterEnvIds { "filterAttack", "filterDecay", "filterSustain", "filterRelease" };
            for (int i = 0; i < 4; ++i)
            {
                positionDial (ampIds[(std::size_t) i], { 17 + i * (bottomW - 12) / 4, bottomY + 110, (bottomW - 12) / 4, juce::jmin (76, bottomH - 115) });
                positionDial (filterEnvIds[(std::size_t) i], { 27 + bottomW + gap + i * (bottomW - 12) / 4, bottomY + 110, (bottomW - 12) / 4, juce::jmin (76, bottomH - 115) });
            }
            const int modX = 10 + 2 * (bottomW + gap);
            positionDial ("lfoRate", { modX + 22, bottomY + 110, 78, juce::jmin (76, bottomH - 115) });
            positionDial ("lfoDepth", { modX + 111, bottomY + 110, 78, juce::jmin (76, bottomH - 115) });
            positionDial ("subLevel", { modX + 203, bottomY + 110, 72, juce::jmin (76, bottomH - 115) });
            positionDial ("noiseLevel", { modX + 279, bottomY + 110, 72, juce::jmin (76, bottomH - 115) });
        }
        else if (designerMode == DesignerMode::mod)
        {
            const int cardW = (w - 30) / 2;
            positionDial ("lfoRate", { 35, footerTop - 150, 88, 78 }); positionDial ("lfoDepth", { 136, footerTop - 150, 88, 78 });
            positionDial ("ampAttack", { cardW + 35, footerTop - 150, 82, 78 }); positionDial ("ampDecay", { cardW + 128, footerTop - 150, 82, 78 });
            positionDial ("ampSustain", { cardW + 221, footerTop - 150, 82, 78 }); positionDial ("ampRelease", { cardW + 314, footerTop - 150, 82, 78 });
        }
        else if (designerMode == DesignerMode::global)
        {
            const int startX = 35, y1 = contentTop + 125, size = 86, gap = 30;
            positionDial ("masterLevel", { startX, y1, size, 82 }); positionDial ("masterPan", { startX + size + gap, y1, size, 82 });
            positionDial ("stereoWidth", { startX + 2 * (size + gap), y1, size, 82 });
            positionDial ("ampAttack", { startX + 3 * (size + gap), y1, size, 82 }); positionDial ("ampDecay", { startX + 4 * (size + gap), y1, size, 82 });
            positionDial ("ampSustain", { startX + 5 * (size + gap), y1, size, 82 }); positionDial ("ampRelease", { startX + 6 * (size + gap), y1, size, 82 });
        }
    }
    else
    {
        modeSynthButton.setVisible (false); modeSeqButton.setVisible (false); modeModButton.setVisible (false); modeGlobalButton.setVisible (false);
    }

    if (currentPage == Page::effects)
    {
        const int cardW = (w - 30 - 3 * 10) / 4, y = contentTop + 100;
        positionDial ("fxDrive", { 25, y, 82, 88 });
        positionDial ("fxChorus", { cardW + 47, y, 82, 88 });
        positionDial ("fxDelay", { 2 * (cardW + 10) + 30, y, 82, 88 });
        positionDial ("delayTime", { 2 * (cardW + 10) + 123, y, 82, 88 });
        positionDial ("delayFeedback", { 2 * (cardW + 10) + 216, y, 82, 88 });
        positionDial ("fxReverb", { 3 * (cardW + 10) + 47, y, 82, 88 });
        positionDial ("stereoWidth", { 3 * (cardW + 10) + 140, y, 82, 88 });
    }

    const int footerY = footerTop + 9, footerH = h - footerY - 16;
    vectorPad->setBounds (14, footerY + 3, 132, footerH - 5);
    performanceKeyboard->setBounds (154, footerY + 8, w - 292, footerH - 17);
    const int dpadX = w - 128, dpadY = footerY + (footerH - 72) / 2;
    padUp.setBounds (dpadX + 24, dpadY, 36, 30); padDown.setBounds (dpadX + 24, dpadY + 42, 36, 30);
    padLeft.setBounds (dpadX, dpadY + 21, 30, 30); padRight.setBounds (dpadX + 54, dpadY + 21, 30, 30);
    vectorPad->setVisible (true); performanceKeyboard->setVisible (true);
    padUp.setVisible (true); padDown.setVisible (true); padLeft.setVisible (true); padRight.setVisible (true);

    savePatchButton.setVisible (currentPage == Page::community);
    importPatchButton.setVisible (currentPage == Page::community);
    savePatchButton.setBounds (42, contentTop + 248, 138, 32);
    importPatchButton.setBounds (190, contentTop + 248, 138, 32);
}

void AnthemAudioProcessorEditor::positionDial (const juce::String& id, juce::Rectangle<int> bounds)
{
    if (auto* dial = dialById[id]) { dial->setBounds (bounds); dial->setVisible (true); }
}

void AnthemAudioProcessorEditor::positionChoice (const juce::String& id, juce::Rectangle<int> bounds)
{
    if (auto* choice = choiceById[id]) { choice->setBounds (bounds); choice->setVisible (true); }
}

void AnthemAudioProcessorEditor::hideAllControls()
{
    for (auto& dial : dials) dial->setVisible (false);
    for (auto& choice : choices) choice->setVisible (false);
}

void AnthemAudioProcessorEditor::setPage (Page page)
{
    currentPage = page;
    browserButton.setColour (juce::TextButton::textColourOffId, page == Page::browser ? textMain : textMuted);
    communityButton.setColour (juce::TextButton::textColourOffId, page == Page::community ? textMain : textMuted);
    designerButton.setColour (juce::TextButton::textColourOffId, page == Page::designer ? textMain : textMuted);
    effectsButton.setColour (juce::TextButton::textColourOffId, page == Page::effects ? textMain : textMuted);
    searchBox.setVisible (false); presetViewport->setVisible (false);
    for (auto& button : categoryButtons) button->setVisible (false);
    savePatchButton.setVisible (false); importPatchButton.setVisible (false);
    hideAllControls();
    resized(); repaint();
}

void AnthemAudioProcessorEditor::setDesignerMode (DesignerMode mode)
{
    designerMode = mode;
    modeSynthButton.setColour (juce::TextButton::textColourOffId, mode == DesignerMode::synth ? cyan : textMuted);
    modeSeqButton.setColour (juce::TextButton::textColourOffId, mode == DesignerMode::seq ? cyan : textMuted);
    modeModButton.setColour (juce::TextButton::textColourOffId, mode == DesignerMode::mod ? cyan : textMuted);
    modeGlobalButton.setColour (juce::TextButton::textColourOffId, mode == DesignerMode::global ? cyan : textMuted);
    hideAllControls(); resized(); repaint();
}

void AnthemAudioProcessorEditor::setCategory (const juce::String& category)
{
    selectedCategory = category;
    for (std::size_t i = 0; i < categoryButtons.size(); ++i)
    {
        categoryButtons[i]->setColour (juce::TextButton::buttonColourId, i == 0 && category == "All" ? juce::Colour (0xff20272b) : panel);
        const juce::String visibleName = categoryButtons[i]->getButtonText();
        const bool aliasMatch = (category == "Plucks" && visibleName == "PLUCK")
                             || (category == "Leads" && visibleName == "LEAD");
        const bool active = aliasMatch || visibleName.equalsIgnoreCase (category);
        categoryButtons[i]->setColour (juce::TextButton::textColourOffId, active ? cyan : textMuted);
    }
    refreshPresets();
}

void AnthemAudioProcessorEditor::refreshPresets()
{
    if (presetGrid != nullptr) presetGrid->setFilter (selectedCategory, searchText);
    if (presetGrid != nullptr) presetGrid->setSelectedName (processor.getCurrentPresetName());
}

void AnthemAudioProcessorEditor::refreshPresetTitle()
{
    currentPresetLabel.setText (processor.getCurrentPresetName(), juce::dontSendNotification);
    if (presetGrid != nullptr) presetGrid->setSelectedName (processor.getCurrentPresetName());
}

void AnthemAudioProcessorEditor::updateFilterModeButtons()
{
    const int mode = juce::roundToInt (processor.getParameterSnapshot()[AnthemAudioProcessor::filterMode]);
    const std::array<int, 4> values { 0, 2, 3, 4 };
    for (std::size_t i = 0; i < filterModeButtons.size(); ++i)
    {
        const bool active = i == 0 ? mode <= 1 : mode == values[i];
        filterModeButtons[i]->setColour (juce::TextButton::textColourOffId, active ? cyan : textMuted);
    }
}

void AnthemAudioProcessorEditor::selectRelativePreset (int delta)
{
    const auto& presets = processor.getPresets();
    int current = 0;
    for (int i = 0; i < (int) presets.size(); ++i)
        if (presets[(std::size_t) i].name == processor.getCurrentPresetName()) { current = i; break; }
    current = (current + delta + (int) presets.size()) % (int) presets.size();
    processor.loadPreset (presets[(std::size_t) current]);
    refreshPresetTitle();
}

void AnthemAudioProcessorEditor::showPresetMenu()
{
    juce::PopupMenu menu;
    int item = 1;
    const auto& presets = processor.getPresets();
    for (const auto& preset : presets)
    {
        menu.addItem (item, preset.name + "   ·   " + preset.category, true, preset.name == processor.getCurrentPresetName());
        ++item;
    }
    auto safeThis = juce::Component::SafePointer<AnthemAudioProcessorEditor> (this);
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&presetMenuButton), [safeThis, item] (int result)
    {
        if (safeThis != nullptr && result > 0 && result < item)
        {
            const auto& presetsNow = safeThis->processor.getPresets();
            safeThis->processor.loadPreset (presetsNow[(std::size_t) (result - 1)]);
            safeThis->refreshPresetTitle();
        }
    });
}

void AnthemAudioProcessorEditor::chooseGeneratorStyle()
{
    juce::PopupMenu menu;
    for (int i = 0; i < 4; ++i)
    {
        const juce::String styles[] { "Atlanta", "Dark", "Melodic", "Bounce" };
        menu.addItem (i + 1, styles[i], true, generatorStyle == styles[i]);
    }
    menu.addItem (5, "Current family", true, generatorStyle == "Current");
    auto safeThis = juce::Component::SafePointer<AnthemAudioProcessorEditor> (this);
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&generatorStyleButton), [safeThis] (int result)
    {
        const juce::String styles[] { "Atlanta", "Dark", "Melodic", "Bounce", "Current" };
        if (safeThis != nullptr && result > 0 && result <= 5)
        {
            safeThis->generatorStyle = styles[result - 1];
            safeThis->generatorStyleButton.setButtonText (safeThis->generatorStyle.toUpperCase() + "  ⌄");
        }
    });
}

void AnthemAudioProcessorEditor::exportPatch()
{
    fileChooser = std::make_unique<juce::FileChooser> ("Export ANTHEM patch", juce::File::getSpecialLocation (juce::File::userDocumentsDirectory), "*.anthem");
    auto safeThis = juce::Component::SafePointer<AnthemAudioProcessorEditor> (this);
    fileChooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
        [safeThis] (const juce::FileChooser& chooser)
        {
            if (safeThis == nullptr) return;
            auto file = chooser.getResult();
            if (file == juce::File()) return;
            if (file.getFileExtension().isEmpty()) file = file.withFileExtension (".anthem");
            juce::MemoryBlock state;
            safeThis->processor.getStateInformation (state);
            file.replaceWithData (state.getData(), state.getSize());
        });
}

void AnthemAudioProcessorEditor::importPatch()
{
    fileChooser = std::make_unique<juce::FileChooser> ("Import ANTHEM patch", juce::File::getSpecialLocation (juce::File::userDocumentsDirectory), "*.anthem");
    auto safeThis = juce::Component::SafePointer<AnthemAudioProcessorEditor> (this);
    fileChooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [safeThis] (const juce::FileChooser& chooser)
        {
            if (safeThis == nullptr) return;
            const auto file = chooser.getResult();
            if (! file.existsAsFile()) return;
            juce::MemoryBlock state;
            if (file.loadFileAsData (state))
            {
                safeThis->processor.setStateInformation (state.getData(), (int) state.getSize());
                safeThis->refreshPresetTitle();
            }
        });
}

void AnthemAudioProcessorEditor::timerCallback()
{
    if (presetGrid != nullptr) presetGrid->setSelectedName (processor.getCurrentPresetName());
    updateFilterModeButtons();
}

