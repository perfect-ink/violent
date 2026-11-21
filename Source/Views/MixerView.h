#pragma once
#include <JuceHeader.h>
#include "../PluginProcessor.h"
#include "SharedComponents.h"

//==============================================================================
/** One channel strip in the mixer: name, live level meter, and Level and
    Pan knobs bound to that generator's parameters. */
class MixerChannel : public juce::Component, private juce::Timer
{
public:
    MixerChannel (ViolentAudioProcessor& p, int generatorIdx);

    void resized() override;
    void paint (juce::Graphics&) override;

private:
    void timerCallback() override { repaint(); }

    ViolentAudioProcessor& processor;
    int generator;

    juce::Label  nameLabel;
    LabelledKnob levelKnob { "Level", ViolentColours::accent };
    LabelledKnob panKnob { "Pan", ViolentColours::yellow };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MixerChannel)
};

//==============================================================================
/** Mixer strip along the bottom — one channel per active generator. */
class MixerPanel : public juce::Component
{
public:
    static constexpr int PANEL_H = 172;

    explicit MixerPanel (ViolentAudioProcessor& p);
    void resized() override;
    void paint (juce::Graphics& g) override;

    void rebuild();

private:
    ViolentAudioProcessor& processor;
    juce::Label sectionLabel;
    std::array<std::unique_ptr<MixerChannel>, MAX_GENERATORS> channels;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MixerPanel)
};

