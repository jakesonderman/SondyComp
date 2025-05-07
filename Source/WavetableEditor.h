#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <array>
#include <functional>

//==============================================================================
class WavetablePresetButton : public juce::Button
{
public:
    WavetablePresetButton(const juce::String& name) : juce::Button(name) {}
    
    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    
    void setCurveType(int type) { curveType = type; }
    int getCurveType() const { return curveType; }
    
private:
    int curveType = 0; // 0: Linear, 1: Exponential, 2: Logarithmic, 3: S-Curve
};

//==============================================================================
class WavetableEditor : public juce::Component
{
public:
    WavetableEditor();
    ~WavetableEditor() override = default;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    
    // Set and get the wavetable data
    void setWavetable(const std::array<float, 256>& newWavetable);
    const std::array<float, 256>& getWavetable() const;
    
    // Set a callback for when the wavetable changes
    using WavetableChangedCallback = std::function<void(const std::array<float, 256>&)>;
    void setWavetableChangedCallback(WavetableChangedCallback callback);
    
    // Set whether this is attack (false) or release (true) mode
    void setIsReleaseMode(bool releaseMode);
    
private:
    // Convert screen coordinates to wavetable coordinates and vice versa
    float xToWavetableIndex(float x) const;
    float yToWavetableValue(float y) const;
    float wavetableIndexToX(float index) const;
    float wavetableValueToY(float value) const;
    
    // Update the wavetable value at a specific index, respecting constraints
    void updateWavetableAtIndex(int index, float value);
    
    // Calculate intermediate points between two indices
    void interpolateWavetableValues(int startIndex, float startValue, int endIndex, float endValue);
    
    // Apply preset curves
    void applyPresetCurve(int curveType);
    void applyLinearCurve();
    void applyExponentialCurve();
    void applyLogarithmicCurve();
    void applySCurve();
    
    // Callback for preset buttons
    void presetButtonClicked(int curveType);
    
    std::array<float, 256> wavetable;
    WavetableChangedCallback wavetableChangedCallback;
    
    bool isDragging = false;
    int lastDragIndex = -1;
    
    bool isReleaseMode = false;
    
    // Preset curve buttons
    WavetablePresetButton linearButton {"Linear"};
    WavetablePresetButton expButton {"Exponential"};
    WavetablePresetButton logButton {"Logarithmic"};
    WavetablePresetButton sCurveButton {"S-Curve"};
}; 