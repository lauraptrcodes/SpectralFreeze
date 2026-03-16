/*
  ==============================================================================

    Spectogram.h
    Created: 16 Jan 2026 7:00:44pm
    Author:  laura

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "FFTProcessor.h"

//==============================================================================
/*
*/
class Spectogram  : public juce::Component, private Timer
{
public:
    Spectogram(FFTProcessor&, juce::AudioProcessorValueTreeState& apvts, const juce::String& parameterID);
    ~Spectogram() override;

    void paint (juce::Graphics&) override;

    static constexpr auto fftOrder = 10;
    static constexpr auto fftSize = 1 << fftOrder; 
    static constexpr float minDb = -80.0f;
    static constexpr float maxDb = 0.0f;

    void drawNextColumn(const std::vector<float>& mags);

private:
    void timerCallback() override;
    juce::Image spectogramImage;
    
    FFTProcessor& fftProcessor;
    std::vector<float> magnitudes;
    juce::Colour getColourForLevel(float level);

    juce::AudioProcessorValueTreeState& parameters;
    juce::String fftParameterID;
    bool isFFTEnabled() const;
    bool drawingCurrentFrame = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Spectogram)
};
