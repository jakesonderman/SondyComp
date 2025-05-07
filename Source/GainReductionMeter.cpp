#include "GainReductionMeter.h"

GainReductionMeter::GainReductionMeter()
{
    setSize(300, 100);
    
    // Initialize gain reduction history with zeros
    gainReductionHistory.resize(historySize, 0.0f);
    
    // Initialize input level history with zeros
    inputLevelHistory.resize(historySize, 0.0f);
    
    // Start the timer for updates and animation
    startTimerHz(60); // Update at 60fps for smoother animation
}

GainReductionMeter::~GainReductionMeter()
{
    stopTimer();
}

void GainReductionMeter::paint(juce::Graphics& g)
{
    // Fill the background with a darker color
    g.fillAll(juce::Colour(0xFF1A1A1A));
    
    if (enableBackgroundAnimation)
    {
        drawAnimatedBackground(g);
    }
    
    // Draw border around the meter
    g.setColour(juce::Colour(0xFF454545));
    g.drawRect(getLocalBounds(), 1);
    
    // Draw inner border
    g.setColour(juce::Colour(0xFF303030));
    g.drawRect(getLocalBounds().reduced(2), 1);
    
    // Add a softer glow effect around the meter
    juce::ColourGradient glowGradient(
        juce::Colour(0xFF2C9AFF).withAlpha(0.08f), getWidth() * 0.5f, getHeight() * 0.5f,
        juce::Colour(0xFF2C9AFF).withAlpha(0.0f), 0, 0,
        true);
    g.setGradientFill(glowGradient);
    g.fillRect(getLocalBounds());
    
    // Draw background grid with dB markings
    g.setColour(juce::Colour(0xFF454545));
    g.setFont(11.0f);
    
    // Draw horizontal grid lines for dB levels
    for (int dB = 0; dB <= static_cast<int>(maxGainReduction); dB += 3)
    {
        float y = dbToY(static_cast<float>(dB));
        g.drawLine(0.0f, y, static_cast<float>(getWidth()), y, 1.0f);
        
        if (dB % 6 == 0) // Label every 6 dB
        {
            g.setColour(juce::Colour(0xFF9E9E9E));
            g.drawText(juce::String(dB) + " dB", 5, static_cast<int>(y) - 12, 40, 12, juce::Justification::left, true);
            g.setColour(juce::Colour(0xFF454545));
        }
    }
    
    // Draw vertical grid lines
    g.setColour(juce::Colour(0xFF353535));
    for (int i = 0; i <= 8; ++i) // More vertical grid lines for a cleaner look
    {
        float x = i * getWidth() / 8.0f;
        g.drawLine(x, 0.0f, x, static_cast<float>(getHeight()), 1.0f);
    }
    
    // Only draw if we have data
    if (gainReductionHistory.size() > 1)
    {
        // Calculate the display width based on available data
        float pointWidth = static_cast<float>(getWidth()) / std::min(historySize, gainReductionHistory.size());
        
        // ==== DRAW INPUT LEVEL FROM THE TOP ====
        if (inputLevelHistory.size() > 1)
        {
            juce::Path inputPath;
            bool pathStarted = false;
            
            for (size_t i = 0; i < inputLevelHistory.size(); ++i)
            {
                float x = static_cast<float>(i) * pointWidth;
                
                // Get actual input level value from the history
                float inputLevelDB = inputLevelHistory[i];
                
                // Clamp to a range suitable for display (negative dB values up to 0dB)
                inputLevelDB = juce::jlimit(-maxGainReduction, 0.0f, inputLevelDB);
                
                // Draw from the top - convert level to Y position
                // We need to map the negative dB scale (-24 to 0) to our Y coordinate space
                float mappedValue = -inputLevelDB / maxGainReduction; // 0 to 1 range
                float y = mappedValue * (getHeight() / 2.0f); // Map to upper half
                
                if (!pathStarted)
                {
                    inputPath.startNewSubPath(x, y);
                    pathStarted = true;
                }
                else
                {
                    inputPath.lineTo(x, y);
                }
            }
            
            // Draw the input level line with enhanced appearance
            g.setColour(inputLineColor);
            g.strokePath(inputPath, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            
            // Add a subtle fill below the input line
            inputPath.lineTo(static_cast<float>(getWidth()), static_cast<float>(getHeight() / 2));
            inputPath.lineTo(0.0f, static_cast<float>(getHeight() / 2));
            inputPath.closeSubPath();
            
            juce::ColourGradient inputGradient(
                inputLineColor.withAlpha(0.2f), 0.0f, 0.0f,
                inputLineColor.withAlpha(0.02f), 0.0f, static_cast<float>(getHeight() / 2),
                false);
            g.setGradientFill(inputGradient);
            g.fillPath(inputPath);
            
            // Add glow effect for input line
            g.setColour(inputLineColor.withAlpha(0.1f));
            g.strokePath(inputPath, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }
        
        // ==== DRAW GAIN REDUCTION FROM THE BOTTOM ====
        {
            juce::Path fillPath;
            bool pathStarted = false;
            
            for (size_t i = 0; i < gainReductionHistory.size(); ++i)
            {
                float x = static_cast<float>(i) * pointWidth;
                // Draw from the bottom of the component
                float y = dbToY(gainReductionHistory[i]);
                
                if (!pathStarted)
                {
                    fillPath.startNewSubPath(x, y);
                    pathStarted = true;
                }
                else
                {
                    fillPath.lineTo(x, y);
                }
            }
            
            // Close the path to the bottom for the inverted visualization
            fillPath.lineTo(static_cast<float>(getWidth()), static_cast<float>(getHeight()));
            fillPath.lineTo(0.0f, static_cast<float>(getHeight()));
            fillPath.closeSubPath();
            
            // Create a gradient fill for the meter (starting from bottom)
            juce::ColourGradient gradient(
                juce::Colour(0xFF2C9AFF).withAlpha(0.3f), 0.0f, static_cast<float>(getHeight()),
                juce::Colour(0xFF2C9AFF).withAlpha(0.05f), 0.0f, static_cast<float>(getHeight() / 2),
                false);
            
            g.setGradientFill(gradient);
            g.fillPath(fillPath);
            
            // Draw the main GR line with a bolder stroke
            g.setColour(juce::Colour(0xFF2C9AFF));
            g.strokePath(fillPath, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            
            // Add glow effect
            g.setColour(juce::Colour(0xFF2C9AFF).withAlpha(0.3f));
            g.strokePath(fillPath, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }
        
        // Draw peak markers
        if (peakGainReduction > 0.01f)
        {
            float peakY = dbToY(peakGainReduction);
            
            // Draw peak marker as a more visible diamond shape
            const float markerSize = 10.0f;
            juce::Path peakMarker;
            peakMarker.addRectangle(-markerSize/2, -markerSize/2, markerSize, markerSize);
            juce::AffineTransform transform = juce::AffineTransform::rotation(juce::MathConstants<float>::pi * 0.25f)
                                             .translated(getWidth() - markerSize - 5.0f, peakY);
            peakMarker.applyTransform(transform);
            
            // Draw glow effect
            g.setColour(peakLineColor.withAlpha(0.4f));
            g.fillPath(peakMarker);
            
            // Draw peak marker
            g.setColour(peakLineColor);
            g.strokePath(peakMarker, juce::PathStrokeType(1.0f));
            
            // Draw peak value text
            g.setFont(12.0f);
            juce::String peakText = juce::String(peakGainReduction, 1) + " dB";
            g.drawText(peakText, getWidth() - 75, static_cast<int>(peakY) - 10, 65, 20, juce::Justification::right, false);
        }
    }
    
    // Add title label with enhanced styling
    juce::Rectangle<int> titleArea = getLocalBounds().removeFromTop(25);
    
    // Draw title background
    g.setColour(juce::Colour(0xFF262626));
    g.fillRect(titleArea.reduced(1));
    
    // Draw title border
    g.setColour(juce::Colour(0xFF3A3A3A));
    g.drawRect(titleArea.reduced(1), 1);
    
    // Draw title text with glow
    g.setFont(14.0f);
    juce::String titleText = "GAIN REDUCTION SCOPE";
    
    // Glow effect
    g.setColour(juce::Colour(0xFF2C9AFF).withAlpha(0.2f));
    g.drawText(titleText, titleArea.translated(0, 1), juce::Justification::centred, false);
    
    // Title text
    g.setColour(juce::Colour(0xFFBBBBBB));
    g.drawText(titleText, titleArea, juce::Justification::centred, false);
}

void GainReductionMeter::drawAnimatedBackground(juce::Graphics& g)
{
    // Create an animated grid effect in the background
    const juce::Colour gridColor = juce::Colour(0xFF232323);
    const int gridSpacing = 15;
    
    // Update animation phase
    float phaseDelta = 0.2f; // Adjust for faster/slower animation
    
    // Vertical lines
    for (int x = static_cast<int>(-animationPhase) % gridSpacing; x < getWidth(); x += gridSpacing)
    {
        float alpha = 0.3f + 0.15f * std::sin((x + animationPhase) * 0.05f);
        g.setColour(gridColor.withAlpha(alpha));
        g.drawVerticalLine(x, 0.0f, static_cast<float>(getHeight()));
    }
    
    // Horizontal lines with less movement
    for (int y = 0; y < getHeight(); y += gridSpacing)
    {
        float alpha = 0.2f + 0.1f * std::sin((y + animationPhase * 0.25f) * 0.05f);
        g.setColour(gridColor.withAlpha(alpha));
        g.drawHorizontalLine(y, 0.0f, static_cast<float>(getWidth()));
    }
}

void GainReductionMeter::updatePeakTracking()
{
    // Update peak hold if current value exceeds previous peak
    if (currentGainReduction > peakGainReduction)
    {
        peakGainReduction = currentGainReduction;
        peakHoldCounter = peakHoldTime;
    }
    else if (peakHoldCounter > 0)
    {
        peakHoldCounter--;
    }
    else if (peakGainReduction > 0.01f)
    {
        // Gradually decay peak value
        peakGainReduction *= 0.995f;
        if (peakGainReduction < 0.01f)
            peakGainReduction = 0.0f;
    }
}

void GainReductionMeter::resized()
{
    // Nothing specific to do here
}

void GainReductionMeter::timerCallback()
{
    // Update animation
    if (enableBackgroundAnimation)
    {
        animationPhase += 0.5f;
        if (animationPhase > 1000.0f) // Reset to avoid float precision issues
            animationPhase = 0.0f;
    }
    
    // Update peak tracking
    updatePeakTracking();
    
    // Process meter animation
    if (animateMeter)
    {
        updateCounter++;
        if (updateCounter >= updateDivider)
        {
            updateCounter = 0;
            
            // Scroll the data
            for (int i = 0; i < scrollSpeed; ++i)
            {
                // Scroll gain reduction data
                if (!gainReductionHistory.empty())
                {
                    gainReductionHistory.pop_front();
                    gainReductionHistory.push_back(currentGainReduction);
                }
                
                // Scroll input level data
                if (!inputLevelHistory.empty())
                {
                    inputLevelHistory.pop_front();
                    inputLevelHistory.push_back(currentInputLevel);
                }
            }
        }
    }
    
    repaint();
}

void GainReductionMeter::setGainReduction(float gainReductionDB)
{
    currentGainReduction = gainReductionDB;
    
    // Add current value to history if not animating (otherwise handled in the timer)
    if (!animateMeter)
    {
        if (gainReductionHistory.size() >= historySize)
        {
            gainReductionHistory.pop_front();
        }
        gainReductionHistory.push_back(gainReductionDB);
    }
}

void GainReductionMeter::setInputLevel(float inputLevelDB)
{
    currentInputLevel = inputLevelDB;
    
    // Add current value to history if not animating (otherwise handled in the timer)
    if (!animateMeter)
    {
        if (inputLevelHistory.size() >= historySize)
        {
            inputLevelHistory.pop_front();
        }
        inputLevelHistory.push_back(inputLevelDB);
    }
}

void GainReductionMeter::setGainReductionHistory(const std::array<float, 256>& history)
{
    // Use this only for non-animated mode or initial values
    if (!animateMeter)
    {
        gainReductionHistory.clear();
        
        for (float value : history)
        {
            if (gainReductionHistory.size() < historySize)
            {
                gainReductionHistory.push_back(value);
            }
        }
    }
}

float GainReductionMeter::dbToY(float db) const
{
    // Clamp the gain reduction to the maximum value
    db = std::min(db, maxGainReduction);
    
    // Convert to y coordinate (0 dB at the BOTTOM, maxGainReduction at the top)
    // This flips the visualization to draw from the bottom up
    return getHeight() - (db / maxGainReduction * getHeight());
}

float GainReductionMeter::dbToAudioY(float db) const
{
    // Clamp the gain reduction to the maximum value
    db = std::min(db, maxGainReduction);
    
    // Convert to y coordinate (0 dB at the TOP, maxGainReduction at the bottom)
    // This draws from the top down, the opposite of dbToY
    return db / maxGainReduction * getHeight();
} 