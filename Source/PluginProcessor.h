/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>;
#include <complex>
#include "FFTProcessor.h"

//==============================================================================
/**
*/
class SpectralFreezeAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    SpectralFreezeAudioProcessor();
    ~SpectralFreezeAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    static constexpr auto fftOrder = 10;
    static constexpr auto fftSize = 1 << fftOrder;

    FFTProcessor& getFFTProcessor() { return fft[0]; }
private:
    void updateParams(int numSamples, int totalNumInputChannels);
    juce::AudioProcessorValueTreeState params;

    bool freezeBypassed = true;
    std::atomic<float>* freezeParam = nullptr;
    std::atomic<float>* retriggerModeParam = nullptr;
    float previousRetriggerRate;
    std::atomic<float>* retriggerRateParam = nullptr;
    std::atomic<float>* bandCountParam = nullptr;
    std::atomic<float>* delayTimeParam = nullptr;
    std::atomic<float>* delayMixParam = nullptr;
    float previousDelayMode;
    std::atomic<float>* delayModeParam = nullptr;
    std::atomic<float>* feedbackParam = nullptr;
    std::atomic<float>* mixParam = nullptr;
    std::atomic<float>* blurParam = nullptr;

    enum RetriggerMode {
        Live,
        Synced,
        Random
    };
    RetriggerMode currentRetriggerMode;

    void updateRetrigger(int numSamples);
    void retriggerFreeze();
    void initRetrigger(RetriggerMode mode = RetriggerMode::Synced);
    double sr;
    int samplesPerTrigger;
    int samplesUntilRetrigger = -1;

    bool gridEnabled = false;
   
    FFTProcessor fft[2];
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectralFreezeAudioProcessor)
};
