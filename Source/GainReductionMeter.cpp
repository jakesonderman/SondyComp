#include "GainReductionMeter.h"

GainReductionMeter::GainReductionMeter()
{
    setSize(300, 100);
    
    // Initialize gain reduction history
    for (auto& value : gainReductionHistory)
    {
        value = 0.0f;
    }
    
    startTimerHz(30); // Update at 30fps
}

GainReductionMeter::~GainReductionMeter()
{
    stopTimer();
}

void GainReductionMeter::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    
    // Draw background grid with dB markings
    g.setColour(juce::Colours::darkgrey.withAlpha(0.5f));
    g.setFont(12.0f);
    
    // Draw horizontal grid lines for dB levels
    for (int dB = 0; dB <= static_cast<int>(maxGainReduction); dB += 3)
    {
        float y = dbToY(static_cast<float>(dB));
        g.drawLine(0.0f, y, static_cast<float>(getWidth()), y, 1.0f);
        
        if (dB % 6 == 0) // Label every 6 dB
        {
            g.drawText(juce::String(dB) + " dB", 5, static_cast<int>(y) - 12, 40, 12, juce::Justification::left, true);
        }
    }
    
    // Draw gain reduction history
    g.setColour(juce::Colours::orange.withAlpha(0.6f));
    
    juce::Path path;
    bool pathStarted = false;
    
    for (int i = 0; i < gainReductionHistory.size(); ++i)
    {
        float x = static_cast<float>(i) / gainReductionHistory.size() * getWidth();
        float y = dbToY(gainReductionHistory[i]);
        
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
    
    // Close the path to the bottom
    path.lineTo(static_cast<float>(getWidth()), static_cast<float>(getHeight()));
    path.lineTo(0.0f, static_cast<float>(getHeight()));
    path.closeSubPath();
    
    g.fillPath(path);
    
    // Draw current gain reduction line
    g.setColour(juce::Colours::red);
    float currentY = dbToY(currentGainReduction);
    g.drawLine(0.0f, currentY, static_cast<float>(getWidth()), currentY, 2.0f);
    
    // Draw current gain reduction value
    g.setFont(14.0f);
    g.drawText(juce::String(currentGainReduction, 1) + " dB", getWidth() - 60, static_cast<int>(currentY) - 15, 
                55, 14, juce::Justification::right, true);
}

void GainReductionMeter::resized()
{
    // Nothing specific to do here
}

void GainReductionMeter::timerCallback()
{
    repaint();
}

void GainReductionMeter::setGainReduction(float gainReductionDB)
{
    currentGainReduction = gainReductionDB;
}

void GainReductionMeter::setGainReductionHistory(const std::array<float, 256>& history)
{
    gainReductionHistory = history;
}

float GainReductionMeter::dbToY(float db) const
{
    // Clamp the gain reduction to the maximum value
    db = std::min(db, maxGainReduction);
    
    // Convert to y coordinate (0 dB at the bottom, maxGainReduction at the top)
    return getHeight() * (1.0f - db / maxGainReduction);
} 