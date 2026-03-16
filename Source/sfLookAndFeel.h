/*
  ==============================================================================

    sfLookAndFeel.h
    Created: 25 Jan 2026 2:48:22pm
    Author:  laura

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class SFLookAndFeel : public juce::LookAndFeel_V4 {
public:
    SFLookAndFeel();
    void drawRotarySlider(juce::Graphics& g,
        int x, int y, int width, int height,
        float sliderPosProportional,
        float rotaryStartAngle,
        float rotaryEndAngle,
        juce::Slider&) override;
    void drawToggleButton(juce::Graphics& g,
        juce::ToggleButton& button,
        bool shouldDrawButtonAsHighlighted,
        bool shouldDrawButtonAsDown) override;
    void drawComboBox(juce::Graphics& g,
        int width, int height,
        bool isButtonDown,
        int buttonX, int buttonY,
        int buttonW, int buttonH,
        juce::ComboBox& box) override;
    void drawPopupMenuItem(juce::Graphics& g,
        const juce::Rectangle<int>& area,
        bool isSeparator,
        bool isActive,
        bool isHighlighted,
        bool isTicked,
        bool hasSubMenu,
        const juce::String& text,
        const juce::String& shortcutKeyText,
        const juce::Drawable* icon,
        const juce::Colour* textColour) override;

private:
    std::unique_ptr<juce::Drawable> knob;
    std::unique_ptr<juce::Drawable> toggleOn, toggleOff;
    std::unique_ptr<juce::Drawable> comboboxbg, comboboxarrow;
};