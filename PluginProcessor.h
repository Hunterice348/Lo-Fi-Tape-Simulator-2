#pragma once
#include <JuceHeader.h>
#include <random>

class LoFiTapeSimulatorAudioProcessor : public juce::AudioProcessor
{
public:
    LoFiTapeSimulatorAudioProcessor();
    ~LoFiTapeSimulatorAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // === ДОБАВЛЕНО: обязательные методы для JUCE 8 ===
    double getTailLengthSeconds() const override { return 0.0; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    // =================================================

    juce::AudioProcessorValueTreeState apvts;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    void applyDrive(juce::AudioBuffer<float>& buffer, float drive);
    void applyFilter(juce::AudioBuffer<float>& buffer, float cutoff);
    void applyWowFlutter(juce::AudioBuffer<float>& buffer, float amount);
    void applyNoise(juce::AudioBuffer<float>& buffer, float amount);

    juce::dsp::IIR::Filter<float> leftFilter, rightFilter;
    std::vector<std::vector<float>> delayBuffers;
    int delayWritePos = 0;
    int maxDelaySamples = 4410;
    float lfoPhase = 0.0f;
    double currentSampleRate = 44100.0;
    float lastCutoff = 0.0f;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> driveSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> mixSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> filterCutoffSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> wowSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> noiseSmoothed;

    juce::AudioBuffer<float> dryBuffer;

    std::mt19937 gen;
    std::uniform_real_distribution<float> noiseDist;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LoFiTapeSimulatorAudioProcessor)
};
