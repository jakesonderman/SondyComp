#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <array>

//==============================================================================
class GainReductionMeter : public juce::Component, private juce::Timer
{
public:
    GainReductionMeter();
    ~GainReductionMeter() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void timerCallback() override;
    
    // Set the current gain reduction value
    void setGainReduction(float gainReductionDB);
    
    // Set the history buffer for visualization
    void setGainReductionHistory(const std::array<float, 256>& history);
    
private:
    float currentGainReduction = 0.0f;
    std::array<float, 256> gainReductionHistory;
    
    const float maxGainReduction = 24.0f; // Maximum gain reduction to display (dB)
    
    // Helper function to convert dB to y coordinate
    float dbToY(float db) const;
}; 