#include "SondyLookAndFeel.h"

SondyLookAndFeel::SondyLookAndFeel()
{
    // Set up the theme colors - dark, scientific theme
    themeColors.background = juce::Colour(0xFF1A1A1A);      // Almost black
    themeColors.darkBackground = juce::Colour(0xFF141414);  // Darker background for contrast
    themeColors.panelBackground = juce::Colour(0xFF232323); // Slightly lighter for panels
    themeColors.accent = juce::Colour(0xFF2C9AFF);          // Bright blue accent
    themeColors.highlight = juce::Colour(0xFF52BEFF);       // Lighter blue for highlights
    themeColors.text = juce::Colour(0xFFE6E6E6);            // Off-white text
    themeColors.dimText = juce::Colour(0xFF9E9E9E);         // Dimmed text for less important items
    themeColors.controlFill = juce::Colour(0xFF3A3A3A);     // Control background
    themeColors.border = juce::Colour(0xFF454545);          // Border color
    themeColors.meterBackground = juce::Colour(0xFF1D1D1D); // Dark meter background
    themeColors.meterForeground = juce::Colour(0xFF2C9AFF); // Blue meter fill
    themeColors.gridLines = juce::Colour(0xFF454545);       // Grid lines color
    
    // Apply the theme colors to the LookAndFeel
    setColour(juce::ResizableWindow::backgroundColourId, themeColors.background);
    setColour(juce::DocumentWindow::backgroundColourId, themeColors.background);
    
    // Labels
    setColour(juce::Label::textColourId, themeColors.text);
    setColour(juce::Label::outlineColourId, juce::Colours::transparentBlack);
    
    // Sliders
    setColour(juce::Slider::backgroundColourId, themeColors.controlFill);
    setColour(juce::Slider::thumbColourId, themeColors.accent);
    setColour(juce::Slider::trackColourId, themeColors.accent.withAlpha(0.6f));
    setColour(juce::Slider::rotarySliderFillColourId, themeColors.accent);
    setColour(juce::Slider::rotarySliderOutlineColourId, themeColors.controlFill);
    setColour(juce::Slider::textBoxTextColourId, themeColors.text);
    setColour(juce::Slider::textBoxBackgroundColourId, themeColors.darkBackground);
    setColour(juce::Slider::textBoxOutlineColourId, themeColors.border);
    setColour(juce::Slider::textBoxHighlightColourId, themeColors.accent.withAlpha(0.2f));
    
    // Buttons
    setColour(juce::TextButton::buttonColourId, themeColors.controlFill);
    setColour(juce::TextButton::buttonOnColourId, themeColors.accent);
    setColour(juce::TextButton::textColourOffId, themeColors.text);
    setColour(juce::TextButton::textColourOnId, themeColors.text);
}

void SondyLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, 
                                        float sliderPos, float rotaryStartAngle, float rotaryEndAngle, 
                                        juce::Slider& slider)
{
    // Define dimensions
    const auto radius = juce::jmin(width, height) * 0.38f;
    const auto centerX = x + width * 0.5f;
    const auto centerY = y + height * 0.5f;
    const auto radian = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    
    // Draw outer shadow for depth
    const auto outerRadius = radius * 1.15f;
    g.setGradientFill(juce::ColourGradient(
        themeColors.darkBackground.darker(0.5f), centerX, centerY,
        themeColors.darkBackground.darker(0.8f), centerX - outerRadius, centerY - outerRadius,
        true));
    g.fillEllipse(centerX - outerRadius, centerY - outerRadius, outerRadius * 2.0f, outerRadius * 2.0f);
    
    // Draw main background (with subtle gradient)
    const auto mainRadius = radius * 1.1f;
    g.setGradientFill(juce::ColourGradient(
        themeColors.controlFill.brighter(0.1f), centerX, centerY - mainRadius * 0.5f,
        themeColors.controlFill.darker(0.2f), centerX, centerY + mainRadius * 0.5f,
        false));
    g.fillEllipse(centerX - mainRadius, centerY - mainRadius, mainRadius * 2.0f, mainRadius * 2.0f);
    
    // Draw outer rim (edge highlight)
    g.setColour(themeColors.border.brighter(0.1f));
    g.drawEllipse(centerX - mainRadius, centerY - mainRadius, mainRadius * 2.0f, mainRadius * 2.0f, 1.0f);
    
    // Draw value arc background track
    g.setColour(themeColors.darkBackground);
    const auto trackRadius = radius * 0.8f;
    const auto trackThickness = radius * 0.2f;
    
    juce::Path trackPath;
    trackPath.addCentredArc(centerX, centerY, trackRadius, trackRadius, 0.0f, 
                            rotaryStartAngle, rotaryEndAngle, true);
    
    g.strokePath(trackPath, juce::PathStrokeType(trackThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    
    // Draw value arc with gradient
    const auto arcRadius = radius * 0.8f;
    const auto arcThickness = radius * 0.2f;
    
    juce::Path valueArc;
    valueArc.addCentredArc(centerX, centerY, arcRadius, arcRadius, 0.0f, 
                           rotaryStartAngle, rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle),
                           true);
    
    const juce::Colour arcStartColor = themeColors.accent.brighter(0.2f);
    const juce::Colour arcEndColor = themeColors.accent;
    
    g.setGradientFill(juce::ColourGradient(
        arcStartColor, centerX + arcRadius * std::cos(rotaryStartAngle), centerY + arcRadius * std::sin(rotaryStartAngle),
        arcEndColor, centerX + arcRadius * std::cos(rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle)), 
        centerY + arcRadius * std::sin(rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle)),
        false));
    
    g.strokePath(valueArc, juce::PathStrokeType(arcThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    
    // Add a glow effect to the arc
    g.setColour(themeColors.accent.withAlpha(0.3f));
    g.strokePath(valueArc, juce::PathStrokeType(arcThickness + 3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    
    // Draw pointer
    const auto pointerThickness = radius * 0.13f;
    const auto pointerLength = radius * 0.8f;
    
    juce::Path pointerPath;
    pointerPath.addRoundedRectangle(-pointerThickness * 0.5f, -radius * 0.2f, pointerThickness, pointerLength, pointerThickness * 0.3f);
    
    // Apply gradient to pointer
    g.setGradientFill(juce::ColourGradient(
        themeColors.accent.brighter(0.2f), 0.0f, -radius * 0.2f,
        themeColors.accent.darker(0.2f), 0.0f, pointerLength - radius * 0.2f,
        false));
    
    g.fillPath(pointerPath, juce::AffineTransform::rotation(radian).translated(centerX, centerY));
    
    // Add center point
    const float centerPointSize = radius * 0.15f;
    g.setGradientFill(juce::ColourGradient(
        themeColors.accent.brighter(0.3f), centerX - centerPointSize * 0.3f, centerY - centerPointSize * 0.3f,
        themeColors.accent.darker(0.3f), centerX + centerPointSize * 0.3f, centerY + centerPointSize * 0.3f,
        true));
    g.fillEllipse(centerX - centerPointSize * 0.5f, centerY - centerPointSize * 0.5f, centerPointSize, centerPointSize);
}

void SondyLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label)
{
    // Clear the background if needed
    if (label.isOpaque())
    {
        g.fillAll(label.findColour(juce::Label::backgroundColourId));
    }
    
    // Get the text to draw
    const auto text = label.getText();
    
    // Set the font
    const auto font = label.getFont();
    g.setFont(font);
    
    // Set the color and draw the text
    g.setColour(label.findColour(juce::Label::textColourId));
    g.drawText(text, 0, 0, label.getWidth(), label.getHeight(), label.getJustificationType(), false);
}

void SondyLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button, 
                                           const juce::Colour& backgroundColour,
                                           bool shouldDrawButtonAsHighlighted, 
                                           bool shouldDrawButtonAsDown)
{
    const auto cornerSize = 3.0f;
    const auto bounds = button.getLocalBounds().toFloat().reduced(0.5f, 0.5f);
    
    juce::Colour baseColor = button.getToggleState() ? themeColors.accent : themeColors.controlFill;
    
    if (shouldDrawButtonAsDown)
        baseColor = baseColor.darker(0.1f);
    else if (shouldDrawButtonAsHighlighted)
        baseColor = baseColor.brighter(0.1f);
    
    // Draw with gradient for depth
    g.setGradientFill(juce::ColourGradient(
        baseColor.brighter(0.1f), bounds.getX(), bounds.getY(),
        baseColor.darker(0.1f), bounds.getX(), bounds.getBottom(),
        false));
    g.fillRoundedRectangle(bounds, cornerSize);
    
    // Draw outer edge
    g.setColour(shouldDrawButtonAsDown ? themeColors.border.darker(0.1f) : themeColors.border);
    g.drawRoundedRectangle(bounds, cornerSize, 1.0f);
}

void SondyLookAndFeel::fillResizableWindowBackground(juce::Graphics& g, int w, int h, 
                                                   const juce::BorderSize<int>& border,
                                                   juce::ResizableWindow& window)
{
    // Fill the main background
    g.fillAll(themeColors.background);
    
    // Draw a border around the window
    g.setColour(themeColors.border);
    g.drawRect(0, 0, w, h, 1);
} 