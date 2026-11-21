#include "SharedComponents.h"

//==============================================================================
// ViolentUI
//==============================================================================
ViolentUI::ViolentUI()
{
    ViolentColours::apply (ViolentColours::make (palette(), getAccent(), isDark()));
}

namespace
{
    // Colour IDs that Violent's views set explicitly from ViolentColours.
    constexpr int remappedColourIds[] = {
        juce::Label::textColourId, juce::Label::backgroundColourId,
        juce::TextButton::buttonColourId, juce::TextButton::buttonOnColourId,
        juce::TextButton::textColourOffId, juce::TextButton::textColourOnId,
        juce::Slider::rotarySliderFillColourId, juce::Slider::thumbColourId,
        juce::Slider::trackColourId, juce::Slider::textBoxTextColourId,
    };

    void remapColours (juce::Component& c, const ViolentColours::Set& from, const ViolentColours::Set& to)
    {
        const std::pair<juce::Colour, juce::Colour> pairs[] = {
            { from.background, to.background }, { from.surface, to.surface }, { from.overlay, to.overlay },
            { from.text, to.text }, { from.subtext, to.subtext }, { from.accent, to.accent },
            { from.green, to.green }, { from.red, to.red }, { from.yellow, to.yellow },
            { from.blue, to.blue }, { from.teal, to.teal },
        };

        for (auto id : remappedColourIds)
        {
            if (! c.isColourSpecified (id))
                continue;

            const auto colour = c.findColour (id);
            for (const auto& [oldColour, newColour] : pairs)
            {
                // Match on RGB so colours set with withAlpha() follow too.
                if (colour.withAlpha (1.0f) == oldColour.withAlpha (1.0f))
                {
                    c.setColour (id, newColour.withAlpha (colour.getFloatAlpha()));
                    break;
                }
            }
        }

        for (auto* child : c.getChildren())
            remapColours (*child, from, to);
    }
}

void ViolentUI::refreshColours (juce::Component& root)
{
    const auto before = ViolentColours::current();
    const auto after  = ViolentColours::make (palette(), getAccent(), isDark());
    ViolentColours::apply (after);
    remapColours (root, before, after);
    root.sendLookAndFeelChange();
    root.repaint();
}

void ViolentUI::drawRotarySlider (juce::Graphics& g,
    int x, int y, int width, int height,
    float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
    juce::Slider& slider)
{
    // The Volta knob: thin track, value arc in the knob's own colour (set via
    // LabelledKnob's constructor or setAccentColour(), else the accent), flat disc.
    VoltaLookAndFeel::drawRotarySlider (g, x, y, width, height, sliderPos, rotaryStartAngle, rotaryEndAngle, slider);
}

void ViolentUI::paintControlShape (juce::Graphics& g, juce::Component& c, juce::Rectangle<float> b,
                                             bool isOn, bool isHovered)
{
    const float hover = animatedHoverAmount (c, isHovered);
    const float alpha = c.isEnabled() ? 1.0f : (float) VOLTA_OPACITY_DISABLED;

    // A component's own buttonOnColourId (e.g. set by GeneratorCard to its
    // per-generator accent) wins over the global accent if it's been set.
    juce::Colour on = c.findColour (juce::TextButton::buttonOnColourId, false);
    if (! on.isOpaque()) on = ViolentColours::accent;

    // Volta control: faint fill and hairline border; "on" is a light tint of the
    // control's colour rather than a solid block.
    const auto& p = palette();
    const float restFill  = (float) VOLTA_OPACITY_CONTROL_FILL;
    const float hoverFill = (float) VOLTA_OPACITY_CONTROL_FILL_HOVER;
    const float fill = restFill + (hoverFill - restFill) * hover;

    g.setColour ((isOn ? on.withAlpha (hoverFill + 0.06f * hover) : p.secondary.withAlpha (fill)).withMultipliedAlpha (alpha));
    g.fillRoundedRectangle (b, ViolentColours::cornerRadius);

    const float divider = (float) VOLTA_OPACITY_DIVIDER;
    const float borderAlpha = divider + (0.5f - divider) * hover;
    g.setColour ((isOn ? on.withAlpha (borderAlpha + 0.1f) : p.secondary.withAlpha (borderAlpha)).withMultipliedAlpha (alpha));
    g.drawRoundedRectangle (b, ViolentColours::cornerRadius, 1.0f);
}

void ViolentUI::drawToggleButton (juce::Graphics& g, juce::ToggleButton& btn,
                                            bool highlighted, bool)
{
    const bool on = btn.getToggleState();
    const auto b  = btn.getLocalBounds().toFloat().reduced (1.0f);
    paintControlShape (g, btn, b, on, highlighted);
    juce::Colour onColour = btn.findColour (juce::TextButton::buttonOnColourId, false);
    if (! onColour.isOpaque()) onColour = ViolentColours::accent;
    g.setColour ((on ? onColour : ViolentColours::text).withMultipliedAlpha (btn.isEnabled() ? 1.0f : (float) VOLTA_OPACITY_DISABLED));
    g.setFont (juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (12.0f).withStyle ("Medium")));
    g.drawFittedText (btn.getButtonText(), btn.getLocalBounds(), juce::Justification::centred, 1);
}

void ViolentUI::drawButtonBackground (juce::Graphics& g, juce::Button& btn,
                                                const juce::Colour&, bool highlighted, bool)
{
    const auto b  = btn.getLocalBounds().toFloat().reduced (1.0f);
    paintControlShape (g, btn, b, btn.getToggleState(), highlighted);
}

void ViolentUI::drawButtonText (juce::Graphics& g, juce::TextButton& btn, bool, bool)
{
    // "On" text takes the button's own colour, matching its tinted background.
    juce::Colour onColour = btn.findColour (juce::TextButton::buttonOnColourId, false);
    if (! onColour.isOpaque()) onColour = ViolentColours::accent;

    const bool on = btn.getToggleState();
    auto colour = on ? onColour : ViolentColours::text;
    if (! btn.isEnabled())
        colour = colour.withMultipliedAlpha ((float) VOLTA_OPACITY_DISABLED);

    g.setFont (getTextButtonFont (btn, btn.getHeight()));
    g.setColour (colour);
    g.drawFittedText (btn.getButtonText(), btn.getLocalBounds().reduced (4, 0), juce::Justification::centred, 1);
}

void ViolentUI::drawComboBox (juce::Graphics& g, int width, int height, bool,
                                        int, int, int, int, juce::ComboBox& box)
{
    const auto b = juce::Rectangle<float> (0.0f, 0.0f, (float) width, (float) height).reduced (1.0f);
    // ComboBox has an internal Label covering most of its area (the text
    // display), so a plain isMouseOver() — which excludes children by
    // default — only ever sees the small uncovered strip near the arrow.
    // Include children so hover covers the whole control. Disabled boxes
    // shouldn't show a hover state at all, same as a disabled text field.
    const bool hovered = box.isEnabled() && box.isMouseOver (true);
    paintControlShape (g, box, b, box.isPopupActive(), hovered);

    const float arrowCX = (float) width - 15.0f, arrowCY = (float) height * 0.5f;
    juce::Path chevron;
    chevron.startNewSubPath (arrowCX - 3.5f, arrowCY - 1.5f);
    chevron.lineTo (arrowCX, arrowCY + 2.0f);
    chevron.lineTo (arrowCX + 3.5f, arrowCY - 1.5f);
    g.setColour (ViolentColours::subtext);
    g.strokePath (chevron, juce::PathStrokeType (1.4f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

juce::Font ViolentUI::getLabelFont (juce::Label&)
{
    return juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (12.0f));
}

juce::Font ViolentUI::getTextButtonFont (juce::TextButton&, int buttonHeight)
{
    return juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (juce::jmin (12.0f, (float) buttonHeight * 0.6f)).withStyle ("Medium"));
}

juce::Font ViolentUI::getComboBoxFont (juce::ComboBox&)
{
    return juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (12.0f));
}

juce::Font ViolentUI::getPopupMenuFont()
{
    return juce::Font (juce::FontOptions().withName ("SF Pro Text").withHeight (12.0f));
}

float ViolentUI::animatedHoverAmount (juce::Component& c, bool isHovered)
{
    constexpr double durationMs = VOLTA_MOTION_DURATION * 1000.0;
    const double now = juce::Time::getMillisecondCounterHiRes();
    const float targetNow = isHovered ? 1.0f : 0.0f;

    auto& props = c.getProperties();
    const bool hasState = props.contains ("vHoverStart");

    double start  = hasState ? (double) props["vHoverStart"]  : now;
    float  from   = hasState ? (float) (double) props["vHoverFrom"]   : targetNow;
    float  target = hasState ? (float) (double) props["vHoverTarget"] : targetNow;

    const double elapsed = juce::jlimit (0.0, durationMs, now - start);
    const float  t = (float) (elapsed / durationMs);
    float progress = from + (target - from) * t;

    if (! hasState || ! juce::approximatelyEqual (target, targetNow))
    {
        // First time seeing this component, or the hover target just
        // flipped — (re)start the transition from wherever we are now, so
        // reversing direction mid-fade stays smooth instead of jumping.
        start = now;
        from = progress;
        target = targetNow;
        props.set ("vHoverStart", start);
        props.set ("vHoverFrom", (double) from);
        props.set ("vHoverTarget", (double) target);
    }

    if (! juce::approximatelyEqual (progress, target))
        registerForHoverAnimation (c);

    return progress;
}

void ViolentUI::registerForHoverAnimation (juce::Component& c)
{
    for (auto& sp : animatingComponents)
        if (sp.getComponent() == &c)
            return;

    animatingComponents.add (juce::Component::SafePointer<juce::Component> (&c));
    if (! isTimerRunning())
        startTimerHz (60);
}

void ViolentUI::timerCallback()
{
    const double now = juce::Time::getMillisecondCounterHiRes();

    for (int i = animatingComponents.size(); --i >= 0;)
    {
        auto* comp = animatingComponents.getReference (i).getComponent();
        if (comp == nullptr)
        {
            animatingComponents.remove (i);
            continue;
        }

        comp->repaint();

        auto& props = comp->getProperties();
        const double start = (double) props.getWithDefault ("vHoverStart", 0.0);
        if (now - start >= VOLTA_MOTION_DURATION * 1000.0)
            animatingComponents.remove (i);
    }

    if (animatingComponents.isEmpty())
        stopTimer();
}

//==============================================================================
// LabelledKnob
//==============================================================================
LabelledKnob::LabelledKnob (const juce::String& name, juce::Colour colour)
    : knobColour (colour)
{
    slider.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    slider.setColour (juce::Slider::rotarySliderFillColourId, colour);
    slider.setRepaintsOnMouseActivity (true);
    slider.setScrollWheelEnabled (true);
    slider.setWantsKeyboardFocus (true);   // arrow keys step the value
    slider.setTitle (name);                // spoken label for accessibility
    addAndMakeVisible (slider);

    nameLabel.setText (name, juce::dontSendNotification);
    nameLabel.setJustificationType (juce::Justification::centred);
    nameLabel.setColour (juce::Label::textColourId, ViolentColours::subtext);
    addAndMakeVisible (nameLabel);

    valueLabel.setJustificationType (juce::Justification::centred);
    valueLabel.setColour (juce::Label::textColourId, ViolentColours::text);
    valueLabel.setEditable (true, false, false);   // single click to type a value
    valueLabel.setMouseCursor (juce::MouseCursor::IBeamCursor);
    valueLabel.setTooltip ("Click to type a value");
    valueLabel.onEditorShow = [this]
    {
        if (auto* editor = valueLabel.getCurrentTextEditor())
            editor->setJustification (juce::Justification::centred);
    };
    valueLabel.onTextChange = [this] { commitTypedValue(); };
    addAndMakeVisible (valueLabel);

    slider.onValueChange = [this]
    {
        valueLabel.setText (slider.getTextFromValue (slider.getValue()),
                            juce::dontSendNotification);
    };

    startTimerHz (10);
}

void LabelledKnob::resized()
{
    auto a = getLocalBounds();
    nameLabel .setBounds (a.removeFromTop    (14));
    valueLabel.setBounds (a.removeFromBottom (13));
    slider    .setBounds (a);
}

void LabelledKnob::attachTo (juce::AudioProcessorValueTreeState& apvts, const juce::String& parameterID)
{
    paramID = parameterID;
    ownerProcessor = dynamic_cast<ViolentAudioProcessor*> (&apvts.processor);

    // Only generator-source and effect knobs are modulatable (matches
    // ViolentAudioProcessor::getModulatableParameters()'s own filter) - tag
    // the slider so the crosshair target-picker's rack-wide hit-test can
    // recognise it as a valid drop target.
    const bool isModulatable = (parameterID.startsWith ("gen_") && parameterID.contains ("_src_"))
                             || parameterID.startsWith ("fx");
    if (isModulatable)
        slider.getProperties().set ("modParamID", parameterID);

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        apvts, parameterID, slider);
    valueLabel.setText (slider.getTextFromValue (slider.getValue()), juce::dontSendNotification);

    // Double-click resets to the default the parameter itself defines.
    if (auto* parameter = apvts.getParameter (parameterID))
        slider.setDoubleClickReturnValue (true, parameter->convertFrom0to1 (parameter->getDefaultValue()));
}

void LabelledKnob::commitTypedValue()
{
    const auto typed = valueLabel.getText().trim();

    // Text that isn't a number leaves the value unchanged. Units are fine:
    // the parameter's own parser reads the leading number ("50%" == "50").
    if (typed.containsAnyOf ("0123456789"))
        slider.setValue (slider.getValueFromText (typed), juce::sendNotificationSync);   // setValue clamps to the range

    valueLabel.setText (slider.getTextFromValue (slider.getValue()), juce::dontSendNotification);
}

void LabelledKnob::timerCallback()
{
    if (ownerProcessor == nullptr || paramID.isEmpty()) return;

    bool isTarget = false;
    for (int m = 0; m < ownerProcessor->numModulators; ++m)
    {
        const auto& mod = ownerProcessor->modulators[(size_t) m];
        if (mod.enabled && mod.targetParamID == paramID) { isTarget = true; break; }
    }

    static const juce::Colour vibrantPurple { 0xffb350ff };
    slider.setColour (juce::Slider::rotarySliderFillColourId, isTarget ? vibrantPurple : knobColour);
}

//==============================================================================
// LevelMeter
//==============================================================================
void LevelMeter::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    const float w = b.getWidth() * 0.45f, gap = b.getWidth() - 2.0f * w;

    auto drawBar = [&] (float x, float level)
    {
        const float db   = juce::Decibels::gainToDecibels (juce::jmax (0.0001f, level));
        const float norm = juce::jlimit (0.0f, 1.0f, (db + 60.0f) / 60.0f);
        g.setColour (ViolentColours::surface);
        g.fillRect (x, b.getY(), w, b.getHeight());
        g.setColour (norm > 0.85f ? ViolentColours::red
                   : norm > 0.6f  ? ViolentColours::yellow : ViolentColours::green);
        g.fillRect (x, b.getBottom() - b.getHeight() * norm, w, b.getHeight() * norm);
    };
    drawBar (b.getX(),         processor.levelL.load (std::memory_order_relaxed));
    drawBar (b.getX() + w + gap, processor.levelR.load (std::memory_order_relaxed));
}

