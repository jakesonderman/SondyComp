#include "PluginProcessor.h"
#include "PluginEditor.h"

MyPluginAudioProcessorEditor::MyPluginAudioProcessorEditor (MyPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), 
      processorRef (p),
      pluginBorder(sondyLookAndFeel)
{
    // Set up our look and feel
    setLookAndFeel(&sondyLookAndFeel);
    
    // Add the border component
    addAndMakeVisible(pluginBorder);
    
    // Configure sliders
    auto setupSlider = [this](juce::Slider& slider, juce::Label& label, const juce::String& text) {
        addAndMakeVisible(slider);
        slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
        slider.setColour(juce::Slider::textBoxOutlineColourId, sondyLookAndFeel.getThemeColors().border);
        slider.setColour(juce::Slider::rotarySliderFillColourId, sondyLookAndFeel.getThemeColors().accent);
        slider.setColour(juce::Slider::rotarySliderOutlineColourId, sondyLookAndFeel.getThemeColors().border);
        slider.setLookAndFeel(&sondyLookAndFeel);
        
        addAndMakeVisible(label);
        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setFont(juce::Font(16.0f, juce::Font::bold));
        label.attachToComponent(&slider, false);
    };
    
    // Setup each slider with its label
    setupSlider(inputGainSlider, inputGainLabel, "INPUT");
    setupSlider(outputGainSlider, outputGainLabel, "OUTPUT");
    setupSlider(thresholdSlider, thresholdLabel, "THRESHOLD");
    setupSlider(kneeSlider, kneeLabel, "KNEE");
    setupSlider(attackTimeSlider, attackTimeLabel, "ATTACK TIME");
    setupSlider(releaseTimeSlider, releaseTimeLabel, "RELEASE TIME");
    
    // Configure sliders to show time in seconds for attack and release
    attackTimeSlider.setTextValueSuffix(" s");
    releaseTimeSlider.setTextValueSuffix(" s");
    
    // Connect sliders to parameters
    auto& parameters = processorRef.getParameters();
    inputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        parameters, MyPluginAudioProcessor::inputGainId, inputGainSlider);
    outputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        parameters, MyPluginAudioProcessor::outputGainId, outputGainSlider);
    thresholdAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        parameters, MyPluginAudioProcessor::thresholdId, thresholdSlider);
    kneeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        parameters, MyPluginAudioProcessor::kneeId, kneeSlider);
    attackTimeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        parameters, MyPluginAudioProcessor::attackTimeId, attackTimeSlider);
    releaseTimeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        parameters, MyPluginAudioProcessor::releaseTimeId, releaseTimeSlider);
    
    // Gain reduction meter
    addAndMakeVisible(gainReductionMeter);
    
    // Wavetable editors
    addAndMakeVisible(attackWavetableEditor);
    attackWavetableEditor.setIsReleaseMode(false);
    attackWavetableEditor.setWavetable(processorRef.getCompressor().getAttackWavetable());
    attackWavetableEditor.setWavetableChangedCallback([this](const std::array<float, 256>& wavetable) {
        processorRef.getCompressor().setAttackWavetable(wavetable);
    });
    
    addAndMakeVisible(releaseWavetableEditor);
    releaseWavetableEditor.setIsReleaseMode(true);
    releaseWavetableEditor.setWavetable(processorRef.getCompressor().getReleaseWavetable());
    releaseWavetableEditor.setWavetableChangedCallback([this](const std::array<float, 256>& wavetable) {
        processorRef.getCompressor().setReleaseWavetable(wavetable);
    });
    
    // Wavetable labels
    auto setupEditorLabel = [this](juce::Label& label, const juce::String& text) {
        addAndMakeVisible(label);
        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setFont(juce::Font(18.0f, juce::Font::bold));
        label.setColour(juce::Label::textColourId, sondyLookAndFeel.getThemeColors().text.brighter(0.2f));
    };
    
    setupEditorLabel(attackEditorLabel, "ATTACK CURVE");
    setupEditorLabel(releaseEditorLabel, "RELEASE CURVE");
    
    // Set size of the editor
    setSize (1000, 650);
    
    // Start the timer to update UI
    startTimerHz(60);
}

MyPluginAudioProcessorEditor::~MyPluginAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
    stopTimer();
}

void MyPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Draw background animation if enabled
    if (enableBackgroundAnimation)
    {
        drawBackgroundAnimation(g);
    }
    
    // Background is also handled by the border component
}

void MyPluginAudioProcessorEditor::drawBackgroundAnimation(juce::Graphics& g)
{
    const auto& colors = sondyLookAndFeel.getThemeColors();
    
    // Create a subtle animated pattern in the background
    // This will be drawn behind the border
    const int gridSize = 40;
    const float minOpacity = 0.05f;
    const float maxOpacity = 0.12f;
    
    // Draw horizontal lines
    for (int y = -gridSize; y < getHeight() + gridSize; y += gridSize)
    {
        float yPhase = (y + animationPhase * 0.3f) / static_cast<float>(getHeight());
        float opacity = minOpacity + (maxOpacity - minOpacity) * 0.5f * (1.0f + std::sin(yPhase * juce::MathConstants<float>::pi * 8.0f + animationPhase * 0.01f));
        
        g.setColour(colors.accent.withAlpha(opacity));
        
        // Draw a wavy line
        juce::Path path;
        path.startNewSubPath(0, y + std::sin(animationPhase * 0.02f) * 5.0f);
        
        for (int x = 0; x < getWidth(); x += 5)
        {
            float xPhase = static_cast<float>(x) / static_cast<float>(getWidth());
            float offset = std::sin(xPhase * juce::MathConstants<float>::pi * 4.0f + animationPhase * 0.02f) * 3.0f;
            path.lineTo(x, y + offset);
        }
        
        g.strokePath(path, juce::PathStrokeType(0.5f));
    }
    
    // Draw vertical lines
    for (int x = -gridSize; x < getWidth() + gridSize; x += gridSize)
    {
        float xPhase = (x + animationPhase * 0.3f) / static_cast<float>(getWidth());
        float opacity = minOpacity + (maxOpacity - minOpacity) * 0.5f * (1.0f + std::sin(xPhase * juce::MathConstants<float>::pi * 8.0f + animationPhase * 0.01f));
        
        g.setColour(colors.accent.withAlpha(opacity));
        
        // Draw a wavy line
        juce::Path path;
        path.startNewSubPath(x + std::sin(animationPhase * 0.02f) * 5.0f, 0);
        
        for (int y = 0; y < getHeight(); y += 5)
        {
            float yPhase = static_cast<float>(y) / static_cast<float>(getHeight());
            float offset = std::sin(yPhase * juce::MathConstants<float>::pi * 4.0f + animationPhase * 0.02f) * 3.0f;
            path.lineTo(x + offset, y);
        }
        
        g.strokePath(path, juce::PathStrokeType(0.5f));
    }
}

void MyPluginAudioProcessorEditor::resized()
{
    // Update border to match component size
    pluginBorder.setBounds(getLocalBounds());
    
    // Content area with some padding from the border
    auto area = getLocalBounds().reduced(15);
    
    // Allow space for the title bar
    area.removeFromTop(30); // Slightly more space for the title
    
    // Calculate overall layout
    const int verticalGap = 15; // Gap between sections
    const int horizontalGap = 25; // Gap between main panels
    
    // We'll use a grid layout with three main columns
    const int totalAvailableWidth = area.getWidth();
    
    // Calculate optimal widths for the three columns (attack, center, release)
    const int wavetableWidth = (totalAvailableWidth - (2 * horizontalGap)) / 3;
    const int centerWidth = wavetableWidth;
    
    // Divide main area into three distinct columns with gaps between them
    auto leftArea = area.removeFromLeft(wavetableWidth);
    area.removeFromLeft(horizontalGap); // Gap between left and center
    auto centerArea = area.removeFromLeft(centerWidth);
    area.removeFromLeft(horizontalGap); // Gap between center and right
    auto rightArea = area; // Remaining area for release section
    
    // --- LEFT COLUMN (ATTACK) ---
    auto attackLabelArea = leftArea.removeFromTop(30);
    attackEditorLabel.setBounds(attackLabelArea);
    
    leftArea.removeFromTop(verticalGap); // Space after label
    
    auto attackSliderArea = leftArea.removeFromTop(80); // Increased height for slider
    attackTimeSlider.setBounds(attackSliderArea);
    
    leftArea.removeFromTop(verticalGap); // Space after slider
    
    // Wavetable editor takes the remaining space with padding
    attackWavetableEditor.setBounds(leftArea);
    
    // --- RIGHT COLUMN (RELEASE) ---
    auto releaseLabelArea = rightArea.removeFromTop(30);
    releaseEditorLabel.setBounds(releaseLabelArea);
    
    rightArea.removeFromTop(verticalGap); // Space after label
    
    auto releaseSliderArea = rightArea.removeFromTop(80); // Increased height for slider
    releaseTimeSlider.setBounds(releaseSliderArea);
    
    rightArea.removeFromTop(verticalGap); // Space after slider
    
    // Wavetable editor takes the remaining space
    releaseWavetableEditor.setBounds(rightArea);
    
    // --- CENTER COLUMN (CONTROLS & METER) ---
    // GR meter at the top
    auto meterHeight = centerArea.getHeight() * 0.4;
    auto meterArea = centerArea.removeFromTop(meterHeight);
    gainReductionMeter.setBounds(meterArea);
    
    centerArea.removeFromTop(verticalGap * 2); // Extra space between meter and controls
    
    // Four knobs in a grid (2x2) layout with spacing
    const int knobWidth = centerWidth / 2;
    const int knobHeight = 110; // Increased height for knobs
    const int knobSpacing = 15;
    
    // Top row - Input and Output
    auto topRowArea = centerArea.removeFromTop(knobHeight);
    auto inputArea = topRowArea.removeFromLeft(knobWidth).reduced(knobSpacing);
    inputGainSlider.setBounds(inputArea);
    
    auto outputArea = topRowArea.reduced(knobSpacing);
    outputGainSlider.setBounds(outputArea);
    
    centerArea.removeFromTop(verticalGap); // Space between rows
    
    // Bottom row - Threshold and Knee
    auto bottomRowArea = centerArea.removeFromTop(knobHeight);
    auto thresholdArea = bottomRowArea.removeFromLeft(knobWidth).reduced(knobSpacing);
    thresholdSlider.setBounds(thresholdArea);
    
    auto kneeArea = bottomRowArea.reduced(knobSpacing);
    kneeSlider.setBounds(kneeArea);
}

void MyPluginAudioProcessorEditor::timerCallback()
{
    // Update the gain reduction meter
    gainReductionMeter.setGainReduction(processorRef.getCompressor().getCurrentGainReduction());
    gainReductionMeter.setInputLevel(processorRef.getCompressor().getCurrentInputLevel());
    
    // Update background animation
    if (enableBackgroundAnimation)
    {
        animationPhase += animationSpeed;
        if (animationPhase > 1000.0f) // Reset to avoid float precision issues
            animationPhase = 0.0f;
            
        repaint();
    }
} 