/*
  ==============================================================================

    SpectralDelay.h
    Created: 29 Dec 2025 10:40:28am
    Author:  laura

  ==============================================================================
*/

#pragma once
#include <complex>

class SpectralDelay
{
public:
    using Complex = std::complex<float>;
    
    SpectralDelay() = default;
    
    void prepare(int maxDelayFrames, int bandSize) {
        delayBuffer.resize(maxDelayFrames);
        for (auto& frame : delayBuffer) {
            frame.resize(bandSize);
            std::fill(frame.begin(), frame.end(), 0.0f);
        }

        writeIndex = 0;
        this->bandSize = bandSize;
    }

    void reset()
    {
        for (auto& frame : delayBuffer)
            std::fill(frame.begin(), frame.end(), 0.0f);

        writeIndex = 0;
    }

    void write(int bin, int binStart, const float input)
    {
        int binIndex = bin - (binStart - 1);
        delayBuffer[writeIndex][binIndex] = input;
    }

    const float& read(int bin, int binStart, int delayFrames) const
    {
        int binIndex = bin - (binStart - 1);

        int index = (writeIndex - 1) - delayFrames;
        if (index < 0)
            index += (int)delayBuffer.size();

        return delayBuffer[index][binIndex];
    } 

    void advance()
    {
        writeIndex = (writeIndex + 1) % delayBuffer.size();
    }

    //crossfading
    bool delayTimeChanging = false;
    const int fadeLength = 4;

    int fadeCounter = 0;
    int previousDelayFrames = 0;
    int targetDelayFrames = 0;

    void initCrossfade(int delayFrames, int targetFrames) {
        fadeCounter = fadeLength;
        previousDelayFrames = delayFrames;
        targetDelayFrames = targetFrames;

        delayTimeChanging = true;
    }

    float crossfadeDelayFrames(int bin, int binStart) {
        
        if (fadeCounter > 0) {
            float alpha = 1.0f - (float)fadeCounter / fadeLength;

            auto& a = read(bin, binStart, previousDelayFrames);
            auto& b = read(bin, binStart, targetDelayFrames);

            auto interpolated = a * (1.0f - alpha) + b * alpha;
            return interpolated;
        }
        else {
            delayTimeChanging = false;
            return read(bin, binStart, targetDelayFrames);
        }
    }

private:
    std::vector<std::vector<float>> delayBuffer;
    int writeIndex = 0;
    int binPos = 0;
    int bandSize = 0;
};