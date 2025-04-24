#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "SondyLookAndFeel.h"

//==============================================================================
class PluginBorder : public juce::Component
{
public:
    PluginBorder(SondyLookAndFeel& lnf) : lookAndFeel(lnf)
    {
        setInterceptsMouseClicks(false, true);
    }
    
    ~PluginBorder() override = default;
    
    void paint(juce::Graphics& g) override
    {
        const auto& colors = lookAndFeel.getThemeColors();
        
        // Fill main background with slight gradient
        juce::ColourGradient bgGradient(
            colors.background.darker(0.3f), 0.0f, 0.0f,
            colors.background.darker(0.1f), 0, static_cast<float>(getHeight()),
            false);
        g.setGradientFill(bgGradient);
        g.fillAll();
        
        // Draw subtle radial glow in the center for depth
        const float centerX = getWidth() * 0.5f;
        const float centerY = getHeight() * 0.4f; // Slightly above center
        const float radius = std::max(getWidth(), getHeight()) * 0.8f;
        
        juce::ColourGradient radialGlow(
            colors.background.brighter(0.06f), centerX, centerY,
            colors.background, centerX + radius, centerY,
            true);
        g.setGradientFill(radialGlow);
        g.fillRect(getLocalBounds());
        
        // Draw the main outer border with 3D effect
        g.setColour(colors.border.darker(0.2f)); // Bottom/right shadow
        g.drawLine(0, getHeight(), getWidth(), getHeight(), 1.5f); // Bottom
        g.drawLine(getWidth(), 0, getWidth(), getHeight(), 1.5f); // Right
        
        g.setColour(colors.border.brighter(0.1f)); // Top/left highlight
        g.drawLine(0, 0, getWidth(), 0, 1.5f); // Top
        g.drawLine(0, 0, 0, getHeight(), 1.5f); // Left
        
        // Draw inner border with gradient effect
        const auto innerBounds = getLocalBounds().reduced(3);
        
        // Draw subtle inner shadow on the border
        juce::Path innerShadowPath;
        innerShadowPath.addRectangle(innerBounds);
        
        juce::DropShadow innerShadow(colors.darkBackground.darker(0.5f), 4, juce::Point<int>(0, 1));
        innerShadow.drawForPath(g, innerShadowPath);
        
        // Draw inner border outline
        g.setColour(colors.border.darker(0.2f));
        g.drawRect(innerBounds, 1);
        
        // Draw title section at the top
        const auto titleHeight = 30;
        const auto titleBounds = juce::Rectangle<int>(0, 0, getWidth(), titleHeight);
        
        // Title bar gradient background
        juce::ColourGradient titleGradient(
            colors.darkBackground.darker(0.3f), 0.0f, 0.0f,
            colors.darkBackground.darker(0.1f), 0.0f, static_cast<float>(titleHeight),
            false);
        g.setGradientFill(titleGradient);
        g.fillRect(titleBounds);
        
        // Title bar border
        g.setColour(colors.border.darker(0.2f));
        g.drawLine(0, titleHeight, getWidth(), titleHeight, 1.5f);
        
        // Title glow effect
        juce::Path titleTextPath;
        juce::GlyphArrangement titleGlyphs;
        
        juce::Font titleFont(titleHeight * 0.6f, juce::Font::bold);
        titleGlyphs.addFittedText(titleFont, "SONDY COMPRESSOR", 
                                  10, titleHeight * 0.5f - 10, getWidth() - 20, titleHeight, 
                                  juce::Justification::centredLeft, 1);
        titleGlyphs.createPath(titleTextPath);
        
        // Add glow effect to title
        g.setColour(colors.accent.withAlpha(0.15f));
        for (int i = 4; i > 0; --i)
        {
            float featherAmount = static_cast<float>(i) * 0.9f;
            juce::Path glowPath(titleTextPath);
            juce::PathStrokeType strokeType(featherAmount, juce::PathStrokeType::curved, juce::PathStrokeType::rounded);
            strokeType.createStrokedPath(glowPath, titleTextPath);
            g.fillPath(glowPath);
        }
        
        // Draw title text
        g.setColour(colors.text.brighter(0.1f));
        g.setFont(titleFont);
        g.drawText("SONDY COMPRESSOR", titleBounds.reduced(10, 0), 
                   juce::Justification::centredLeft, true);
        
        // Draw version in the top right corner with subtle styling
        g.setFont(titleHeight * 0.4f);
        g.setColour(colors.dimText);
        
        juce::String versionText = "v1.0";
        juce::Rectangle<int> versionBounds(getWidth() - 60, 5, 50, titleHeight - 10);
        
        // Version background
        g.setColour(colors.darkBackground.darker(0.3f));
        g.fillRoundedRectangle(versionBounds.toFloat(), 3.0f);
        
        // Version border
        g.setColour(colors.border.darker(0.1f));
        g.drawRoundedRectangle(versionBounds.toFloat(), 3.0f, 1.0f);
        
        // Version text
        g.setColour(colors.dimText);
        g.drawText(versionText, versionBounds, juce::Justification::centred, false);
    }
    
    void resized() override {}
    
private:
    SondyLookAndFeel& lookAndFeel;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginBorder)
}; 