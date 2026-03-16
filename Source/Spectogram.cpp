/*
  ==============================================================================

    Spectogram.cpp
    Created: 16 Jan 2026 7:00:44pm
    Author:  laura

  ==============================================================================
*/

#include <JuceHeader.h>
#include "Spectogram.h"

//==============================================================================
Spectogram::Spectogram(FFTProcessor& fftp, juce::AudioProcessorValueTreeState& apvts,
    const juce::String& parameterID) : fftProcessor(fftp), spectogramImage(juce::Image::RGB, 64, 64, true), parameters(apvts)
    , fftParameterID(parameterID)
{
    auto* param = parameters.getRawParameterValue(fftParameterID);
    bool fftEnabled = param != nullptr && *param > 0.5f;
    magnitudes.resize(fftSize / 2 + 1);
    drawingCurrentFrame = false;
    startTimerHz(30);
}

Spectogram::~Spectogram()
{
    stopTimer();
}

void Spectogram::paint (juce::Graphics& g)
{
    g.drawImage(spectogramImage, getLocalBounds().toFloat());
}

void Spectogram::timerCallback()
{    
    if (!isFFTEnabled()) {
        return;
    }
    
    else if (fftProcessor.nextFrameReady && !drawingCurrentFrame) {
        const auto& fftData = fftProcessor.getFFTData();
        drawNextColumn(fftData);
        repaint();
    }
}

void Spectogram::drawNextColumn(const std::vector<float>& magnitudes)
{
    drawingCurrentFrame = true;
    auto imageHeight = spectogramImage.getHeight();
    auto imageWidth = spectogramImage.getWidth();
    auto rightEdge = imageWidth - 1;

    juce::Image newImage(juce::Image::ARGB, imageWidth, imageHeight, true);
    juce::Graphics g(newImage);
    spectogramImage.moveImageSection(0, 0, 1, 0, rightEdge, imageHeight);

    for (auto y = 1; y < imageHeight; ++y) {

        int binIndex = juce::jmap(y, 0, imageHeight, 0, fftProcessor.numBins);
        float level = magnitudes[binIndex];

         float normalised = juce::jmap(level, -60.0f, 0.0f, 0.0f, 1.0f);
        normalised = juce::jlimit(0.0f, 1.0f, normalised);
        
        juce::Colour colour = getColourForLevel(normalised);
        spectogramImage.setPixelAt(rightEdge, imageHeight - 1 - y, colour);
    }

    drawingCurrentFrame = false;
}

juce::Colour Spectogram::getColourForLevel(float level)
{
    // magma heatmap

    if (level < 0.2f)
    {
        // Dunkelviolett (0,0,4) -> Lila (50,20,80)
        juce::Colour c1(0, 0, 4);
        juce::Colour c2(50, 20, 80);
        return c1.interpolatedWith(c2, level * 5.0f);
    }
    else if (level < 0.4f)
    {
        // Lila (50,20,80) -> Magenta-Rot (130,30,100)
        juce::Colour c1(50, 20, 80);
        juce::Colour c2(130, 30, 100);
        return c1.interpolatedWith(c2, (level - 0.2f) * 5.0f);
    }
    else if (level < 0.6f)
    {
        // Magenta-Rot (130,30,100) -> Orange-Rot (210,70,60)
        juce::Colour c1(130, 30, 100);
        juce::Colour c2(210, 70, 60);
        return c1.interpolatedWith(c2, (level - 0.4f) * 5.0f);
    }
    else if (level < 0.8f)
    {
        // Orange-Rot (210,70,60) -> Orange-Gelb (250,160,60)
        juce::Colour c1(210, 70, 60);
        juce::Colour c2(250, 160, 60);
        return c1.interpolatedWith(c2, (level - 0.6f) * 5.0f);
    }
    else
    {
        // Orange-Gelb (250,160,60) -> Hellgelb-Weiß (252,250,245)
        juce::Colour c1(250, 160, 60);
        juce::Colour c2(252, 250, 245);
        return c1.interpolatedWith(c2, (level - 0.8f) * 5.0f);
    }
}

bool Spectogram::isFFTEnabled() const
{
    auto* param = parameters.getRawParameterValue(fftParameterID);
    return param != nullptr && *param > 0.5f;
}
