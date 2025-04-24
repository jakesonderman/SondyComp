#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <array>
#include <deque>

//==============================================================================
class GainReductionMeter : public juce::Component, private juce::Timer
{
public:
    GainReductionMeter();
    ~GainReductionMeter() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void timerCallback() override;
    
    // Set the current gain reduction value and add it to history
    void setGainReduction(float gainReductionDB);
    
    // Set the current input level value (pre-compression)
    void setInputLevel(float inputLevelDB);
    
    // Set the history buffer for visualization (optional, usually using live data)
    void setGainReductionHistory(const std::array<float, 256>& history);
    
    // Add audio sample to the visualization buffer
    void pushAudioSample(float sample);
    
    // Set the entire audio buffer (for bulk updates)
    void setAudioBuffer(const float* buffer, int numSamples);
    
private:
    // Current gain reduction value
    float currentGainReduction = 0.0f;
    
    // Current input level value (pre-compression)
    float currentInputLevel = 0.0f;
    
    // Circular buffer for gain reduction history
    static constexpr size_t historySize = 512; // Longer history buffer
    std::deque<float> gainReductionHistory;
    
    // Circular buffer for input level history
    std::deque<float> inputLevelHistory;
    
    // Peak hold for visualization
    float peakGainReduction = 0.0f;
    int peakHoldCounter = 0;
    static constexpr int peakHoldTime = 60; // Frames to hold peak
    
    // History update parameters
    int updateCounter = 0;
    int updateDivider = 1; // Update history every N frames
    bool animateMeter = true;
    int scrollSpeed = 1; // Pixels to scroll per update
    
    const float maxGainReduction = 24.0f; // Maximum gain reduction to display (dB)
    juce::uint32 lastUpdateTime = 0;
    
    // Background animation properties
    float animationPhase = 0.0f;
    bool enableBackgroundAnimation = true;
    
    // Line styles
    juce::Colour currentLineColor = juce::Colour(0xFFE6E6E6);
    juce::Colour averageLineColor = juce::Colour(0xFF2C9AFF);
    juce::Colour peakLineColor = juce::Colour(0xFFFF5D5D);
    juce::Colour inputLineColor = juce::Colour(0xFF00DD77);
    
    // Helper function to convert dB to y coordinate for GR (from the bottom)
    float dbToY(float db) const;
    
    // Helper function to convert dB to y coordinate for audio input (from the top)
    float dbToAudioY(float db) const;
    
    // Draw the animated background
    void drawAnimatedBackground(juce::Graphics& g);
    
    // Update the peak tracking values
    void updatePeakTracking();
}; 