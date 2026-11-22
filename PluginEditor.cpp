#include "PluginProcessor.h"
#include "PluginEditor.h"

LoFiTapeSimulatorAudioProcessorEditor::LoFiTapeSimulatorAudioProcessorEditor(
    LoFiTapeSimulatorAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(600, 400);
    setResizable(false, false);

    startTimerHz(60);

    auto setupSlider = [this](juce::Slider& slider,
                              juce::Label& label,
                              const juce::String& text,
                              std::unique_ptr<SliderAttachment>& attachment,
                              const juce::String& paramID)
    {
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
        slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xFFD4A574));
        slider.setColour(juce::Slider::thumbColourId, juce::Colour(0xFF8B6F47));
        addAndMakeVisible(slider);

        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setColour(juce::Label::textColourId, juce::Colour(0xFFE8D5C4));
        addAndMakeVisible(label);

        attachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, paramID, slider);
    };

    setupSlider(driveSlider,  driveLabel,  "DRIVE",  driveAttachment,  "DRIVE");
    setupSlider(filterSlider, filterLabel, "FILTER", filterAttachment, "FILTER");
    setupSlider(wowSlider,    wowLabel,    "WOW",    wowAttachment,    "WOW");
    setupSlider(noiseSlider,  noiseLabel,  "NOISE",  noiseAttachment,  "NOISE");
    setupSlider(mixSlider,    mixLabel,    "MIX",    mixAttachment,    "MIX");
}

LoFiTapeSimulatorAudioProcessorEditor::~LoFiTapeSimulatorAudioProcessorEditor()
{
    stopTimer();
}

void LoFiTapeSimulatorAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xFF3D2817), 0.0f, 0.0f,
                                           juce::Colour(0xFF1A0F0A), 0.0f,
                                           static_cast<float>(getHeight()), false));
    g.fillAll();

    g.setColour(juce::Colour(0xFFD4A574));
    g.setFont(32.0f);
    g.drawText("LO-FI TAPE", titleBounds, juce::Justification::centred);

    g.setFont(16.0f);
    g.drawText("SIMULATOR", subtitleBounds, juce::Justification::centred);

    g.setColour(juce::Colour(0xFF8B6F47));
    g.fillEllipse(leftReelBounds.toFloat());

    g.setColour(juce::Colour(0xFF2C1810));
    for (int i = 0; i < 6; ++i)
    {
        float angle = reelAngle + i * juce::MathConstants<float>::pi / 3.0f;
        float cx = leftReelBounds.getCentreX();
        float cy = leftReelBounds.getCentreY();
        g.drawLine(cx + std::cos(angle) * 15.0f, cy + std::sin(angle) * 15.0f,
                   cx + std::cos(angle) * 35.0f, cy + std::sin(angle) * 35.0f, 3.0f);
    }

    g.setColour(juce::Colour(0xFF8B6F47));
    g.fillEllipse(rightReelBounds.toFloat());

    g.setColour(juce::Colour(0xFF2C1810));
    for (int i = 0; i < 6; ++i)
    {
        float angle = -reelAngle + i * juce::MathConstants<float>::pi / 3.0f;
        float cx = rightReelBounds.getCentreX();
        float cy = rightReelBounds.getCentreY();
        g.drawLine(cx + std::cos(angle) * 15.0f, cy + std::sin(angle) * 15.0f,
                   cx + std::cos(angle) * 35.0f, cy + std::sin(angle) * 35.0f, 3.0f);
    }
}

void LoFiTapeSimulatorAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(20);

    titleBounds = area.removeFromTop(65);
    subtitleBounds = area.removeFromTop(30);

    auto reelArea = area.removeFromTop(100);
    leftReelBounds = reelArea.removeFromLeft(100).reduced(10);
    reelArea.removeFromLeft(20);
    rightReelBounds = reelArea.removeFromLeft(100).reduced(10);

    auto sliderArea = area.removeFromTop(170);

    int sliderWidth = sliderArea.getWidth() / 5;

    auto positionSlider = [&sliderWidth](juce::Slider& slider,
                                         juce::Label& label,
                                         juce::Rectangle<int>& area)
    {
        auto sliderBounds = area.removeFromLeft(sliderWidth).reduced(10);
        label.setBounds(sliderBounds.removeFromTop(30));
        slider.setBounds(sliderBounds);
    };

    positionSlider(driveSlider,  driveLabel,  sliderArea);
    positionSlider(filterSlider, filterLabel, sliderArea);
    positionSlider(wowSlider,    wowLabel,    sliderArea);
    positionSlider(noiseSlider,  noiseLabel,  sliderArea);
    positionSlider(mixSlider,    mixLabel,    sliderArea);
}

void LoFiTapeSimulatorAudioProcessorEditor::timerCallback()
{
    reelAngle += 0.05f;
    if (reelAngle > juce::MathConstants<float>::twoPi)
        reelAngle -= juce::MathConstants<float>::twoPi;

    if (!leftReelBounds.isEmpty())  repaint(leftReelBounds);
    if (!rightReelBounds.isEmpty()) repaint(rightReelBounds);
}
