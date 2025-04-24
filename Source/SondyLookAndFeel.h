#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class SondyLookAndFeel : public juce::LookAndFeel_V4
{
public:
    SondyLookAndFeel();
    ~SondyLookAndFeel() override = default;
    
    // Override rotary slider drawing
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, 
                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle, 
                           juce::Slider& slider) override;
    
    // Override slider text box drawing
    void drawLabel(juce::Graphics& g, juce::Label& label) override;
    
    // Override button drawing
    void drawButtonBackground(juce::Graphics& g, juce::Button& button, 
                              const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted, 
                              bool shouldDrawButtonAsDown) override;
    
    // Override drawing of meter background
    void fillResizableWindowBackground(juce::Graphics& g, int w, int h, 
                                      const juce::BorderSize<int>& border,
                                      juce::ResizableWindow& window) override;
    
    // Getter for theme colors
    struct ThemeColors {
        juce::Colour background;
        juce::Colour darkBackground;
        juce::Colour panelBackground;
        juce::Colour accent;
        juce::Colour highlight;
        juce::Colour text;
        juce::Colour dimText;
        juce::Colour controlFill;
        juce::Colour border;
        juce::Colour meterBackground;
        juce::Colour meterForeground;
        juce::Colour gridLines;
    };
    
    const ThemeColors& getThemeColors() const { return themeColors; }
    
private:
    ThemeColors themeColors;
}; 