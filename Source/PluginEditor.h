/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Spectogram.h"
#include "sfLookAndFeel.h"

//#include <juce_audio_processors/utilities/juce_AudioProcessorValueTreeState.h>

//==============================================================================
/**
*/
class SpectralFreezeAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    SpectralFreezeAudioProcessorEditor (SpectralFreezeAudioProcessor&, juce::AudioProcessorValueTreeState& vts);
    ~SpectralFreezeAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    SpectralFreezeAudioProcessor& audioProcessor;

    juce::AudioProcessorValueTreeState& valueTreeState;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> freezeAttachment;
    juce::ToggleButton freezeButton;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> retriggerModeAttachement;
    juce::ComboBox retriggerModeDropDown;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> retriggerRateAttachement;
    juce::Slider retriggerRateSlider;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> bandCountAttachement;
    juce::ComboBox bandCountDropDown;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> delayModeAttachement;
    juce::ComboBox delayModeDropDown;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> delayTimeAttachement;
    juce::Slider delayTimeSlider;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> feedbackAttachement;
    juce::Slider feedbackSlider;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> delayMixAttachement;
    juce::Slider delayMixSlider;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachement;
    juce::Slider mixSlider;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> blurAttachement;
    juce::Slider blurSlider;

    juce::Label retriggerModeLabel;
    juce::Label fftSizeLabel;
    juce::Label dryWetLabel;
    juce::Label bandCountLabel;
    juce::Label delayModeLabel;
    juce::Label delayTimeLabel;
    juce::Label delayMixLabel;
    juce::Label feedbackLabel;
    juce::Label retriggerRateLabel;
    juce::Label blurLabel;


    void configLabel(juce::Label& l, juce::Component& attachedComp, String text);
    Spectogram spectogram;
    SFLookAndFeel lnf;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectralFreezeAudioProcessorEditor)
};
