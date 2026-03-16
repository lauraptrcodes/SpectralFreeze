/*
  ==============================================================================

    FFTProcessor.h
    Created: 8 Dec 2025 4:26:38pm
    Author:  laura

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>;
#include "SpectralDelay.h";

using namespace juce;

class FFTProcessor {
public:
    FFTProcessor();
    void reset();
    int getLatencyInSamples();
    float processSample(float& sample, bool freezeBypassed, bool delayBypassed);
    void processBlock(float* data, int numSamples, bool freezeBypassed, bool delayBypassed);

    void startFreeze();
    void setFreezeEnabled(bool enabled);
    void setTriggerFreeze(bool freeze);
    bool getTriggerFreeze() { return triggerFreeze; };

    void initGrid(int bandCountIndex, float delayTime, float feedback, float delayMix, double sr, int delayMode = 0);
    void setDelayTime(float delayTime, double sampleRate, int mode = 0);
    void setFeedback(float feedback);
    void setDelayMix(float delayMix);
    void setMix(float mix);
    float getMix() { return globalMix; }
    float getDelayMix() { return delayMix; }
    float getFeedback() { return feedback; }
    float getDelayTime() { return delayTime; }
    void setBandCountIndex(int index) { bandCountIndex = index; }
    int getBandCountIndex() { return bandCountIndex; } 
    void setBlur(float blur) { phaseBlur = blur; }
    float getBlur() { return phaseBlur; }

    static constexpr int fftOrder = 10;
    static constexpr int fftSize = 1 << fftOrder;
    static constexpr int overlap = 4;
    static constexpr int hopSize = fftSize / overlap;
    static constexpr int numBins = fftSize / 2 + 1;    // 513 bins
    static constexpr float windowCorrection = 2.0f / 3.0f;

    static constexpr int bandCountValues[] = { 1, 8, 16, 32, 64 };

    bool isReseted;
    bool delayEnabled = true;
    
    struct Band {
        //fft bin range
        int binStart = 0;
        int binEnd = 0;

        //delay
        int delayFrames = 0;
        float feedback = 0.0f;
        float mix = 0.5f;

        SpectralDelay delay;
    };
    std::vector<Band> bands;
    Band* getBandIndex(int bin);

    enum DelayMode {
        Linear,
        Random
    };

    std::vector<float> magnitudes;
    std::vector<float> frozenMagnitudes;

    bool nextFrameReady = false;
    bool gridEnabled = false;

    const std::vector<float>& getFFTData() const;
    void pullFFTData(std::vector<float>& destination);

private:
    void processFrame(bool freezeBypassed, bool delayBypassed);
    void processSpectrum(float* data, int numBins, std::vector<float>& scopedData);
    void pushSpectrogramFrame(const std::vector<float>& mags);

    float blurPhase(float randPhase, float livePhase, float t);

    juce::dsp::FFT fft;
    juce::dsp::WindowingFunction<float> window;
    std::array<float, fftSize * 2> fftData;
    std::array<float, fftSize> inputFifo;
    std::array<float, fftSize> outputFifo;

    int count = 0;
    int pos = 0;

    int bandCountIndex = 1;
    int numBands = 0;

    bool freezeEnabled;
    bool triggerFreeze;

    double fMin = 20.;
    double fMax = 20500.;

    //spectrogram
    std::vector<float> buffer1;
    std::vector<float> buffer2;
    std::vector<float> scopedData;

    std::atomic<std::vector<float>*> writeBuffer;
    std::atomic<std::vector<float>*> readBuffer;

    float globalMix = 0.5f;
    float delayMix = 0.5f;
    float feedback = 0.0f;
    float delayTime = 0.0f;
    float phaseBlur = 0.5f;

    juce::Random& r = juce::Random::getSystemRandom();
};