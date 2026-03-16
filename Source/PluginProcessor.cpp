/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SpectralFreezeAudioProcessor::SpectralFreezeAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    )
#endif
    , params(*this, nullptr, juce::Identifier("apvts"), {
            std::make_unique<juce::AudioParameterBool>("freeze", "Freeze", false),
            std::make_unique<juce::AudioParameterChoice>("retriggerMode", "RetriggerMode", StringArray{"Live", "Synced", "Random"}, 0),
            std::make_unique<juce::AudioParameterFloat>("retriggerRate", "RetriggerRate", juce::NormalisableRange<float>(0.1f, 20.0f, 0.001f, 0.5f), 1.0f, "Hz"),
            std::make_unique<juce::AudioParameterChoice>("bandCount", "BandCount",juce::StringArray { "1", "8", "16", "32", "64" }, 1),
            std::make_unique<juce::AudioParameterFloat>("delayTime", "DelayTime", juce::NormalisableRange<float>(-1.0f, 1.0f, 0.1f), 0.0f, "ms"),
            std::make_unique<juce::AudioParameterChoice>("delayMode", "DelayMode",juce::StringArray { "Linear", "Random"}, 0),
            std::make_unique<juce::AudioParameterFloat>("feedback", "Feedback", juce::NormalisableRange<float>(0.0f, 1.0f, 0.1f), 0.5f, "%"),
            std::make_unique<juce::AudioParameterFloat>("mix", "Mix", juce::NormalisableRange<float>(0.0f, 1.0f, 0.1f), 0.5f, "%"),
            std::make_unique<juce::AudioParameterFloat>("delayMix", "DelayMix", juce::NormalisableRange<float>(0.0f, 1.0f, 0.1f), 0.5f, "%"),
            std::make_unique<juce::AudioParameterFloat>("phaseBlur", "PhaseBlur", juce::NormalisableRange<float>(0.0f, 1.0f, 0.1f), 0.5f)
        })
{
    freezeParam = params.getRawParameterValue("freeze");
    retriggerModeParam = params.getRawParameterValue("retriggerMode");
    retriggerRateParam = params.getRawParameterValue("retriggerRate");
    bandCountParam = params.getRawParameterValue("bandCount");
    currentRetriggerMode = RetriggerMode::Live;
    delayTimeParam = params.getRawParameterValue("delayTime");
    delayMixParam = params.getRawParameterValue("delayMix");
    delayModeParam = params.getRawParameterValue("delayMode");
    feedbackParam = params.getRawParameterValue("feedback");
    mixParam = params.getRawParameterValue("mix");
    blurParam = params.getRawParameterValue("phaseBlur");

}

SpectralFreezeAudioProcessor::~SpectralFreezeAudioProcessor()
{
}

//==============================================================================
const juce::String SpectralFreezeAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SpectralFreezeAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SpectralFreezeAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SpectralFreezeAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SpectralFreezeAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SpectralFreezeAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SpectralFreezeAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SpectralFreezeAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SpectralFreezeAudioProcessor::getProgramName (int index)
{
    return {};
}

void SpectralFreezeAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SpectralFreezeAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    previousRetriggerRate = *retriggerRateParam;

    sr = sampleRate;
    samplesPerTrigger = static_cast<int>(sr / previousRetriggerRate);
    setLatencySamples(fft[0].getLatencyInSamples());

    fft[0].reset();
    fft[1].reset();
}

void SpectralFreezeAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SpectralFreezeAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}
#endif

void SpectralFreezeAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels() > 2 ? 2 : getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    auto numSamples = buffer.getNumSamples();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    updateParams(numSamples, totalNumInputChannels);

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    { 
        auto* channelData = buffer.getWritePointer (channel);
        fft[channel].processBlock(channelData, numSamples, freezeBypassed, false);
    }

}

//==============================================================================
bool SpectralFreezeAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SpectralFreezeAudioProcessor::createEditor()
{
    return new SpectralFreezeAudioProcessorEditor (*this, params);
}

//==============================================================================
void SpectralFreezeAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = params.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void SpectralFreezeAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(params.state.getType()))
            params.replaceState(juce::ValueTree::fromXml(*xmlState));
}

void SpectralFreezeAudioProcessor::updateParams(int numSamples, int totalNumInputChannels)
{
    freezeBypassed = *freezeParam < 0.5f;
    auto bandCountIndex = static_cast<int>(bandCountParam->load());
    float delayTime = delayTimeParam->load();
    float delayMix = delayMixParam->load();
    float feedback = feedbackParam->load();
    float mix = mixParam->load();
    int delMode = delayModeParam->load();
    float blur = blurParam->load();

    for (int channel = 0; channel < totalNumInputChannels; ++channel) {
        //wenn freeze laufen soll & noch nicht initialisiert wurde (bei freeze off -> on)
        if (!freezeBypassed && !fft[0].getTriggerFreeze()) {
            for (int channel = 0; channel < totalNumInputChannels; ++channel) {
                fft[channel].startFreeze();
            }
        }
        // wenn freeze stoppen soll & noch nicht resetet wurde (bei freeze on -> off)
        else if (freezeBypassed && !fft[0].isReseted) {
            for (int channel = 0; channel < totalNumInputChannels; ++channel) {
                fft[channel].reset();
            }
        }

        if (delMode != static_cast<int>(previousDelayMode)) {
            previousDelayMode = delMode;
        }

        if (fft[channel].getBandCountIndex() != 0 && !fft[channel].gridEnabled) {
            fft[channel].gridEnabled = true;
            fft[channel].setBandCountIndex(bandCountIndex);
            fft[channel].initGrid(bandCountIndex, delayTime, feedback, delayMix, sr, previousDelayMode);
        }

        if (bandCountIndex != fft[channel].getBandCountIndex()) {
            fft[channel].setBandCountIndex(bandCountIndex);
            fft[channel].initGrid(bandCountIndex, delayTime, feedback, delayMix, sr, previousDelayMode);
        }

        if (!juce::approximatelyEqual(fft[channel].getDelayTime(), delayTime)) {
            fft[channel].setDelayTime(delayTime, sr, previousDelayMode);
        }

        if (!juce::approximatelyEqual(fft[channel].getFeedback(), feedback)) {
            fft[channel].setFeedback(feedback);
        }

        if (!juce::approximatelyEqual(fft[channel].getMix(), mix)) {
            fft[channel].setMix(mix);
            fft[channel].delayEnabled = !juce::approximatelyEqual(fft[channel].getMix(), 0.0f);
        }

        if (!juce::approximatelyEqual(fft[channel].getDelayMix(), delayMix)) {
            fft[channel].setDelayMix(delayMix);
        }

        if (!juce::approximatelyEqual(fft[channel].getBlur(), blur)) {
            fft[channel].setBlur(blur);
        }
    }

    // retrigger
    auto mode = static_cast<RetriggerMode>(retriggerModeParam->load());

    if (!freezeBypassed && mode != RetriggerMode::Live) {
        auto rate = retriggerRateParam->load();
        if (mode != currentRetriggerMode || !juce::approximatelyEqual(rate, previousRetriggerRate)) {
            previousRetriggerRate = rate;
            initRetrigger(mode);
        }
        updateRetrigger(numSamples);
    }
}

void SpectralFreezeAudioProcessor::updateRetrigger(int numSamples)
{
    samplesUntilRetrigger -= numSamples;
    if (samplesUntilRetrigger <= 0.0)
    {
        if (currentRetriggerMode == RetriggerMode::Random) {
            auto& r = juce::Random::getSystemRandom();
            samplesPerTrigger = static_cast<int>(r.nextFloat() * (sr / previousRetriggerRate));
            samplesUntilRetrigger = samplesPerTrigger;
        }
        retriggerFreeze();       
        samplesUntilRetrigger = samplesPerTrigger;
    }
}

void SpectralFreezeAudioProcessor::retriggerFreeze()
{
    for (int i = 0; i < 2; ++i) {
        fft[i].reset();
    }
}

void SpectralFreezeAudioProcessor::initRetrigger(RetriggerMode mode)
{
    currentRetriggerMode = mode;

    if (mode == RetriggerMode::Synced) {
        samplesPerTrigger = static_cast<int>(sr / previousRetriggerRate);
    }
    else if (mode == RetriggerMode::Random) {
        auto& r = juce::Random::getSystemRandom();
        samplesPerTrigger = static_cast<int>(r.nextFloat() * (sr / previousRetriggerRate));
    }
    samplesUntilRetrigger = samplesPerTrigger;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SpectralFreezeAudioProcessor();
}
