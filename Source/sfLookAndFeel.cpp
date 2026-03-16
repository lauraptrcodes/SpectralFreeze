/*
  ==============================================================================

    sfLookAndFeel.cpp
    Created: 25 Jan 2026 2:48:22pm
    Author:  laura

  ==============================================================================
*/

#include "sfLookAndFeel.h"

SFLookAndFeel::SFLookAndFeel()
{
    knob = juce::Drawable::createFromImageData(
        BinaryData::knob2_svg,
        BinaryData::knob2_svgSize
    );

    toggleOn = juce::Drawable::createFromImageData(
        BinaryData::freezeon_svg,
        BinaryData::freezeon_svgSize);

    toggleOff = juce::Drawable::createFromImageData(
        BinaryData::freezeoff_svg,
        BinaryData::freezeoff_svgSize);

    comboboxbg = juce::Drawable::createFromImageData(
        BinaryData::comboboxbg_svg,
        BinaryData::comboboxbg_svgSize);

    comboboxarrow = juce::Drawable::createFromImageData(
        BinaryData::comboboxarrow_svg,
        BinaryData::comboboxarrow_svgSize);
}

void SFLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle, juce::Slider&)
{
    auto bounds = juce::Rectangle<float>(x, y, width * 0.88f, height * 0.88f);

    float angle = juce::jmap(sliderPosProportional,
        rotaryStartAngle,
        rotaryEndAngle);

    auto transform = juce::AffineTransform()
        .translated(-bounds.getCentreX(), -bounds.getCentreY())
        .rotated(angle)
        .translated(bounds.getCentreX(), bounds.getCentreY());

    knob->draw(g, 1.0f, transform);
}

void SFLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown){
    auto bounds = button.getLocalBounds().toFloat();

    auto* drawable = button.getToggleState()
        ? toggleOff.get()
        : toggleOn.get();

    if (drawable != nullptr)
    {
        drawable->drawWithin(g,
            bounds,
            juce::RectanglePlacement::centred,
            1.0f);
    }
}

void SFLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown, int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox& box)
{
    auto bounds = juce::Rectangle<float>(0, 0, width, height);

    // background
    if (comboboxbg)
        comboboxbg->drawWithin(g,
            bounds,
            juce::RectanglePlacement::stretchToFit,
            1.0f);

    // arrow area
    auto arrowArea = juce::Rectangle<float>(
        (float)buttonX, (float)buttonY,
        (float)buttonW, (float)buttonH);

    float alpha = isButtonDown ? 0.8f : 1.0f;

    if (comboboxarrow)
        comboboxarrow->drawWithin(g,
            arrowArea.reduced(4),
            juce::RectanglePlacement::centred,
            alpha);
}

void SFLookAndFeel::drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area, bool isSeparator, bool isActive, bool isHighlighted, bool isTicked, bool hasSubMenu, const juce::String& text, const juce::String& shortcutKeyText, const juce::Drawable* icon, const juce::Colour* textColour)
{
    g.fillAll(juce::Colour::fromString("#FF121212"));
    if (isHighlighted) g.fillAll(juce::Colours::darkgrey);

    g.setColour(juce::Colours::white);
    g.drawText(text, area.reduced(8),
    juce::Justification::centredLeft);
}
