#include "WavetableEditor.h"
#include <cmath>

// WavetablePresetButton implementation
void WavetablePresetButton::paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    // Draw button background
    const auto bounds = getLocalBounds().toFloat().reduced(1.0f);
    const auto cornerSize = 3.0f;
    
    // Choose appropriate colors
    juce::Colour baseColour = juce::Colour(0xFF3A3A3A);
    if (shouldDrawButtonAsDown)
        baseColour = juce::Colour(0xFF1C8AFF);
    else if (shouldDrawButtonAsHighlighted)
        baseColour = juce::Colour(0xFF4A4A4A);
    else if (getToggleState())
        baseColour = juce::Colour(0xFF2C9AFF);
    
    // Fill button
    g.setColour(baseColour);
    g.fillRoundedRectangle(bounds, cornerSize);
    
    // Draw outline
    g.setColour(shouldDrawButtonAsDown ? juce::Colour(0xFF2C9AFF) : juce::Colour(0xFF5A5A5A));
    g.drawRoundedRectangle(bounds, cornerSize, 1.0f);
    
    // Draw the curve symbol based on curve type
    const float symbolMargin = 4.0f;
    const juce::Rectangle<float> symbolBounds = bounds.reduced(symbolMargin);
    juce::Path curvePath;
    
    switch (curveType)
    {
        case 0: // Linear
        {
            // Draw straight line from bottom-left to top-right
            curvePath.startNewSubPath(symbolBounds.getX(), symbolBounds.getBottom());
            curvePath.lineTo(symbolBounds.getRight(), symbolBounds.getY());
            break;
        }
        case 1: // Exponential
        {
            // Draw exponential curve
            curvePath.startNewSubPath(symbolBounds.getX(), symbolBounds.getBottom());
            
            for (float x = 0.0f; x <= 1.0f; x += 0.05f)
            {
                float y = 1.0f - std::pow(x, 2.0f); // Exponential curve
                curvePath.lineTo(
                    symbolBounds.getX() + x * symbolBounds.getWidth(),
                    symbolBounds.getY() + y * symbolBounds.getHeight()
                );
            }
            break;
        }
        case 2: // Logarithmic
        {
            // Draw logarithmic curve
            curvePath.startNewSubPath(symbolBounds.getX(), symbolBounds.getBottom());
            
            for (float x = 0.0f; x <= 1.0f; x += 0.05f)
            {
                float y = 1.0f - std::sqrt(x); // Logarithmic curve
                curvePath.lineTo(
                    symbolBounds.getX() + x * symbolBounds.getWidth(),
                    symbolBounds.getY() + y * symbolBounds.getHeight()
                );
            }
            break;
        }
        case 3: // S-Curve
        {
            // Draw S-curve
            curvePath.startNewSubPath(symbolBounds.getX(), symbolBounds.getBottom());
            
            for (float x = 0.0f; x <= 1.0f; x += 0.05f)
            {
                // Sigmoid function to create S-curve
                float y = 1.0f - (1.0f / (1.0f + std::exp(-10.0f * (x - 0.5f))));
                curvePath.lineTo(
                    symbolBounds.getX() + x * symbolBounds.getWidth(),
                    symbolBounds.getY() + y * symbolBounds.getHeight()
                );
            }
            break;
        }
    }
    
    // Draw the curve with appropriate styling
    g.setColour(juce::Colours::white);
    g.strokePath(curvePath, juce::PathStrokeType(1.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

WavetableEditor::WavetableEditor()
{
    // Initialize wavetable with a linear ramp
    for (int i = 0; i < wavetable.size(); ++i)
    {
        wavetable[i] = static_cast<float>(i) / static_cast<float>(wavetable.size() - 1);
    }
    
    // Set up preset buttons
    linearButton.setCurveType(0);
    expButton.setCurveType(1);
    logButton.setCurveType(2);
    sCurveButton.setCurveType(3);
    
    // Add buttons to component
    addAndMakeVisible(linearButton);
    addAndMakeVisible(expButton);
    addAndMakeVisible(logButton);
    addAndMakeVisible(sCurveButton);
    
    // Set up button callbacks
    linearButton.onClick = [this] { presetButtonClicked(0); };
    expButton.onClick = [this] { presetButtonClicked(1); };
    logButton.onClick = [this] { presetButtonClicked(2); };
    sCurveButton.onClick = [this] { presetButtonClicked(3); };
    
    setSize(300, 150);
}

void WavetableEditor::paint(juce::Graphics& g)
{
    // Fill background with dark color
    g.fillAll(juce::Colour(0xFF1D1D1D));
    
    // Create an inner shadow effect for depth
    const auto innerShadowBounds = getLocalBounds().reduced(1);
    g.setGradientFill(juce::ColourGradient(
        juce::Colour(0xFF161616), 2.0f, 2.0f,
        juce::Colour(0xFF1D1D1D), static_cast<float>(getWidth() / 2), static_cast<float>(getHeight() / 2),
        true));
    g.fillRect(innerShadowBounds);
    
    // Draw border with gradient effect
    g.setGradientFill(juce::ColourGradient(
        juce::Colour(0xFF555555), 0.0f, 0.0f,
        juce::Colour(0xFF383838), 0.0f, static_cast<float>(getHeight()),
        false));
    g.drawRect(getLocalBounds(), 1);
    
    // Draw second inner border
    g.setColour(juce::Colour(0xFF303030));
    g.drawRect(getLocalBounds().reduced(2), 1);
    
    // Draw grid - more grid lines for larger display
    g.setColour(juce::Colour(0xFF353535));
    
    // Draw horizontal grid lines
    const int numHorizontalLines = 10;
    for (int i = 0; i <= numHorizontalLines; ++i)
    {
        float y = getHeight() * (1.0f - static_cast<float>(i) / numHorizontalLines);
        float alpha = (i % 5 == 0) ? 0.5f : 0.25f; // Stronger lines every 5 steps
        g.setColour(juce::Colour(0xFF353535).withAlpha(alpha));
        g.drawLine(0.0f, y, static_cast<float>(getWidth()), y, 1.0f);
    }
    
    // Draw vertical grid lines
    const int numVerticalLines = 10;
    for (int i = 0; i <= numVerticalLines; ++i)
    {
        float x = getWidth() * static_cast<float>(i) / numVerticalLines;
        float alpha = (i % 5 == 0) ? 0.5f : 0.25f; // Stronger lines every 5 steps
        g.setColour(juce::Colour(0xFF353535).withAlpha(alpha));
        g.drawLine(x, 0.0f, x, static_cast<float>(getHeight()), 1.0f);
    }
    
    // Draw key grid lines with labels
    g.setColour(juce::Colour(0xFF9E9E9E));
    g.setFont(11.0f);
    
    // Draw 0% and 100% labels
    g.drawText("0%", 5, getHeight() - 20, 30, 15, juce::Justification::left, false);
    g.drawText("100%", getWidth() - 40, getHeight() - 20, 35, 15, juce::Justification::right, false);
    
    // Draw 0% and 100% vertical labels
    float valueY0 = wavetableValueToY(0.0f);
    float valueY1 = wavetableValueToY(1.0f);
    g.drawText("0", 5, static_cast<int>(valueY0) - 15, 15, 15, juce::Justification::left, false);
    g.drawText("1", 5, static_cast<int>(valueY1) - 15, 15, 15, juce::Justification::left, false);
    
    // Draw wavetable curve with gradient
    juce::Path path;
    bool pathStarted = false;
    
    for (int i = 0; i < wavetable.size(); ++i)
    {
        float x = wavetableIndexToX(i);
        float y = wavetableValueToY(wavetable[i]);
        
        if (!pathStarted)
        {
            path.startNewSubPath(x, y);
            pathStarted = true;
        }
        else
        {
            path.lineTo(x, y);
        }
    }
    
    // Create stroke with thicker line for larger display
    const float lineThickness = 3.0f;
    
    // Draw a shadow under the curve for depth
    g.setColour(juce::Colour(0xFF151515));
    g.strokePath(path, juce::PathStrokeType(lineThickness + 5.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    
    // Draw the glow effect
    g.setColour(juce::Colour(0xFF2C9AFF).withAlpha(0.35f));
    g.strokePath(path, juce::PathStrokeType(lineThickness + 8.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    
    // Draw the brighter glow effect (closer to the line)
    g.setColour(juce::Colour(0xFF2C9AFF).withAlpha(0.4f));
    g.strokePath(path, juce::PathStrokeType(lineThickness + 4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    
    // Draw the main curve with gradient
    juce::ColourGradient gradient(
        juce::Colour(0xFF4CAFFF), 0.0f, 0.0f,
        juce::Colour(0xFF1C8AFF), 0.0f, static_cast<float>(getHeight()),
        false);
    
    g.setGradientFill(gradient);
    g.strokePath(path, juce::PathStrokeType(lineThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    
    // Draw square handles at key points for easier editing on large display
    const float handleSize = 6.0f;
    g.setColour(juce::Colour(0xFFDDDDDD));
    
    // Draw handle at start and end points
    float startX = wavetableIndexToX(0);
    float startY = wavetableValueToY(wavetable[0]);
    float endX = wavetableIndexToX(wavetable.size() - 1);
    float endY = wavetableValueToY(wavetable[wavetable.size() - 1]);
    
    g.fillRect(startX - handleSize/2, startY - handleSize/2, handleSize, handleSize);
    g.fillRect(endX - handleSize/2, endY - handleSize/2, handleSize, handleSize);
    
    // Draw handles at quarter points
    const int quarterIdx = wavetable.size() / 4;
    const int midIdx = wavetable.size() / 2;
    const int threeQuarterIdx = 3 * wavetable.size() / 4;
    
    float quarterX = wavetableIndexToX(quarterIdx);
    float quarterY = wavetableValueToY(wavetable[quarterIdx]);
    g.fillRect(quarterX - handleSize/2, quarterY - handleSize/2, handleSize, handleSize);
    
    float midX = wavetableIndexToX(midIdx);
    float midY = wavetableValueToY(wavetable[midIdx]);
    g.fillRect(midX - handleSize/2, midY - handleSize/2, handleSize, handleSize);
    
    float threeQuarterX = wavetableIndexToX(threeQuarterIdx);
    float threeQuarterY = wavetableValueToY(wavetable[threeQuarterIdx]);
    g.fillRect(threeQuarterX - handleSize/2, threeQuarterY - handleSize/2, handleSize, handleSize);
}

void WavetableEditor::resized()
{
    // Position preset buttons at the top right of the component
    const int buttonWidth = 32;
    const int buttonHeight = 32;
    const int buttonSpacing = 10;
    const int rightMargin = 10;
    const int topMargin = 10;
    
    auto buttonY = topMargin;
    auto buttonX = getWidth() - rightMargin - (buttonWidth * 4) - (buttonSpacing * 3);
    
    linearButton.setBounds(buttonX, buttonY, buttonWidth, buttonHeight);
    buttonX += buttonWidth + buttonSpacing;
    
    expButton.setBounds(buttonX, buttonY, buttonWidth, buttonHeight);
    buttonX += buttonWidth + buttonSpacing;
    
    logButton.setBounds(buttonX, buttonY, buttonWidth, buttonHeight);
    buttonX += buttonWidth + buttonSpacing;
    
    sCurveButton.setBounds(buttonX, buttonY, buttonWidth, buttonHeight);
}

void WavetableEditor::mouseDown(const juce::MouseEvent& e)
{
    isDragging = true;
    lastDragIndex = -1;
    
    float index = xToWavetableIndex(e.position.x);
    float value = yToWavetableValue(e.position.y);
    
    int intIndex = juce::jlimit(0, static_cast<int>(wavetable.size()) - 1, static_cast<int>(index));
    updateWavetableAtIndex(intIndex, value);
    
    lastDragIndex = intIndex;
    
    repaint();
}

void WavetableEditor::mouseDrag(const juce::MouseEvent& e)
{
    if (!isDragging)
        return;
    
    float index = xToWavetableIndex(e.position.x);
    float value = yToWavetableValue(e.position.y);
    
    int intIndex = juce::jlimit(0, static_cast<int>(wavetable.size()) - 1, static_cast<int>(index));
    
    if (intIndex != lastDragIndex && lastDragIndex >= 0)
    {
        // Interpolate between last drag point and current point
        interpolateWavetableValues(lastDragIndex, wavetable[lastDragIndex], intIndex, value);
    }
    else
    {
        updateWavetableAtIndex(intIndex, value);
    }
    
    lastDragIndex = intIndex;
    
    repaint();
}

void WavetableEditor::mouseUp(const juce::MouseEvent& e)
{
    isDragging = false;
    
    if (wavetableChangedCallback)
    {
        wavetableChangedCallback(wavetable);
    }
}

void WavetableEditor::setWavetable(const std::array<float, 256>& newWavetable)
{
    wavetable = newWavetable;
    
    // Apply constraints based on mode
    if (isReleaseMode)
    {
        wavetable[0] = 1.0f;                       // Release starts at 1
        wavetable[wavetable.size() - 1] = 0.0f;    // Release ends at 0
    }
    else
    {
        wavetable[0] = 0.0f;                       // Attack starts at 0
        wavetable[wavetable.size() - 1] = 1.0f;    // Attack ends at 1
    }
    
    repaint();
}

const std::array<float, 256>& WavetableEditor::getWavetable() const
{
    return wavetable;
}

void WavetableEditor::setWavetableChangedCallback(WavetableChangedCallback callback)
{
    wavetableChangedCallback = std::move(callback);
}

void WavetableEditor::setIsReleaseMode(bool releaseMode)
{
    isReleaseMode = releaseMode;
    
    // Apply appropriate constraints based on mode
    if (isReleaseMode)
    {
        wavetable[0] = 1.0f;                       // Release starts at 1
        wavetable[wavetable.size() - 1] = 0.0f;    // Release ends at 0
    }
    else
    {
        wavetable[0] = 0.0f;                       // Attack starts at 0
        wavetable[wavetable.size() - 1] = 1.0f;    // Attack ends at 1
    }
    
    repaint();
}

float WavetableEditor::xToWavetableIndex(float x) const
{
    return (x / getWidth()) * (wavetable.size() - 1);
}

float WavetableEditor::yToWavetableValue(float y) const
{
    return 1.0f - (y / getHeight());
}

float WavetableEditor::wavetableIndexToX(float index) const
{
    return (index / (wavetable.size() - 1)) * getWidth();
}

float WavetableEditor::wavetableValueToY(float value) const
{
    return (1.0f - value) * getHeight();
}

void WavetableEditor::updateWavetableAtIndex(int index, float value)
{
    // Apply constraints based on mode
    if (index == 0)
    {
        value = isReleaseMode ? 1.0f : 0.0f;  // First point is constrained
    }
    else if (index == wavetable.size() - 1)
    {
        value = isReleaseMode ? 0.0f : 1.0f;  // Last point is constrained
    }
    
    wavetable[index] = juce::jlimit(0.0f, 1.0f, value);
}

void WavetableEditor::interpolateWavetableValues(int startIndex, float startValue, int endIndex, float endValue)
{
    // Make sure indices are in the right order
    if (startIndex > endIndex)
    {
        std::swap(startIndex, endIndex);
        std::swap(startValue, endValue);
    }
    
    // Special case for start and end constraints
    if (startIndex == 0)
        startValue = isReleaseMode ? 1.0f : 0.0f;
    
    if (endIndex == wavetable.size() - 1)
        endValue = isReleaseMode ? 0.0f : 1.0f;
    
    for (int i = startIndex; i <= endIndex; ++i)
    {
        if (startIndex == endIndex)
        {
            updateWavetableAtIndex(i, endValue);
        }
        else
        {
            float t = static_cast<float>(i - startIndex) / static_cast<float>(endIndex - startIndex);
            float interpolatedValue = startValue + t * (endValue - startValue);
            updateWavetableAtIndex(i, interpolatedValue);
        }
    }
}

// Apply chosen preset curve
void WavetableEditor::presetButtonClicked(int curveType)
{
    applyPresetCurve(curveType);
    
    // Notify callback after changing the wavetable
    if (wavetableChangedCallback)
    {
        wavetableChangedCallback(wavetable);
    }
    
    repaint();
}

// Apply a preset curve based on the selected type
void WavetableEditor::applyPresetCurve(int curveType)
{
    switch (curveType)
    {
        case 0: applyLinearCurve(); break;
        case 1: applyExponentialCurve(); break;
        case 2: applyLogarithmicCurve(); break;
        case 3: applySCurve(); break;
    }
}

// Linear curve: straight line from start to end
void WavetableEditor::applyLinearCurve()
{
    // For attack mode: 0 to 1
    // For release mode: 1 to 0
    const float startValue = isReleaseMode ? 1.0f : 0.0f;
    const float endValue = isReleaseMode ? 0.0f : 1.0f;
    
    for (size_t i = 0; i < wavetable.size(); ++i)
    {
        float normalizedPos = static_cast<float>(i) / static_cast<float>(wavetable.size() - 1);
        wavetable[i] = startValue + normalizedPos * (endValue - startValue);
    }
}

// Exponential curve: starts slow, accelerates
void WavetableEditor::applyExponentialCurve()
{
    const float startValue = isReleaseMode ? 1.0f : 0.0f;
    const float endValue = isReleaseMode ? 0.0f : 1.0f;
    const float range = endValue - startValue;
    
    for (size_t i = 0; i < wavetable.size(); ++i)
    {
        float normalizedPos = static_cast<float>(i) / static_cast<float>(wavetable.size() - 1);
        float curveValue;
        
        if (isReleaseMode)
        {
            // For release: 1 - x^2 gives slower initial decrease
            curveValue = 1.0f - std::pow(normalizedPos, 2.0f);
        }
        else
        {
            // For attack: x^2 gives slower initial increase
            curveValue = std::pow(normalizedPos, 2.0f);
        }
        
        wavetable[i] = startValue + curveValue * range;
    }
}

// Logarithmic curve: starts fast, slows down
void WavetableEditor::applyLogarithmicCurve()
{
    const float startValue = isReleaseMode ? 1.0f : 0.0f;
    const float endValue = isReleaseMode ? 0.0f : 1.0f;
    const float range = endValue - startValue;
    
    for (size_t i = 0; i < wavetable.size(); ++i)
    {
        float normalizedPos = static_cast<float>(i) / static_cast<float>(wavetable.size() - 1);
        float curveValue;
        
        if (isReleaseMode)
        {
            // For release: sqrt(1-x) gives faster initial decrease
            curveValue = std::sqrt(1.0f - normalizedPos);
        }
        else
        {
            // For attack: sqrt(x) gives faster initial increase
            curveValue = std::sqrt(normalizedPos);
        }
        
        wavetable[i] = startValue + curveValue * range;
    }
}

// S-Curve: smooth transition with sigmoid shape
void WavetableEditor::applySCurve()
{
    const float startValue = isReleaseMode ? 1.0f : 0.0f;
    const float endValue = isReleaseMode ? 0.0f : 1.0f;
    const float range = endValue - startValue;
    
    for (size_t i = 0; i < wavetable.size(); ++i)
    {
        float normalizedPos = static_cast<float>(i) / static_cast<float>(wavetable.size() - 1);
        float curveValue;
        
        // Sigmoid function: 1/(1+e^(-k*(x-0.5)))
        const float k = 10.0f; // Steepness of S-curve
        curveValue = 1.0f / (1.0f + std::exp(-k * (normalizedPos - 0.5f)));
        
        if (isReleaseMode)
        {
            // For release: invert the curve
            curveValue = 1.0f - curveValue;
        }
        
        wavetable[i] = startValue + curveValue * range;
    }
} 