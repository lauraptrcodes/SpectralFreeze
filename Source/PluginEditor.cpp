/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


namespace UIColours
{

    static const juce::Colour accent = juce::Colour::fromString("#E03131");
    static const juce::Colour accentSoft = juce::Colour::fromString("#CCFF6A00");

    static const juce::Colour textPrimary = juce::Colour::fromString("#FFFFFF");
    static const juce::Colour textMuted = juce::Colour::fromString("#FFADADAD");
}


//==============================================================================
SpectralFreezeAudioProcessorEditor::SpectralFreezeAudioProcessorEditor (SpectralFreezeAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p), valueTreeState(vts), spectogram(p.getFFTProcessor(), vts, "freeze")
{
    juce::LookAndFeel::setDefaultLookAndFeel(&lnf);

    freezeButton.setButtonText("Freeze");
    freezeButton.setLookAndFeel(&lnf);
    addAndMakeVisible(freezeButton);
    freezeAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "freeze", freezeButton));

    retriggerModeDropDown.addItem("Live", 1);
    retriggerModeDropDown.addItem("Synced", 2);
    retriggerModeDropDown.addItem("Random", 3);

    retriggerModeAttachement.reset(new juce::AudioProcessorValueTreeState::ComboBoxAttachment(valueTreeState, "retriggerMode", retriggerModeDropDown));
    addAndMakeVisible(retriggerModeDropDown);

    retriggerRateAttachement.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, "retriggerRate", retriggerRateSlider));
    retriggerRateSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    retriggerRateSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible(retriggerRateSlider);

    bandCountAttachement.reset(new juce::AudioProcessorValueTreeState::ComboBoxAttachment(valueTreeState, "bandCount", bandCountDropDown));
    bandCountDropDown.addItemList(juce::StringArray{"1", "8", "16", "32", "64" }, 1);
    addAndMakeVisible(bandCountDropDown);

    delayModeDropDown.addItem("linear", 1);
    delayModeDropDown.addItem("random", 2);
    delayModeDropDown.setSelectedId(1);
    delayModeAttachement.reset(new juce::AudioProcessorValueTreeState::ComboBoxAttachment(valueTreeState, "delayMode", delayModeDropDown));
    addAndMakeVisible(delayModeDropDown);

    delayTimeAttachement.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, "delayTime", delayTimeSlider));
    delayTimeSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    delayTimeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible(delayTimeSlider);

    feedbackAttachement.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, "feedback", feedbackSlider));
    feedbackSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    feedbackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible(feedbackSlider);

    delayMixAttachement.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, "delayMix", delayMixSlider));
    delayMixSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    delayMixSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible(delayMixSlider);

    mixAttachement.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, "mix", mixSlider));
    mixSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    mixSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible(mixSlider);

    blurAttachement.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(valueTreeState, "phaseBlur", blurSlider));
    blurSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    blurSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible(blurSlider);

    configLabel(retriggerModeLabel, retriggerModeDropDown, "mode");
    configLabel(retriggerRateLabel, retriggerRateSlider, "rate");
    configLabel(dryWetLabel, mixSlider, "dry/wet");
    configLabel(bandCountLabel, bandCountDropDown, "bands");
    configLabel(delayModeLabel, delayModeDropDown, "delay mode");
    configLabel(delayTimeLabel, delayTimeSlider, "time");
    configLabel(delayMixLabel, delayMixSlider, "delay mix");
    configLabel(feedbackLabel, feedbackSlider, "feedback");
    configLabel(blurLabel, blurSlider, "spectral blur");

    addAndMakeVisible(spectogram);
    setSize (760, 360);
}

SpectralFreezeAudioProcessorEditor::~SpectralFreezeAudioProcessorEditor()
{
    LookAndFeel::setDefaultLookAndFeel(nullptr);
}

//==============================================================================
void SpectralFreezeAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromString("#FF121212"));
    g.setColour (juce::Colour::fromString("#FFff8383"));
    g.setFont (juce::FontOptions (15.0f));

    auto tl = getLocalBounds().getTopLeft();
    auto tr = getLocalBounds().getTopRight();
    g.drawText("retrigger", tl.getX() + 36, tl.getY() + 36, 80, 20, juce::Justification::left);
    g.drawText("band delay", tr.getX() - (36 * 9), tr.getY() + 36, 80, 20, juce::Justification::left);
}

void SpectralFreezeAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    auto padding = 36;
    auto bottomleft = area.getBottomLeft();
    auto topLeft = area.getTopLeft();
    auto topRight = area.getTopRight();
    auto bottomRight = area.getBottomRight();
    freezeButton.setBounds(topLeft.getX() + padding * 5, bottomleft.getY() - padding * 2, 80, 36);
    retriggerModeDropDown.setBounds(topLeft.getX() + padding, topLeft.getY() + padding * 2.5, 80, 36);
    retriggerRateSlider.setBounds(topLeft.getX() + padding, topLeft.getY() + padding * 4.5, 100, 120);
    bandCountDropDown.setBounds(topRight.getX() - padding * 6, topRight.getY() + padding * 2.5, 80, 36);
    delayModeDropDown.setBounds(topRight.getX() - padding * 9, topRight.getY() + padding * 2.5, 80, 36);
    delayTimeSlider.setBounds(bottomRight.getX() - padding * 9, bottomRight.getY() - padding * 4, 100, 120);
    feedbackSlider.setBounds(bottomRight.getX() - padding * 6, bottomRight.getY() - padding * 4, 100, 120);
    mixSlider.setBounds(bottomRight.getX() - padding * 3, topRight.getY() + padding, 100, 120);
    delayMixSlider.setBounds(bottomRight.getX() - padding * 3, bottomRight.getY() - padding * 4, 100, 120);
    blurSlider.setBounds(bottomleft.getX() + padding * 8, bottomleft.getY() - padding * 4, 100, 120);

    auto center = area.getCentre();

    int sWidth = 200;
    int sHeight = 160;
    spectogram.setBounds(topLeft.getX() + padding * 5, topLeft.getY() + padding, sWidth, sHeight);
}

void SpectralFreezeAudioProcessorEditor::configLabel(juce::Label& l, juce::Component& attachedComp, String text)
{
    l.setColour(juce::Label::textColourId, UIColours::textMuted);
    l.setText(text, juce::dontSendNotification);
    l.setJustificationType(juce::Justification::left);
    l.attachToComponent(&attachedComp, false);
}

