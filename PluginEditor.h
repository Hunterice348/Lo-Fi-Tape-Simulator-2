#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class LoFiTapeSimulatorAudioProcessorEditor : public juce::AudioProcessorEditor,
                                              private juce::Timer
{
public:
    explicit LoFiTapeSimulatorAudioProcessorEditor(LoFiTapeSimulatorAudioProcessor&);
    ~LoFiTapeSimulatorAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    LoFiTapeSimulatorAudioProcessor& audioProcessor;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    juce::Slider driveSlider, filterSlider, wowSlider, noiseSlider, mixSlider;
    juce::Label  driveLabel,  filterLabel,  wowLabel,  noiseLabel,  mixLabel;

    std::unique_ptr<SliderAttachment> driveAttachment;
    std::unique_ptr<SliderAttachment> filterAttachment;
    std::unique_ptr<SliderAttachment> wowAttachment;
    std::unique_ptr<SliderAttachment> noiseAttachment;
    std::unique_ptr<SliderAttachment> mixAttachment;

    juce::Rectangle<int> titleBounds;
    juce::Rectangle<int> subtitleBounds;
    juce::Rectangle<int> leftReelBounds;
    juce::Rectangle<int> rightReelBounds;

    float reelAngle = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LoFiTapeSimulatorAudioProcessorEditor)
};
