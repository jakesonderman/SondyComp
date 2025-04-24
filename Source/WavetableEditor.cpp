#include "WavetableEditor.h"

WavetableEditor::WavetableEditor()
{
    // Initialize wavetable with a linear ramp
    for (int i = 0; i < wavetable.size(); ++i)
    {
        wavetable[i] = static_cast<float>(i) / static_cast<float>(wavetable.size() - 1);
    }
    
    setSize(300, 150);
}

void WavetableEditor::paint(juce::Graphics& g)
{
    // Fill background
    g.fillAll(juce::Colours::black);
    
    // Draw grid
    g.setColour(juce::Colours::darkgrey.withAlpha(0.5f));
    for (int i = 0; i <= 4; ++i)
    {
        float y = getHeight() * (1.0f - static_cast<float>(i) / 4.0f);
        g.drawLine(0.0f, y, static_cast<float>(getWidth()), y, 1.0f);
    }
    
    for (int i = 0; i <= 4; ++i)
    {
        float x = getWidth() * static_cast<float>(i) / 4.0f;
        g.drawLine(x, 0.0f, x, static_cast<float>(getHeight()), 1.0f);
    }
    
    // Draw wavetable curve
    g.setColour(juce::Colours::white);
    
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
    
    g.strokePath(path, juce::PathStrokeType(2.0f));
    
    // Draw constraints text
    g.setColour(juce::Colours::white);
    g.setFont(12.0f);
    
    if (isReleaseMode)
    {
        g.drawText("Release (Starts at 100%, ends at 0%)", getLocalBounds(), juce::Justification::topLeft, true);
    }
    else
    {
        g.drawText("Attack (Starts at 0%, ends at 100%)", getLocalBounds(), juce::Justification::topLeft, true);
    }
}

void WavetableEditor::resized()
{
    // Nothing specific to do here
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