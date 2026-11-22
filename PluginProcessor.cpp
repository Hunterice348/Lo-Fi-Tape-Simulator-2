#include "PluginProcessor.h"
#include "PluginEditor.h"

LoFiTapeSimulatorAudioProcessor::LoFiTapeSimulatorAudioProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout()),
      gen(std::random_device{}()),
      noiseDist(-1.0f, 1.0f)
{
}
LoFiTapeSimulatorAudioProcessor::~LoFiTapeSimulatorAudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout
LoFiTapeSimulatorAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"DRIVE", 1}, "Drive",
        juce::NormalisableRange<float>(0.0f, 100.0f), 20.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"FILTER", 1}, "Filter",
        juce::NormalisableRange<float>(200.0f, 20000.0f, 1.0f, 0.3f), 8000.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"WOW", 1}, "Wow & Flutter",
        juce::NormalisableRange<float>(0.0f, 100.0f), 30.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"NOISE", 1}, "Noise",
        juce::NormalisableRange<float>(0.0f, 100.0f), 10.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"MIX", 1}, "Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f), 50.0f));

    return { params.begin(), params.end() };
}

void LoFiTapeSimulatorAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    maxDelaySamples = static_cast<int>(sampleRate * 0.1) + 1;
    delayBuffers.resize(2);
    for (auto& buf : delayBuffers)
        buf.assign(static_cast<size_t>(maxDelaySamples), 0.0f);

    delayWritePos = 0;
    lfoPhase = 0.0f;
    lastCutoff = 0.0f;

    leftFilter.reset();
    rightFilter.reset();

    auto spec = juce::dsp::ProcessSpec();
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = 1;
    leftFilter.prepare(spec);
    rightFilter.prepare(spec);

    driveSmoothed.reset(sampleRate, 0.05);
    mixSmoothed.reset(sampleRate, 0.05);
    filterCutoffSmoothed.reset(sampleRate, 0.05);
    wowSmoothed.reset(sampleRate, 0.05);
    noiseSmoothed.reset(sampleRate, 0.05);

    dryBuffer.setSize(getTotalNumInputChannels(), samplesPerBlock);
}

void LoFiTapeSimulatorAudioProcessor::releaseResources() {}

void LoFiTapeSimulatorAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    dryBuffer.makeCopyOf(buffer, true);

    float driveTarget        = *apvts.getRawParameterValue("DRIVE")  / 100.0f;
    float filterCutoffTarget = *apvts.getRawParameterValue("FILTER");
    float wowTarget          = *apvts.getRawParameterValue("WOW")    / 100.0f;
    float noiseTarget        = *apvts.getRawParameterValue("NOISE")  / 100.0f;
    float mixTarget          = *apvts.getRawParameterValue("MIX")    / 100.0f;

    driveSmoothed.setTargetValue(driveTarget);
    filterCutoffSmoothed.setTargetValue(filterCutoffTarget);
    wowSmoothed.setTargetValue(wowTarget);
    noiseSmoothed.setTargetValue(noiseTarget);
    mixSmoothed.setTargetValue(mixTarget);

    float drive        = driveSmoothed.getNextValue();
    float filterCutoff = filterCutoffSmoothed.getNextValue();
    float wowAmount    = wowSmoothed.getNextValue();
    float noiseAmount  = noiseSmoothed.getNextValue();
    float mix          = mixSmoothed.getNextValue();

    applyWowFlutter(buffer, wowAmount);
    applyDrive(buffer, drive);
    applyFilter(buffer, filterCutoff);
    applyNoise(buffer, noiseAmount);

    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* dryData = dryBuffer.getReadPointer(channel);
        auto* wetData = buffer.getWritePointer(channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            wetData[sample] = dryData[sample] * (1.0f - mix) + wetData[sample] * mix;
    }
}

void LoFiTapeSimulatorAudioProcessor::applyDrive(juce::AudioBuffer<float>& buffer, float drive)
{
    if (drive < 0.001f) return;

    float gain = 1.0f + drive * 8.0f;

    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* data = buffer.getWritePointer(channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            data[sample] = std::tanh(data[sample] * gain) * 0.8f;
    }
}

void LoFiTapeSimulatorAudioProcessor::applyFilter(juce::AudioBuffer<float>& buffer, float cutoff)
{
    cutoff = juce::jlimit(20.0f, static_cast<float>(currentSampleRate * 0.45), cutoff);

    if (std::abs(cutoff - lastCutoff) > 5.0f)
    {
        leftFilter.reset();
        rightFilter.reset();
        lastCutoff = cutoff;
    }

    auto coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(
        currentSampleRate, cutoff, 0.707f);

    *leftFilter.coefficients  = *coefficients;
    *rightFilter.coefficients = *coefficients;

    const int numSamples = buffer.getNumSamples();
    for (int sample = 0; sample < numSamples; ++sample)
    {
        buffer.setSample(0, sample, leftFilter.processSample(buffer.getSample(0, sample)));
        if (buffer.getNumChannels() > 1)
            buffer.setSample(1, sample, rightFilter.processSample(buffer.getSample(1, sample)));
    }
}

void LoFiTapeSimulatorAudioProcessor::applyWowFlutter(juce::AudioBuffer<float>& buffer, float amount)
{
    if (amount < 0.001f || maxDelaySamples <= 1) return;

    if (delayWritePos >= maxDelaySamples) delayWritePos = 0;

    const double sampleRate = currentSampleRate;
    const float lfoFreq = 0.5f + amount * 3.0f;
    const float modulationDepth = amount * 0.015f * static_cast<float>(sampleRate);
    const int numChannels = buffer.getNumChannels();
    const int numSamples  = buffer.getNumSamples();

    for (int sample = 0; sample < numSamples; ++sample)
    {
        float lfo = std::sin(lfoPhase * juce::MathConstants<float>::twoPi);
        lfoPhase += lfoFreq / static_cast<float>(sampleRate);
        if (lfoPhase >= 1.0f) lfoPhase -= 1.0f;

        float delayMod = modulationDepth * (1.0f + lfo);
        delayMod = juce::jlimit(0.0f, static_cast<float>(maxDelaySamples - 2), delayMod);

        int readPos = delayWritePos - static_cast<int>(delayMod);
        if (readPos < 0) readPos += maxDelaySamples;

        int readPos2 = (readPos + 1) % maxDelaySamples;
        float frac = delayMod - std::floor(delayMod);

        for (int channel = 0; channel < numChannels; ++channel)
        {
            delayBuffers[channel][delayWritePos] = buffer.getSample(channel, sample);
            float s1 = delayBuffers[channel][readPos];
            float s2 = delayBuffers[channel][readPos2];
            buffer.setSample(channel, sample, s1 * (1.0f - frac) + s2 * frac);
        }

        delayWritePos = (delayWritePos + 1) % maxDelaySamples;
    }
}

void LoFiTapeSimulatorAudioProcessor::applyNoise(juce::AudioBuffer<float>& buffer, float amount)
{
    if (amount < 0.001f) return;

    const float noiseLevel = amount * 0.03f;

    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* data = buffer.getWritePointer(channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            data[sample] += noiseDist(gen) * noiseLevel;
    }
}

juce::AudioProcessorEditor* LoFiTapeSimulatorAudioProcessor::createEditor()
{
    return new LoFiTapeSimulatorAudioProcessorEditor(*this);
}

bool LoFiTapeSimulatorAudioProcessor::hasEditor() const { return true; }

const juce::String LoFiTapeSimulatorAudioProcessor::getName() const
{
    return "Lo-Fi Tape Simulator";
}

void LoFiTapeSimulatorAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void LoFiTapeSimulatorAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LoFiTapeSimulatorAudioProcessor();
}
