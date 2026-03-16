/*
  ==============================================================================

    FFTProcessor.cpp
    Created: 8 Dec 2025 4:26:38pm
    Author:  laura

  ==============================================================================
*/

#include "FFTProcessor.h"
#include <cmath>;


FFTProcessor::FFTProcessor() :
    fft(fftOrder),
    window(fftSize + 1, juce::dsp::WindowingFunction<float>::WindowingMethod::hann, false), 
    triggerFreeze(false), freezeEnabled(false), isReseted(true)
{
    magnitudes.resize(numBins);
    frozenMagnitudes.resize(numBins);
    
    //für spectrogram
    buffer1.resize(numBins);
    buffer2.resize(numBins);
    scopedData.resize(numBins);

    writeBuffer.store(&buffer1);
    readBuffer.store(&buffer2);
}

void FFTProcessor::reset()
{
    count = 0;
    pos = 0;
    std::fill(inputFifo.begin(), inputFifo.end(), 0.0f);
    std::fill(outputFifo.begin(), outputFifo.end(), 0.0f);
    freezeEnabled = false;
    triggerFreeze = false;

    for (auto& band : bands) {
        band.delay.reset();
    }

    isReseted = true;
}

int FFTProcessor::getLatencyInSamples()
{
    return fftSize;
}

float FFTProcessor::processSample(float& sample, bool freezeBypassed, bool delayBypassed)
{
    inputFifo[pos] = sample;   

    float outputSample = outputFifo[pos];
    outputFifo[pos] = 0.0f;

    pos += 1;
    if (pos == fftSize) {
        pos = 0;
    }

    count += 1;
    if (count == hopSize) {
        count = 0;
        processFrame(freezeBypassed, delayBypassed);
    }

    return outputSample;
}

void FFTProcessor::processBlock(float* data, int numSamples, bool freezeBypassed, bool delayBypassed)
{

    for (int sample = 0; sample < numSamples; ++sample) {
        data[sample] = data[sample] * (1.0f - globalMix) + processSample(data[sample], freezeBypassed, delayBypassed) * globalMix;
    }
}

void FFTProcessor::startFreeze()
{
    triggerFreeze = true;
    isReseted = false;
}

void FFTProcessor::setFreezeEnabled(bool enabled)
{
    freezeEnabled = enabled;
}

void FFTProcessor::setTriggerFreeze(bool freeze)
{
    triggerFreeze = freeze;
}

void FFTProcessor::initGrid(int bandCountIndex, float delayTime, float feedback, float delayMix, double sr, int delayMode)
{
    gridEnabled = true;
    fMax = sr / 2.;
    numBands = bandCountValues[bandCountIndex];
    float ratio = pow(fMax / fMin, 1.0f / numBands);
    float delayTimePerBand = std::abs(delayTime) / numBands;

    bands.clear();

    for (int i = 0; i < numBands; ++i) {
        Band b;
        float f0 = fMin * pow(ratio, i);
        float f1 = fMin * pow(ratio, i + 1);

        b.binStart = ceil(f0 * fftSize / sr);
        b.binEnd = floor(f1 * fftSize / sr);
       
        int maxDelayFrames = sr / hopSize;
        int bandSize = b.binEnd - b.binStart + 1;        

        b.delay.prepare(maxDelayFrames, bandSize);
        if (b.binStart <= b.binEnd) bands.push_back(b);

    }
    setFeedback(feedback);
    setDelayMix(delayMix);
    setDelayTime(delayTime, sr, delayMode);
}

void FFTProcessor::setDelayTime(float delayTime, double sampleRate, int mode)
{
    this->delayTime = delayTime;
    for (int i = 0; i < bands.size(); i++){
        Band& b = bands[i];

        if (!b.delay.delayTimeChanging) {
            int targetDelayFrames;
            float delayTimePerBand = std::abs(delayTime) / numBands;

            float bandDelayTime = 0.0f;
            if (static_cast<DelayMode>(mode) == Linear) {
                bandDelayTime = delayTime >= 0.f ? (delayTimePerBand * (i + 1)) : (delayTimePerBand * (numBands - i));
            }
            // Random delayMode
            else {
                auto& r = juce::Random::getSystemRandom();
                bandDelayTime = r.nextFloat() * std::abs(delayTime);
            }

            targetDelayFrames = bandDelayTime * sampleRate / hopSize;

            b.delay.initCrossfade(b.delayFrames, targetDelayFrames);
            b.delayFrames = targetDelayFrames;
        }
        
    }
}

void FFTProcessor::setFeedback(float feedback)
{
    this->feedback = feedback;
    for (auto& band : bands) {
        band.feedback = feedback;
    }
}

void FFTProcessor::setDelayMix(float delayMix)
{
    this->delayMix = delayMix;
    for (auto& band : bands) {
        band.mix = delayMix;
    }
}

void FFTProcessor::setMix(float mix)
{
    globalMix = mix;
}

FFTProcessor::Band* FFTProcessor::getBandIndex(int bin)
{
    for (auto& band : bands) {
        if (band.binStart <= bin && bin <= band.binEnd) {
            return &band;
        }
    }
    return nullptr;
}

const std::vector<float>& FFTProcessor::getFFTData() const
{
    return *readBuffer.load(std::memory_order_acquire);
}

void FFTProcessor::pullFFTData(std::vector<float>& destination)
{
    destination.resize(numBins);

    auto* readPtr = readBuffer.load();
    std::copy(readPtr->begin(), readPtr->end(), destination.begin());
}

void FFTProcessor::processFrame(bool freezeBypassed, bool delayBypassed)
{
    const float* inputPtr = inputFifo.data();
    float* fftPtr = fftData.data();

    std::memcpy(fftPtr, inputPtr + pos, (fftSize - pos) * sizeof(float));
    if (pos > 0) {
        std::memcpy(fftPtr + fftSize - pos, inputPtr, pos * sizeof(float));
    }
    window.multiplyWithWindowingTable(fftPtr, fftSize);

    if (!freezeBypassed || !delayBypassed) {
        fft.performRealOnlyForwardTransform(fftPtr, true);  //true = nur positive freqs(schneller)
        processSpectrum(fftPtr, numBins, scopedData);
        fft.performRealOnlyInverseTransform(fftPtr);        
    }
    pushSpectrogramFrame(scopedData);


    window.multiplyWithWindowingTable(fftPtr, fftSize);

    for (int i = 0; i < fftSize; ++i) {
        fftPtr[i] *= windowCorrection; // gain correction
    }

    for (int i = 0; i < pos; ++i) {
        outputFifo[i] += fftData[i + fftSize - pos];
    }
    for (int i = 0; i < fftSize - pos; ++i) {
        outputFifo[i + pos] += fftData[i];
    }

}

void FFTProcessor::processSpectrum(float* data, int numBins, std::vector<float>& scopedData)
{
    auto* cdata = reinterpret_cast<std::complex<float>*>(data);
    Band* currentBand = getBandIndex(1);

    for (int i = 0; i < numBins; ++i) {
        float magnitude = std::abs(cdata[i]);
        float phase = std::arg(cdata[i]);

        Band* newBand = getBandIndex(i + 1);

        if (currentBand != newBand) {
            currentBand->delay.advance(); 
            if (currentBand->delay.delayTimeChanging) currentBand->delay.fadeCounter--;
            currentBand = newBand;
        }

        float delayed = 0.0f;
        if (currentBand) {

            if (freezeEnabled) {
                magnitude = frozenMagnitudes[i];                
            }
            if (delayEnabled) {
                if (currentBand->delay.delayTimeChanging) {
                    delayed = currentBand->delay.crossfadeDelayFrames(i, currentBand->binStart);
                }
                else {
                    delayed = currentBand->delay.read(i, currentBand->binStart, currentBand->delayFrames);
                }
            }
                        
            float fb = delayed * currentBand->feedback;
            float wet = magnitude + fb;

            if (triggerFreeze ) {
                frozenMagnitudes[i] = magnitude;
            }

            if (delayEnabled) {
                currentBand->delay.write(i, currentBand->binStart, wet);
            }
            
            float outMag = ((1.f - currentBand->mix) * magnitude) + (currentBand->mix * fb);
            float randPhase = r.nextFloat() * 2.0f * float_Pi;
            float outPhase = blurPhase(randPhase, phase, phaseBlur);

            cdata[i] = std::polar(outMag, outPhase);
        }
        //for spectrogram
        scopedData[i] = juce::Decibels::gainToDecibels(magnitude) - juce::Decibels::gainToDecibels(numBins);
    }
    if (triggerFreeze) {
        freezeEnabled = true;
    } 

}

void FFTProcessor::pushSpectrogramFrame(const std::vector<float>& mags)
{
    nextFrameReady = false;

    auto* writePtr = writeBuffer.load();
    std::copy(mags.begin(), mags.end(), writePtr->begin());
    auto* readPtr = readBuffer.load();
    writeBuffer.store(readPtr);
    readBuffer.store(writePtr);

    nextFrameReady = true;
}

float FFTProcessor::blurPhase(float randPhase, float livePhase, float t)
{
    float delta = randPhase - livePhase;

    delta = std::atan2(std::sin(delta), std::cos(delta));
    return livePhase + t * delta;
}

